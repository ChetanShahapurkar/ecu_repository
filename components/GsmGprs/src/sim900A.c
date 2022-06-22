#include "../../HardwareConfig.h"

#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_modem_dce_service.h"
#include "bg96.h"
#include "driver/gpio.h"
#include "esp_netif_ppp.h"
#include "esp_log.h"
#include "sim900A.h"
#include "sim800.h"

#include "esp_netif.h"
#include "mqtt_client.h"
#include "esp_modem.h"
#include "esp_modem_netif.h"


#include "libGSM.h"



//=================================================== PUBLIC VARIABLES=======================================================
modem_dce_t *sim900a_dce;
modem_dte_t *sim900a_dte;
esp_netif_t *sim900a_esp_netif;
void *sim900a_modem_netif_adapter;
esp_netif_auth_type_t sim900a_auth_type;
static EventGroupHandle_t event_sim900a_group = NULL;
bool sim900a_network_status = false;


//=================================================== PRIVATE VARIABLES=======================================================

 static const char *TAG = "SIM900A";
 static const int CONNECT_BIT = BIT0;
 static const int STOP_BIT = BIT1;
//  static const int GOT_DATA_BIT = BIT2;


/**
 * @brief 
 *
 */

void sim900A__SetPowerPin(void)
{
    gpio_set_level(SIM900_POWER_PIN, 1);
}

/**
 * @brief 
 *
 */

void sim900A__ClearPowerPin(void)
{
    gpio_set_level(SIM900_POWER_PIN, 0);
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
         break;
     case ESP_MODEM_EVENT_PPP_STOP:
         ESP_LOGI(TAG, "Modem PPP Stopped");
         xEventGroupSetBits(event_sim900a_group, STOP_BIT);
         break;
     case ESP_MODEM_EVENT_UNKNOWN:
         
         ESP_LOGW(TAG, "Unknow line received: %s", (char *)event_data);

         sprintf(mystr, "%s", (char *)event_data);

         ESP_LOGI(TAG,"String is : %s",mystr);
         
         break;
     default:
         break;
     }

     ESP_LOGI(TAG, "Event is triggered");
 }

/**
 * @brief 
 * 
 */

 static void sim900a_on_ppp_changed(void *arg, esp_event_base_t event_base,
                            int32_t event_id, void *event_data)
 {
     ESP_LOGI(TAG, "PPP state changed event %d", event_id);
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
 static void sim900a_on_ip_event(void *arg, esp_event_base_t event_base,
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
         
         xEventGroupSetBits(event_sim900a_group, CONNECT_BIT);

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
void sim900A__HardwareInit()
{

     //zero-initialize the config structure.
    gpio_config_t io_conf = {};
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = SIM900_POWER_PIN_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 1;
    //configure GPIO with the given settings
    gpio_config(&io_conf);


    #if CONFIG_LWIP_PPP_PAP_SUPPORT
     sim900a_auth_type = NETIF_PPP_AUTHTYPE_PAP;
 #elif CONFIG_LWIP_PPP_CHAP_SUPPORT
     esp_netif_auth_type_t auth_type = NETIF_PPP_AUTHTYPE_CHAP;
 #elif !defined(CONFIG_EXAMPLE_MODEM_PPP_AUTH_NONE)
 #error "Unsupported AUTH Negotiation"
 #endif
 
     ESP_ERROR_CHECK(esp_netif_init());
     (esp_event_loop_create_default());
     ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &sim900a_on_ip_event, NULL));
     ESP_ERROR_CHECK(esp_event_handler_register(NETIF_PPP_STATUS, ESP_EVENT_ANY_ID, &sim900a_on_ppp_changed, NULL));

    event_sim900a_group = xEventGroupCreate();

    /* create dte object */
    esp_modem_dte_config_t config = ESP_MODEM_DTE_DEFAULT_CONFIG();
    /* setup UART specific configuration based on kconfig options */
    config.tx_io_num = SIM900_RX_PIN;   //CONFIG_EXAMPLE_MODEM_UART_TX_PIN;
    config.rx_io_num = SIM900_TX_PIN;   //CONFIG_EXAMPLE_MODEM_UART_RX_PIN;
    config.rts_io_num = CONFIG_EXAMPLE_MODEM_UART_RTS_PIN;
    config.cts_io_num = CONFIG_EXAMPLE_MODEM_UART_CTS_PIN;
    config.rx_buffer_size = CONFIG_EXAMPLE_MODEM_UART_RX_BUFFER_SIZE;
    config.tx_buffer_size = CONFIG_EXAMPLE_MODEM_UART_TX_BUFFER_SIZE;
    config.pattern_queue_size = CONFIG_EXAMPLE_MODEM_UART_PATTERN_QUEUE_SIZE;
    config.event_queue_size = CONFIG_EXAMPLE_MODEM_UART_EVENT_QUEUE_SIZE;
    config.event_task_stack_size = CONFIG_EXAMPLE_MODEM_UART_EVENT_TASK_STACK_SIZE;
    config.event_task_priority = CONFIG_EXAMPLE_MODEM_UART_EVENT_TASK_PRIORITY;
    config.line_buffer_size = CONFIG_EXAMPLE_MODEM_UART_RX_BUFFER_SIZE / 2;
    config.baud_rate = 9600;
    config.port_num = UART_NUM_2;

    sim900a_dte = esp_modem_dte_init(&config);


}



