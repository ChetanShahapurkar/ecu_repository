// Copyright 2020 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_modem_dce_service.h"
#include "bg96.h"
#include "driver/gpio.h"
#include "../../HardwareConfig.h"
#include "esp_netif_ppp.h"
#include "esp_log.h"
#include "sim7600.h"

#include "esp_netif.h"
#include "mqtt_client.h"
#include "esp_modem.h"
#include "esp_modem_netif.h"
 #include "cJSON.h"

 #include "GsmGprsManager.h"
 #include "SdCard.h"
 #include "libGSM.h"
 #include "Timers.h"

bool intteruptDisabled_status = false;
bool incomming_call = false;
bool _system_boot = false;

//=================================================== CONSTANTS==============================================================
// #define MQTT_BROKER_URL 	"mqtt://abhed.nexlein.com"
#define MQTT_QOS			(1)
#define MQTT_RETAIN			(0)

//=================================================== PUBLIC VARIABLES=======================================================

modem_dce_t *sim7600_dce;
modem_dte_t *sim7600_dte;
esp_netif_t *sim7600_esp_netif;
void *sim7600_modem_netif_adapter;
esp_netif_auth_type_t sim7600_auth_type;
static EventGroupHandle_t event_sim7600_group = NULL;


esp_mqtt_client_handle_t mqtt_client;
esp_mqtt_client_handle_t client; 
int msg_id;
bool sim7600_network_status = false;
bool mqtt_connected = false;
bool mqtt_subscribed = false;
bool system_boot_message_tx = false;
char *_json_message;
uint32_t sim7600_ppp_state = 0;
bool mqtt_msg_send_status = false;
bool sim7600_new_sms_received = false;

//=================================================== PRIVATE VARIABLES=======================================================

 static const char *TAG = "SIM7600";
 static const int CONNECT_BIT = BIT0;
 static const int STOP_BIT = BIT1;
 static const int GOT_DATA_BIT = BIT2;

 char * lastsms_cmd;

//=================================================== PRIVATE FUNCTIONS=======================================================
 static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event);

/**
 * @brief This module supports SIM7600 module, which has a very similar interface
 * to the BG96, so it just references most of the handlers from BG96 and implements
 * only those that differ.
 */
static const char *DCE_TAG = "SIM7600";

/**
 * @brief Macro defined for error checking
 *
 */
#define DCE_CHECK(a, str, goto_tag, ...)                                              \
    do                                                                                \
    {                                                                                 \
        if (!(a))                                                                     \
        {                                                                             \
            ESP_LOGE(DCE_TAG, "%s(%d): " str, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
            goto goto_tag;                                                            \
        }                                                                             \
    } while (0)

/**
 * @brief Handle response from AT+CBC
 */
static esp_err_t sim7600_handle_cbc(modem_dce_t *dce, const char *line)
{
    esp_err_t err = ESP_FAIL;
    esp_modem_dce_t *esp_modem_dce = __containerof(dce, esp_modem_dce_t, parent);
    if (strstr(line, MODEM_RESULT_CODE_SUCCESS)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_SUCCESS);
    } else if (strstr(line, MODEM_RESULT_CODE_ERROR)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_FAIL);
    } else if (!strncmp(line, "+CBC", strlen("+CBC"))) {
        /* store value of bcs, bcl, voltage */
        int32_t **cbc = esp_modem_dce->priv_resource;
        int32_t volts = 0, fraction = 0;
        /* +CBC: <voltage in Volts> V*/
        sscanf(line, "+CBC: %d.%dV", &volts, &fraction);
        /* Since the "read_battery_status()" API (besides voltage) returns also values for BCS, BCL (charge status),
         * which are not applicable to this modem, we return -1 to indicate invalid value
         */
        *cbc[0] = -1; // BCS
        *cbc[1] = -1; // BCL
        *cbc[2] = volts*1000 + fraction;
        err = ESP_OK;
    }
    return err;
}

/**
 * @brief Handle response from AT+CPOF
 */
static esp_err_t sim7600_handle_cpof(modem_dce_t *dce, const char *line)
{
    esp_err_t err = ESP_OK;
    if (strstr(line, MODEM_RESULT_CODE_SUCCESS)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_SUCCESS);
    } else if (strstr(line, MODEM_RESULT_CODE_NO_CARRIER)) {
        err = ESP_OK;
    } else if (strstr(line, MODEM_RESULT_CODE_ERROR)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_FAIL);
    }
    return err;
}

/**
 * @brief Get battery status
 *
 * @param dce Modem DCE object
 * @param bcs Battery charge status
 * @param bcl Battery connection level
 * @param voltage Battery voltage
 * @return esp_err_t
 *      - ESP_OK on success
 *      - ESP_FAIL on error
 */
static esp_err_t sim7600_get_battery_status(modem_dce_t *dce, uint32_t *bcs, uint32_t *bcl, uint32_t *voltage)
{
    modem_dte_t *dte = dce->dte;
    esp_modem_dce_t *esp_modem_dce = __containerof(dce, esp_modem_dce_t, parent);
    uint32_t *resource[3] = {bcs, bcl, voltage};
    esp_modem_dce->priv_resource = resource;
    dce->handle_line = sim7600_handle_cbc;
    DCE_CHECK(dte->send_cmd(dte, "AT+CBC\r", MODEM_COMMAND_TIMEOUT_DEFAULT) == ESP_OK, "send command failed", err);
    DCE_CHECK(dce->state == MODEM_STATE_SUCCESS, "inquire battery status failed", err);
    ESP_LOGD(DCE_TAG, "inquire battery status ok");
    return ESP_OK;
err:
    return ESP_FAIL;
}

/**
 * @brief Set the SIM7600 device to power down mode
 *
 * @param dce common modem dce object (modem_dce_t)
 * @return esp_err_t
 *      - ESP_OK on success
 *      - ESP_FAIL on error
 */