/**
 * @brief 
 * 
 */

void sim900A__ModemBootsup(void)
{
    /* Register event handler */
    (esp_modem_set_event_handler(sim900a_dte, modem_event_handler, ESP_EVENT_ANY_ID, NULL));

    reint_modem:
    // recheck_opr_name:

    sim900a_dce = sim800_init(sim900a_dte);

    if(sim900a_dce == NULL)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        goto reint_modem;
    }

    assert(sim900a_dce != NULL);


    ESP_ERROR_CHECK(sim900a_dce->set_flow_ctrl(sim900a_dce, MODEM_FLOW_CONTROL_NONE));
    ESP_ERROR_CHECK(sim900a_dce->store_profile(sim900a_dce));
    /* Print Module ID, Operator, IMEI, IMSI */
    ESP_LOGI(TAG, "Module: %s", sim900a_dce->name);
    ESP_LOGI(TAG, "Operator: %s", sim900a_dce->oper);
    ESP_LOGI(TAG, "IMEI: %s", sim900a_dce->imei);
    ESP_LOGI(TAG, "IMSI: %s", sim900a_dce->imsi);
    
    sim900a_network_status = true;

    if(!strcmp (sim900a_dce->oper,""))
    {
        ESP_LOGI(TAG, "Re-checking Operator Name");
        sim900a_network_status = false;
        // goto recheck_opr_name;
    }

}

SemaphoreHandle_t xSemaphore_sim900 = NULL;

void IRAM_ATTR sim900__Ring_Intr_Handler(void* arg) {
	
    // notify the button task
	xSemaphoreGiveFromISR(xSemaphore_sim900, NULL);
}

void sim900__Ring_Task(void* arg) {
	
	// infinite loop
	for(;;) {
		
		// wait for the notification from the ISR
		if(xSemaphoreTake(xSemaphore_sim900,portMAX_DELAY) == pdTRUE) {
			printf("SMS / CALL is detected\n");
			
		}
	}
}

void sim900__ConfigureRingInterrupt(void)
{
    // create the binary semaphore
	xSemaphore_sim900 = xSemaphoreCreateBinary();
	
	// configure button and led pins as GPIO pins
	gpio_pad_select_gpio(SIM7600_RING);
	// gpio_pad_select_gpio(CONFIG_LED_PIN);
	
	// set the correct direction
	gpio_set_direction(SIM7600_RING, GPIO_MODE_INPUT);
    // gpio_set_direction(CONFIG_LED_PIN, GPIO_MODE_OUTPUT);
	
	// enable interrupt on falling (1->0) edge for button pin
	gpio_set_intr_type(SIM7600_RING, GPIO_INTR_NEGEDGE);
	
	// start the task that will handle the button
	xTaskCreate(sim900__Ring_Task, "sim7600_ring_task", 2048, NULL, 10, NULL);
	
	// install ISR service with default configuration
	gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
	
	// attach the interrupt service routine
	gpio_isr_handler_add(SIM7600_RING, sim900__Ring_Intr_Handler, NULL);
}