static esp_err_t sim7600_power_down(modem_dce_t *dce)
{
    modem_dte_t *dte = dce->dte;
    dce->handle_line = sim7600_handle_cpof;
    DCE_CHECK(dte->send_cmd(dte, "AT+CPOF\r", MODEM_COMMAND_TIMEOUT_POWEROFF) == ESP_OK, "send command failed", err);
    DCE_CHECK(dce->state == MODEM_STATE_SUCCESS, "power down failed", err);
    ESP_LOGD(DCE_TAG, "power down ok");
    return ESP_OK;
err:
    return ESP_FAIL;
}

/**
 * @brief Create and initialize SIM7600 object
 *
 */
modem_dce_t *sim7600_init(modem_dte_t *dte)
{
    modem_dce_t *dce = bg96_init(dte);
    dte->dce->get_battery_status = sim7600_get_battery_status;
    dte->dce->power_down = sim7600_power_down;
    return dce;
}


/**
 * @brief 
 *
 */

void sim7600__SetPowerPin(void)
{
    gpio_set_level(SIM7600_POWER_PIN, 1);
}

/**
 * @brief 
 *
 */

void sim7600__ClearPowerPin(void)
{
    gpio_set_level(SIM7600_POWER_PIN, 0);
}

/**
 * @brief 
 * 
 */

static esp_err_t example_default_handle(modem_dce_t *dce, const char *line)
{
    printf("===Rec Line : %s", line);
    esp_err_t err = ESP_FAIL;
    if (strstr(line, MODEM_RESULT_CODE_SUCCESS)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_SUCCESS);
    } else if (strstr(line, MODEM_RESULT_CODE_ERROR)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_FAIL);
    }
    return err;
}

bool pppStop_status = false;

bool sim7600__GetPPPStopStatus(void)
{
    return pppStop_status;
}
/**
 * @brief 
 * 
 */
static void modem_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
 {
     char mystr[250];

     switch (event_id) {
     case ESP_MODEM_EVENT_PPP_START:
         ESP_LOGI(TAG, "Modem PPP Started");
         pppStop_status = false;
         break;
     case ESP_MODEM_EVENT_PPP_STOP:
         ESP_LOGI(TAG, "Modem PPP Stopped");
         xEventGroupSetBits(event_sim7600_group, STOP_BIT);
         pppStop_status = true;
         break;
     case ESP_MODEM_EVENT_UNKNOWN:

  
        //  ESP_LOGW(TAG, "Unknow line received: %s", (char *)event_data);
        //  sprintf(mystr, "%s", (char *)event_data);
        //  ESP_LOGI(TAG,"String is : %s",mystr);
        //  if(!strcmp(mystr, "RING"))
        //  {
        //      LibGSM__DisconnectCall();
        //      incomming_call = true;
        //  }
         
         break;
     default:
         break;
     }

    //  ESP_LOGI(TAG, "Event is triggered");
 }

/**
 * @brief 
 * 
 */

 static void on_ppp_changed(void *arg, esp_event_base_t event_base,
                            int32_t event_id, void *event_data)
 {
     sim7600_ppp_state = event_id;
     ESP_LOGI(TAG, "PPP state changed event is %d", event_id);
     if (event_id == NETIF_PPP_ERRORUSER) {
         /* User interrupted event from esp-netif */
         esp_netif_t *netif = *(esp_netif_t**)event_data;
         ESP_LOGI(TAG, "User interrupted event from netif:%p", netif);
     }
 }


/**
 * @brief 
 * 
 */
 static void on_ip_event(void *arg, esp_event_base_t event_base,
                         int32_t event_id, void *event_data)
 {
     ESP_LOGD(TAG, "IP event! %d", event_id);
     if (event_id == IP_EVENT_PPP_GOT_IP) {
         esp_netif_dns_info_t dns_info;

         ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
         esp_netif_t *netif = event->esp_netif;

         ESP_LOGI(TAG, "Modem Connect to PPP Server");
         ESP_LOGI(TAG, "~~~~~~~~~~~~~~");
         ESP_LOGI(TAG, "IP          : " IPSTR, IP2STR(&event->ip_info.ip));
         ESP_LOGI(TAG, "Netmask     : " IPSTR, IP2STR(&event->ip_info.netmask));
         ESP_LOGI(TAG, "Gateway     : " IPSTR, IP2STR(&event->ip_info.gw));
         esp_netif_get_dns_info(netif, 0, &dns_info);
         ESP_LOGI(TAG, "Name Server1: " IPSTR, IP2STR(&dns_info.ip.u_addr.ip4));
         esp_netif_get_dns_info(netif, 1, &dns_info);
         ESP_LOGI(TAG, "Name Server2: " IPSTR, IP2STR(&dns_info.ip.u_addr.ip4));
         ESP_LOGI(TAG, "~~~~~~~~~~~~~~");
         
         xEventGroupSetBits(event_sim7600_group, CONNECT_BIT);

         ESP_LOGI(TAG, "GOT ip event!!!");
     } else if (event_id == IP_EVENT_PPP_LOST_IP) {
         ESP_LOGI(TAG, "Modem Disconnect from PPP Server");
     } else if (event_id == IP_EVENT_GOT_IP6) {
         ESP_LOGI(TAG, "GOT IPv6 event!");

         ip_event_got_ip6_t *event = (ip_event_got_ip6_t *)event_data;
         ESP_LOGI(TAG, "Got IPv6 address " IPV6STR, IPV62STR(event->ip6_info.ip));
     }
 }


/**
 * @brief 
 * 
 */

void sim7600__HardwareInit(void)
{

     //zero-initialize the config structure.
    gpio_config_t io_conf = {};
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = SIM7600_POWER_PIN_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 1;
    //configure GPIO with the given settings
    gpio_config(&io_conf);


     #if CONFIG_LWIP_PPP_PAP_SUPPORT
     sim7600_auth_type = NETIF_PPP_AUTHTYPE_PAP;
 #elif CONFIG_LWIP_PPP_CHAP_SUPPORT
     esp_netif_auth_type_t auth_type = NETIF_PPP_AUTHTYPE_CHAP;
 #elif !defined(CONFIG_EXAMPLE_MODEM_PPP_AUTH_NONE)
 #error "Unsupported AUTH Negotiation"
 #endif
     ESP_ERROR_CHECK(esp_netif_init());
     (esp_event_loop_create_default());
     ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &on_ip_event, NULL));
     ESP_ERROR_CHECK(esp_event_handler_register(NETIF_PPP_STATUS, ESP_EVENT_ANY_ID, &on_ppp_changed, NULL));



    event_sim7600_group = xEventGroupCreate();

    /* create dte object */
    esp_modem_dte_config_t config = ESP_MODEM_DTE_DEFAULT_CONFIG();
    /* setup UART specific configuration based on kconfig options */
    config.tx_io_num = SIM7600_RX_PIN;  //CONFIG_EXAMPLE_MODEM_UART_TX_PIN;
    config.rx_io_num = SIM7600_TX_PIN;  //CONFIG_EXAMPLE_MODEM_UART_RX_PIN;
    config.rts_io_num = CONFIG_EXAMPLE_MODEM_UART_RTS_PIN;
    config.cts_io_num = CONFIG_EXAMPLE_MODEM_UART_CTS_PIN;
    config.rx_buffer_size = CONFIG_EXAMPLE_MODEM_UART_RX_BUFFER_SIZE;
    config.tx_buffer_size = CONFIG_EXAMPLE_MODEM_UART_TX_BUFFER_SIZE;
    config.pattern_queue_size = CONFIG_EXAMPLE_MODEM_UART_PATTERN_QUEUE_SIZE;
    config.event_queue_size = CONFIG_EXAMPLE_MODEM_UART_EVENT_QUEUE_SIZE;
    config.event_task_stack_size = CONFIG_EXAMPLE_MODEM_UART_EVENT_TASK_STACK_SIZE;
    config.event_task_priority = CONFIG_EXAMPLE_MODEM_UART_EVENT_TASK_PRIORITY;
    config.line_buffer_size = CONFIG_EXAMPLE_MODEM_UART_RX_BUFFER_SIZE / 2;
    config.baud_rate = SIM7600_BAUD;

    sim7600_dte = esp_modem_dte_init(&config);

}



#if CONFIG_EXAMPLE_SEND_MSG
/**
 * @brief This example will also show how to send short message using the infrastructure provided by esp modem library.
 * @note Not all modem support SMG.
 *
 */

static esp_err_t example_handle_cmgs(modem_dce_t *dce, const char *line)
{
    esp_err_t err = ESP_FAIL;
    if (strstr(line, MODEM_RESULT_CODE_SUCCESS)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_SUCCESS);
    } else if (strstr(line, MODEM_RESULT_CODE_ERROR)) {
        err = esp_modem_process_command_done(dce, MODEM_STATE_FAIL);
    } else if (!strncmp(line, "+CMGS", strlen("+CMGS"))) {
        err = ESP_OK;
    }
    return err;
}

#define MODEM_SMS_MAX_LENGTH (128)
#define MODEM_COMMAND_TIMEOUT_SMS_MS (120000)
#define MODEM_PROMPT_TIMEOUT_MS (100)

static esp_err_t example_send_message_text(modem_dce_t *dce, const char *phone_num, const char *text)
{
    modem_dte_t *dte = dce->dte;
    dce->handle_line = example_default_handle;
    /* Set text mode */
    if (dte->send_cmd(dte, "AT+CMGF=1\r", MODEM_COMMAND_TIMEOUT_DEFAULT) != ESP_OK) {
        ESP_LOGE(TAG, "send command failed");
        goto err;
    }
    if (dce->state != MODEM_STATE_SUCCESS) {
        ESP_LOGE(TAG, "set message format failed");
        goto err;
    }
    ESP_LOGD(TAG, "set message format ok");
    /* Specify character set */ 
    dce->handle_line = example_default_handle;
    if (dte->send_cmd(dte, "AT+CSCS=\"GSM\"\r", MODEM_COMMAND_TIMEOUT_DEFAULT) != ESP_OK) {
        ESP_LOGE(TAG, "send command failed");
        goto err;
    }
    if (dce->state != MODEM_STATE_SUCCESS) {
        ESP_LOGE(TAG, "set character set failed");
        goto err;
    }
    ESP_LOGD(TAG, "set character set ok");
    /* send message */
    char command[MODEM_SMS_MAX_LENGTH] = {0};
    int length = snprintf(command, MODEM_SMS_MAX_LENGTH, "AT+CMGS=\"%s\"\r", phone_num);
    /* set phone number and wait for "> " */
    dte->send_wait(dte, command, length, "\r\n> ", MODEM_PROMPT_TIMEOUT_MS);
    /* end with CTRL+Z */
    snprintf(command, MODEM_SMS_MAX_LENGTH, "%s\x1A", text);
    dce->handle_line = example_handle_cmgs;
    if (dte->send_cmd(dte, command, MODEM_COMMAND_TIMEOUT_SMS_MS) != ESP_OK) {
        ESP_LOGE(TAG, "send command failed");
        goto err;
    }
    if (dce->state != MODEM_STATE_SUCCESS) {
        ESP_LOGE(TAG, "send message failed");
        goto err;
    }
    ESP_LOGD(TAG, "send message ok");
    return ESP_OK;
err:
    return ESP_FAIL;
}
#endif

esp_err_t sim7600__NetTimeSetup(void)
{
    modem_dte_t *dte = sim7600_dce->dte;
    sim7600_dce->handle_line = example_default_handle;
    /* Set text mode */
    if (sim7600_dte->send_cmd(sim7600_dte, "ATE0\r", MODEM_COMMAND_TIMEOUT_DEFAULT) != ESP_OK) {
        ESP_LOGE(TAG, "send ATE0 command failed");
        goto err;
    }
    if (sim7600_dce->state != MODEM_STATE_SUCCESS) {
        ESP_LOGE(TAG, "set ATE0 cmd failed");
        goto err;
    }
    ESP_LOGD(TAG, "set AT ok");


    /*  */ 
    sim7600_dce->handle_line = example_default_handle;
    if (sim7600_dte->send_cmd(sim7600_dte, "AT+CCLK?\r", 5000) != ESP_OK) {
        ESP_LOGE(TAG, "send AT+CCLK? failed");
        goto err;
    }
    if (sim7600_dce->state != MODEM_STATE_SUCCESS) {
        ESP_LOGE(TAG, "set AT+CCLK? failed");
        // goto err;
    }


    /*  */ 
    sim7600_dce->handle_line = example_default_handle;
    if (sim7600_dte->send_cmd(sim7600_dte, "AT+CLTS=1\r", 5000) != ESP_OK) {
        ESP_LOGE(TAG, "send AT+CLTS failed");
        goto err;
    }
    if (sim7600_dce->state != MODEM_STATE_SUCCESS) {
        ESP_LOGE(TAG, "set AT+CLTS failed");
        // goto err;
    }

    /*  */ 
    sim7600_dce->handle_line = example_default_handle;
    if (sim7600_dte->send_cmd(sim7600_dte, "AT&W\r", 5000) != ESP_OK) {
        ESP_LOGE(TAG, "send AT&W failed");
        goto err;
    }
    if (sim7600_dce->state != MODEM_STATE_SUCCESS) {
        ESP_LOGE(TAG, "set AT&W failed");
        // goto err;
    }



    /*  */ 
    // sim7600_dce->handle_line = example_default_handle;
    // if (sim7600_dte->send_cmd(sim7600_dte, "AT+QNITZ=1\r", 5000) != ESP_OK) {
    //     ESP_LOGE(TAG, "send AT+QNITZ failed");
    //     goto err;
    // }
    // if (sim7600_dce->state != MODEM_STATE_SUCCESS) {
    //     ESP_LOGE(TAG, "set AT+QNITZ failed");
    //     // goto err;
    // }

    /*  */ 
    sim7600_dce->handle_line = example_default_handle;
    if (sim7600_dte->send_cmd(sim7600_dte, "AT+CTZU=1\r", MODEM_COMMAND_TIMEOUT_DEFAULT) != ESP_OK) {
        ESP_LOGE(TAG, "send AT+CTZU=1 failed");
        goto err;
    }
    if (sim7600_dce->state != MODEM_STATE_SUCCESS) {
        ESP_LOGE(TAG, "set AT+CTZU=1 failed");
        goto err;
    }


    /*  */ 
    // sim7600_dce->handle_line = example_default_handle;
    // if (sim7600_dte->send_cmd(sim7600_dte, "AT+CFUN=1,1\r", MODEM_COMMAND_TIMEOUT_DEFAULT) != ESP_OK) {
    //     ESP_LOGE(TAG, "send AT+CFUN=1,1 failed");
    //     goto err;
    // }
    // if (sim7600_dce->state != MODEM_STATE_SUCCESS) {
    //     ESP_LOGE(TAG, "set AT+CFUN=1,1 failed");
    //     goto err;
    // }

    printf("All Good");


    return ESP_OK;
err:
    return ESP_FAIL;
}


esp_err_t sim7600__ConnectMqttServer(void)
{
//     char ATcommand[80];


    
//     modem_dte_t *dte = sim7600_dce->dte;
//     sim7600_dce->handle_line = example_default_handle;
//     /* Set text mode */
//     if (sim7600_dte->send_cmd(sim7600_dte, "ATE0\r", MODEM_COMMAND_TIMEOUT_DEFAULT) != ESP_OK) {
//         ESP_LOGE(TAG, "send ATE0 command failed");
//         goto err;
//     }
//     if (sim7600_dce->state != MODEM_STATE_SUCCESS) {
//         ESP_LOGE(TAG, "set ATE0 cmd failed");
//         goto err;
//     }

//     sprintf(ATcommand,"AT+CGSOCKCONT=1,\"IP\",\"%s\"\r\n","airtelgprs.com"); // Specify the value of  PDP context
	

//     if (sim7600_dte->send_cmd(sim7600_dte, ATcommand, MODEM_COMMAND_TIMEOUT_DEFAULT) != ESP_OK) {
//         ESP_LOGE(TAG, "send ATE0 command failed");
//         goto err;
//     }
//     if (sim7600_dce->state != MODEM_STATE_SUCCESS) {
//         ESP_LOGE(TAG, "set ATE0 cmd failed");
//         goto err;
//     }

//     if (sim7600_dte->send_cmd(sim7600_dte, "AT+CMQTTACCQ=0,\"client01\"\r\n", MODEM_COMMAND_TIMEOUT_DEFAULT) != ESP_OK) {
//         ESP_LOGE(TAG, "send ATE0 command failed");
//         goto err;
//     }
//     if (sim7600_dce->state != MODEM_STATE_SUCCESS) {
//         ESP_LOGE(TAG, "set ATE0 cmd failed");
//         goto err;
//     }
    

//     // sprintf(ATcommand,"AT+CMQTTCONNECT=0,\"%s:%d\",60,1\r\n","abhed.nexlein.com",1883,"apachet20dun","c3Flx12Q"); // Connect to a MQTT Server
// 	sprintf(ATcommand,"AT+CMQTTCONNECT=0,\"%s:%d\",60,1,%s,%s\r\n","abhed.nexlein.com",1883,"boot:apachet20dun","c3Flx12Q");
// 	if (sim7600_dte->send_cmd(sim7600_dte, ATcommand, MODEM_COMMAND_TIMEOUT_DEFAULT) != ESP_OK) {
//         ESP_LOGE(TAG, "send ATE0 command failed");
//         goto err;
//     }
//     if (sim7600_dce->state != MODEM_STATE_SUCCESS) {
//         ESP_LOGE(TAG, "set ATE0 cmd failed");
//         goto err;
//     }
// 	sprintf(ATcommand,"AT+CMQTTTOPIC=0,%d\r\n",strlen("apachet20dun")); // Set the topic for publish message
// 	if (sim7600_dte->send_cmd(sim7600_dte, ATcommand, MODEM_COMMAND_TIMEOUT_DEFAULT) != ESP_OK) {
//         ESP_LOGE(TAG, "send ATE0 command failed");
//         goto err;
//     }
//     if (sim7600_dce->state != MODEM_STATE_SUCCESS) {
//         ESP_LOGE(TAG, "set ATE0 cmd failed");
//         goto err;
//     }
// 	sprintf(ATcommand,"%s\r\n","apachet20dun");
// 	if (sim7600_dte->send_cmd(sim7600_dte, ATcommand, MODEM_COMMAND_TIMEOUT_DEFAULT) != ESP_OK) {
//         ESP_LOGE(TAG, "send ATE0 command failed");
//         goto err;
//     }
//     if (sim7600_dce->state != MODEM_STATE_SUCCESS) {
//         ESP_LOGE(TAG, "set ATE0 cmd failed");
//         goto err;
//     }
// 	sprintf(ATcommand,"AT+CMQTTPAYLOAD=0,%d\r\n",strlen("Hello test new message")-2); // Set the payload
// 	if (sim7600_dte->send_cmd(sim7600_dte, ATcommand, MODEM_COMMAND_TIMEOUT_DEFAULT) != ESP_OK) {
//         ESP_LOGE(TAG, "send ATE0 command failed");
//         goto err;
//     }
//     if (sim7600_dce->state != MODEM_STATE_SUCCESS) {
//         ESP_LOGE(TAG, "set ATE0 cmd failed");
//         goto err;
//     }

//     if (sim7600_dte->send_cmd(sim7600_dte, "Hello test new message", MODEM_COMMAND_TIMEOUT_DEFAULT) != ESP_OK) {
//         ESP_LOGE(TAG, "send ATE0 command failed");
//         goto err;
//     }
//     if (sim7600_dce->state != MODEM_STATE_SUCCESS) {
//         ESP_LOGE(TAG, "set ATE0 cmd failed");
//         goto err;
//     }

//     if (sim7600_dte->send_cmd(sim7600_dte, "AT+CMQTTPUB=0,1,60\r\n", MODEM_COMMAND_TIMEOUT_DEFAULT) != ESP_OK) {
//         ESP_LOGE(TAG, "send ATE0 command failed");
//         goto err;
//     }
//     if (sim7600_dce->state != MODEM_STATE_SUCCESS) {
//         ESP_LOGE(TAG, "set ATE0 cmd failed");
//         goto err;
//     }

//     if (sim7600_dte->send_cmd(sim7600_dte, "AT+CMQTTDISC=0,120\r\n", MODEM_COMMAND_TIMEOUT_DEFAULT) != ESP_OK) {
//         ESP_LOGE(TAG, "send ATE0 command failed");
//         goto err;
//     }
//     if (sim7600_dce->state != MODEM_STATE_SUCCESS) {
//         ESP_LOGE(TAG, "set ATE0 cmd failed");
//         goto err;
//     }

//     if (sim7600_dte->send_cmd(sim7600_dte, "AT+CMQTTREL=0\r\n", MODEM_COMMAND_TIMEOUT_DEFAULT) != ESP_OK) {
//         ESP_LOGE(TAG, "send ATE0 command failed");
//         goto err;
//     }
//     if (sim7600_dce->state != MODEM_STATE_SUCCESS) {
//         ESP_LOGE(TAG, "set ATE0 cmd failed");
//         goto err;
//     }

//      if (sim7600_dte->send_cmd(sim7600_dte, "AT+CMQTTSTOP\r\n", MODEM_COMMAND_TIMEOUT_DEFAULT) != ESP_OK) {
//         ESP_LOGE(TAG, "send ATE0 command failed");
//         goto err;
//     }
//     if (sim7600_dce->state != MODEM_STATE_SUCCESS) {
//         ESP_LOGE(TAG, "set ATE0 cmd failed");
//         goto err;
//     }


    





// //     printf("All Good");


    return ESP_OK;
// err:
//     return ESP_FAIL;
}

void sim7600__StopPPP(void)
{
    vTaskDelay(pdMS_TO_TICKS(500));
    esp_mqtt_client_destroy(mqtt_client);
    vTaskDelay(pdMS_TO_TICKS(500));

        /* Exit PPP mode */
    ESP_ERROR_CHECK(esp_modem_stop_ppp(sim7600_dte));
    vTaskDelay(pdMS_TO_TICKS(500));
    xEventGroupWaitBits(event_sim7600_group, STOP_BIT, pdTRUE, pdTRUE, portMAX_DELAY);
    vTaskDelay(pdMS_TO_TICKS(500));
}

/**
 * @brief 
 * 
 */

void sim7600__ModemBootsup(void)
{
    /* Register event handler */
    (esp_modem_set_event_handler(sim7600_dte, modem_event_handler, ESP_EVENT_ANY_ID, NULL));

    // Init netif object
    esp_netif_config_t sim7600_cfg = ESP_NETIF_DEFAULT_PPP();
    sim7600_esp_netif = esp_netif_new(&sim7600_cfg);

    assert(sim7600_esp_netif);

    sim7600_modem_netif_adapter = esp_modem_netif_setup(sim7600_dte);
    esp_modem_netif_set_default_handlers(sim7600_modem_netif_adapter, sim7600_esp_netif);


    reint_modem:
    // recheck_opr_name:

    sim7600_dce = sim7600_init(sim7600_dte);

    if(sim7600_dce == NULL)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        goto reint_modem;
    }

    assert(sim7600_dce != NULL);


    (sim7600_dce->set_flow_ctrl(sim7600_dce, MODEM_FLOW_CONTROL_NONE));
    (sim7600_dce->store_profile(sim7600_dce));
    /* Print Module ID, Operator, IMEI, IMSI */
    ESP_LOGI(TAG, "Module: %s", sim7600_dce->name);
    ESP_LOGI(TAG, "Operator: %s", sim7600_dce->oper);
    ESP_LOGI(TAG, "IMEI: %s", sim7600_dce->imei);
    ESP_LOGI(TAG, "IMSI: %s", sim7600_dce->imsi);

    /* Get signal quality */
    uint32_t rssi = 0, ber = 0;
    ESP_ERROR_CHECK(sim7600_dce->get_signal_quality(sim7600_dce, &rssi, &ber));
    ESP_LOGI(TAG, "rssi: %d, ber: %d", rssi, ber);
    
    sim7600_network_status = true;

    if(!strcmp (sim7600_dce->oper,""))
    {
        ESP_LOGI(TAG, "Re-checking Operator Name");
        sim7600_network_status = false;
        // goto recheck_opr_name;
    } 

}

void sim7600__NetworkCheck(void)
{
    ESP_LOGI(TAG,"Rechecking Network");
    
    reint_modem:
    
    sim7600_dce = sim7600_init(sim7600_dte);

    if(sim7600_dce == NULL)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        goto reint_modem;
    }

    assert(sim7600_dce != NULL);

    (sim7600_dce->set_flow_ctrl(sim7600_dce, MODEM_FLOW_CONTROL_NONE));
    (sim7600_dce->store_profile(sim7600_dce));
    /* Print Module ID, Operator, IMEI, IMSI */
    ESP_LOGI(TAG, "Module: %s", sim7600_dce->name);
    ESP_LOGI(TAG, "Operator: %s", sim7600_dce->oper);
    ESP_LOGI(TAG, "IMEI: %s", sim7600_dce->imei);
    ESP_LOGI(TAG, "IMSI: %s", sim7600_dce->imsi);

    /* Get signal quality */
    uint32_t rssi = 0, ber = 0;
    (sim7600_dce->get_signal_quality(sim7600_dce, &rssi, &ber));
    ESP_LOGI(TAG, "rssi: %d, ber: %d", rssi, ber);


}



SemaphoreHandle_t xSemaphore = NULL;

void IRAM_ATTR sim7600__Ring_Intr_Handler(void* arg) {
	
    // notify the button task
	xSemaphoreGiveFromISR(xSemaphore, NULL);
}

uint32_t sim7600__GetPPPState(void)
{
    return sim7600_ppp_state;
}

bool sim7600__GetNewSmsReceivedStatus(void)
{
    return sim7600_new_sms_received;
}

void sim7600__SetNewSmsReceivedStatus(bool status)
{
    sim7600_new_sms_received = status;
}


void sim7600__Ring_Task(void* arg) {
	
	// infinite loop
	for(;;) {
		
		// wait for the notification from the ISR
		if(xSemaphoreTake(xSemaphore,portMAX_DELAY) == pdTRUE) {
			printf("SMS / CALL is detected\n");

            // if(sim7600__GetSystemBootStatus() == false)
            // {
            //     sim7600__SetSystemBootStatus(true);
            // }
            // else
            {
                // vTaskDelay(1000 / portTICK_RATE_MS);

                // if(incomming_call == true)
                // {
                    
                //     incomming_call = false;

                // }
                // else
                {
                    sim7600__DisableRingInterrupt();
                    sim7600_new_sms_received = true;

                }
                
                
            }

            
			
		}
	}
}

void sim7600__EnableRingInterrupt(void)
{

    LibGSM__CNMIEnable();


    // create the binary semaphore
	xSemaphore = xSemaphoreCreateBinary();
	
	// configure button and led pins as GPIO pins
	gpio_pad_select_gpio(SIM7600_RING);
	// gpio_pad_select_gpio(CONFIG_LED_PIN);
	
	// set the correct direction
	gpio_set_direction(SIM7600_RING, GPIO_MODE_INPUT);
    // gpio_set_direction(CONFIG_LED_PIN, GPIO_MODE_OUTPUT);
	
	// enable interrupt on falling (1->0) edge for button pin
	gpio_set_intr_type(SIM7600_RING, GPIO_INTR_NEGEDGE);
	
	// start the task that will handle the button
	xTaskCreate(sim7600__Ring_Task, "sim7600_ring_task", 1024*4, NULL, 10, NULL);
	
	// install ISR service with default configuration
	gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
	
	// attach the interrupt service routine
	gpio_isr_handler_add(SIM7600_RING, sim7600__Ring_Intr_Handler, NULL);

    intteruptDisabled_status = false;
}

void sim7600__DisableRingInterrupt(void)
{

    LibGSM__CNMIDisable();

  
    // configure button and led pins as GPIO pins
	gpio_pad_select_gpio(SIM7600_RING);
	// gpio_pad_select_gpio(CONFIG_LED_PIN);
	
	// set the correct direction
	gpio_set_direction(SIM7600_RING, GPIO_MODE_INPUT);
    // gpio_set_direction(CONFIG_LED_PIN, GPIO_MODE_OUTPUT);
	
	// enable interrupt on falling (1->0) edge for button pin
	gpio_intr_disable(SIM7600_RING);
	
	// start the task that will handle the button
	// xTaskCreate(sim7600__Ring_Task, "sim7600_ring_task", 2048, NULL, 10, NULL);
	
	// // install ISR service with default configuration
	// gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
	
	// // attach the interrupt service routine
	// gpio_isr_handler_add(SIM7600_RING, sim7600__Ring_Intr_Handler, NULL);

    intteruptDisabled_status = true;
}

bool sim7600__GetMqttConnectStatus(void)
{
    return mqtt_connected;
}

bool sim7600__GetMqttSubscribeStatus(void)
{
    return mqtt_subscribed;
}

bool reconnect_bit = false;

void sim7600__Setreconnectbit(bool setb)
{
    reconnect_bit = setb;
}

void sim7600__ReConnectMQTT(void )
{

    ESP_LOGI(TAG, "====RECONNECTING  MQTT CONN====");

    
    /* setup PPPoS network parameters */
    #if !defined(CONFIG_EXAMPLE_MODEM_PPP_AUTH_NONE) && (defined(CONFIG_LWIP_PPP_PAP_SUPPORT) || defined(CONFIG_LWIP_PPP_CHAP_SUPPORT))
            esp_netif_ppp_set_auth(sim7600_esp_netif, sim7600_auth_type, CONFIG_EXAMPLE_MODEM_PPP_AUTH_USERNAME, CONFIG_EXAMPLE_MODEM_PPP_AUTH_PASSWORD);
    #endif


}


void sim7600__ConnectMQTT(void)
{

    ESP_LOGI(TAG, "====SETTING UP MQTT CONN====");

    
    /* setup PPPoS network parameters */
    #if !defined(CONFIG_EXAMPLE_MODEM_PPP_AUTH_NONE) && (defined(CONFIG_LWIP_PPP_PAP_SUPPORT) || defined(CONFIG_LWIP_PPP_CHAP_SUPPORT))
            esp_netif_ppp_set_auth(sim7600_esp_netif, sim7600_auth_type, CONFIG_EXAMPLE_MODEM_PPP_AUTH_USERNAME, CONFIG_EXAMPLE_MODEM_PPP_AUTH_PASSWORD);
    #endif


            /* attach the modem to the network interface */
            esp_netif_attach(sim7600_esp_netif, sim7600_modem_netif_adapter);
    
            /* Wait for IP address */
            xEventGroupWaitBits(event_sim7600_group, CONNECT_BIT, pdTRUE, pdTRUE, portMAX_DELAY);

    
    char * host_url;char * vhost; char * portnum; char * host_username; char * host_password;
    GsmGprsManager__GetMQTTConfiguration(&host_url, &vhost, &portnum, &host_username, &host_password);

    ESP_LOGI(TAG,"CONN Params: un = %s, pass = %s, url =%s, vhost = %s",host_username, host_password, host_url ,vhost);
        
        reconnect_bit = true;

        char TEMP[100]="";
        strcat(TEMP,vhost);
        strcat(TEMP,":");
        strcat(TEMP,host_username);

        // strcat(MQTT_VHOST,":");
        // strcat(MQTT_VHOST,MQTT_USERNAME);
            /* Config MQTT */
        esp_mqtt_client_config_t mqtt_config = {
            .uri = host_url,//MQTT_BROKER_URL,
            .port = atoi(portnum),//1883,
            .username = TEMP,//"boot:apachet20dun",
            .password = host_password,//"c3Flx12Q",
            .keepalive = 3600,
            .event_handle = mqtt_event_handler,
        };

            mqtt_client = esp_mqtt_client_init(&mqtt_config);
            esp_mqtt_client_start(mqtt_client);
            xEventGroupWaitBits(event_sim7600_group, GOT_DATA_BIT, pdTRUE, pdTRUE, portMAX_DELAY);

}

bool bootup_subscribe = false;



static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
 {
     client = event->client;
     char * message_1;char * message_2; char * topic_1; char * topic_2;

      GsmGprsManager__GetTopicsSend(&topic_1, &topic_2);

      GsmGprsManager__GetMsgToSend(&message_1, &message_2);
     
     switch (event->event_id) {
     case MQTT_EVENT_CONNECTED:
         ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
         
 
            
            // ESP_LOGI(TAG,"top1 = %s", topic_1);
            // ESP_LOGI(TAG,"top2 = %s", topic_2);
            
            
            // ESP_LOGI(TAG,"msg1 = %s", message_1);
            // ESP_LOGI(TAG,"msg2 = %s", message_2);

  
    
        if(!strcmp(topic_2, "NULL"))
        {
            ESP_LOGI(TAG, "TOPIC 2 is null");
            if(!strcmp(message_2, "NULL"))
            {
                msg_id = esp_mqtt_client_subscribe(client, topic_1, 0);
                ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
            }
            else
            {
                msg_id = esp_mqtt_client_subscribe(client, topic_1, 0);
                ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

            }

        }
        else
        {
            msg_id = esp_mqtt_client_subscribe(client, topic_2, 0);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
        }
   
         mqtt_connected = true;
         break;
     case MQTT_EVENT_DISCONNECTED:
            mqtt_connected = false;
            mqtt_subscribed = false;
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");


         break;

     case MQTT_EVENT_SUBSCRIBED:

            mqtt_subscribed = true;

            
            if(!strcmp(topic_2, "NULL"))
            {
                ESP_LOGI(TAG, "TOPIC 2 is null");
                if(!strcmp(message_2, "NULL"))
                {
                    ESP_LOGI(TAG, "message 2 is null");
                    msg_id = esp_mqtt_client_publish(client, topic_1, message_1, 0, 0, 0);

                }
                else
                {
                    
                    ESP_LOGI(TAG, "publishing 2 messages");

                    msg_id = esp_mqtt_client_publish(client, topic_1, message_1, 0, 0, 0);
                    vTaskDelay(1000 / portTICK_RATE_MS);
                    msg_id = esp_mqtt_client_publish(client, topic_1, message_2, 0, 0, 0);
                    vTaskDelay(1000 / portTICK_RATE_MS);

                }

            }
            else
            {
                // send handshake message first "Waiting for data"
                msg_id = esp_mqtt_client_publish(client, topic_2, HANDSHAKE_MSG, 0, 0, 0);
            }

            

 

         break;
     case MQTT_EVENT_UNSUBSCRIBED:
        mqtt_subscribed  = false;
         ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
         break;
     case MQTT_EVENT_PUBLISHED:
         ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
         break;
     case MQTT_EVENT_DATA:
         ESP_LOGI(TAG, "MQTT_EVENT_DATA");
         printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
         printf("DATA=%.*s\r\n", event->data_len, event->data);

         
         if(GsmGprsManager__GetState() == STATE_GSMGPRS_CONNECT_NEW_TOPIC )
         {
             char temp_topic[50];
             char temp_data[2000];
             sprintf(temp_topic,"%.*s",event->topic_len, event->topic);
             sprintf(temp_data,"%.*s",event->data_len, event->data);

             char *cmd;
             GsmGprs__GetCurrentCMD(&cmd);

             if(strcmp(temp_data,HANDSHAKE_MSG))
             {
                 //Received data should not ne Handshake message
                 if((!strcmp(temp_topic,topic_2)) && (!strcmp(cmd,"SOTP")) )
                {
                    SDCard__WriteToOTPFile("/root/Configuration/otp.txt", temp_data);


                    ESP_LOGI(TAG, "publish feedback to temp queue");
                    msg_id = esp_mqtt_client_publish(client, topic_2, CONFIGURATION_UPDATE_MSG, 0, 0, 0);

                    ESP_LOGI(TAG, "publish feedback to original queue");
                    msg_id = esp_mqtt_client_publish(client, topic_1, message_1, 0, 0, 0);

                    
                    Timers__MsExpire(MS__TIMER_MQTT_WAIT_BEFORE_DISCONNECT);
                }

                if((!strcmp(temp_topic,topic_2)) && (!strcmp(cmd,"SCFG")) )
                {
                    SDCard__WriteToConfigFile("/root/Configuration/Abhed.txt", temp_data);

                    ESP_LOGI(TAG, "publish feedback to temp queue");
                    msg_id = esp_mqtt_client_publish(client, topic_2, CONFIGURATION_UPDATE_MSG, 0, 0, 0);

                    ESP_LOGI(TAG, "publish feedback to original queue");
                    msg_id = esp_mqtt_client_publish(client, topic_1, message_1, 0, 0, 0);

                    Timers__MsExpire(MS__TIMER_MQTT_WAIT_BEFORE_DISCONNECT);
                }

             }

         }

         

         xEventGroupSetBits(event_sim7600_group, GOT_DATA_BIT);

         break;
     case MQTT_EVENT_ERROR:
         ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
         break;
     default:
         ESP_LOGI(TAG, "MQTT other event id: %d", event->event_id);
         break;
     }
     return ESP_OK;
 }

void sim7600__SetMqttMsgSendStatus(bool status)
{
    mqtt_msg_send_status = status;
}
bool sim7600__GetMqttMsgSendStatus()
{
    return mqtt_msg_send_status;
}


void sim7600__mqttConnectToQueue(char * new_queue)
{

    ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
    msg_id = esp_mqtt_client_subscribe(client, new_queue, 0);// /topic/esp-pppos  
    ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

    // msg_id = esp_mqtt_client_publish(client, "apachet20dt", _json_message, 0, 0, 0);
    // ESP_LOGI(TAG, "Diagnostics message sent publish successful, msg_id=%d", msg_id);
    // mqtt_msg_send_status = true; //message sent successfully

}

void sim7600__SendMqttMsg(char * _json_message)
{
    msg_id = esp_mqtt_client_publish(client, "apachet20dt", _json_message, 0, 0, 0);
    ESP_LOGI(TAG, "Diagnostics message sent publish successful, msg_id=%d", msg_id);
    mqtt_msg_send_status = true; //message sent successfully

}

esp_err_t sim7600__DeleteAllSms(void)
{
    /* delete all sms */
    sim7600_dce->handle_line = example_default_handle;
    if (sim7600_dte->send_cmd(sim7600_dte, "AT+CMGD=1,4\r", MODEM_COMMAND_TIMEOUT_DEFAULT) != ESP_OK) {
        ESP_LOGI(TAG, "send delete all msg command failed");
        goto err;
    }
    return ESP_OK;
    err:
    ESP_LOGI(TAG, "Delete sms command failed");
    return ESP_FAIL;

}




void sim7600__GetImei(char **buff)
{
    *buff = sim7600_dce->imei;
}

void sim7600__GetOperator(char ** buff)
{
    *buff =  sim7600_dce->oper;
}

void *sim7600__GetOperatorNew(void)
{
    return sim7600_dce->oper;
}

void sim7600__GetImsi(char **buff)
{
    *buff =  sim7600_dce->imsi;
}
uint32_t sim7600__GetRssi(void)
{
    uint32_t rssi = 0, ber = 0;
    char str_rssi[10];
    ESP_ERROR_CHECK(sim7600_dce->get_signal_quality(sim7600_dce, &rssi, &ber));
    // ESP_LOGI(TAG, "rssi: %d, ber: %d", rssi, ber);
    return  rssi;
}

 LST_CMD_TYPE lastcmd;

 LST_CMD_TYPE sim7600__GetLastSmsCmd(void)
 {
     return lastcmd;
 }

 void sim7600__SetLastSmsCmd(LST_CMD_TYPE cmd)
 {
     lastcmd = cmd;
 }


 bool sim7600__Getinterruptdisable_status(void)
 {
     return intteruptDisabled_status;
 }

 bool sim7600__GetSystemBootStatus(void)
 {
     return _system_boot;
 }
 void sim7600__SetSystemBootStatus(bool status)
 {
     _system_boot = status;
 }