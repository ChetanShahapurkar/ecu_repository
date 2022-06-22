/*
 FTP Client example.
 This example code is in the Public Domain (or CC0 licensed, at your option.)

 Unless required by applicable law or agreed to in writing, this
 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs.h"
#include "nvs_flash.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "sdmmc_cmd.h"
#include "mdns.h"
#include "esp_sntp.h"

#include "lwip/dns.h"


char File_content[800];

static const char *TAG = "MAIN";
static char *MOUNT_POINT = "/root";

bool config_file_updated = false;
EventGroupHandle_t xEventTask;

//for test
//#define CONFIG_FLASH	1
//#define CONFIG_SPI_SDCARD  1
//#define CONFIG_MMC_SDCARD  1

/* FreeRTOS event group to signal when we are connected*/
int FTP_TASK_FINISH_BIT = BIT2;
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

#if CONFIG_SPI_SDCARD
// Pin mapping when using SPI mode.
// With this mapping, SD card can be used both in SPI and 1-line SD mode.
// Note that a pull-up on CS line is required in SD mode.
#define PIN_NUM_MISO CONFIG_MISO_GPIO
#define PIN_NUM_MOSI CONFIG_MOSI_GPIO
#define PIN_NUM_SCLK CONFIG_SCLK_GPIO
#define PIN_NUM_CS   CONFIG_CS_GPIO
#define PIN_POWER    CONFIG_POWER_GPIO

// #define PIN_NUM_MISO 2
// #define PIN_NUM_MOSI 15
// #define PIN_NUM_CLK  14
// #define PIN_NUM_CS   13
#endif // CONFIG_SPI_SDCARD

static int s_retry_num = 0;

SemaphoreHandle_t Semaphore_sd = NULL;

static void event_handler(void* arg, esp_event_base_t event_base,
								int32_t event_id, void* event_data)
{
	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
		esp_wifi_connect();
	} else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
		if (s_retry_num < CONFIG_ESP_MAXIMUM_RETRY) {
			esp_wifi_connect();
			s_retry_num++;
			ESP_LOGI(TAG, "retry to connect to the AP");
		} else {
			// xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
		}
		ESP_LOGI(TAG,"connect to the AP fail");
	} else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
		ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
		ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
		s_retry_num = 0;
		xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
	}
} 

esp_err_t wifi_init_sta()
{
	esp_err_t ret_value = ESP_OK;
	s_wifi_event_group = xEventGroupCreate();
	ESP_LOGI(TAG,"ESP-IDF Ver%d.%d", ESP_IDF_VERSION_MAJOR, ESP_IDF_VERSION_MINOR);

	ESP_LOGI(TAG,"ESP-IDF esp_netif");


	//------------------------------------------
	// ESP_ERROR_CHECK(esp_netif_init());
	// ESP_ERROR_CHECK(esp_event_loop_create_default());
	(esp_netif_init());
	(esp_event_loop_create_default());
	//------------------------------------------


	esp_netif_t *netif = esp_netif_create_default_wifi_sta();

#if CONFIG_STATIC_IP

	ESP_LOGI(TAG, "CONFIG_STATIC_IP_ADDRESS=[%s]",CONFIG_STATIC_IP_ADDRESS);
	ESP_LOGI(TAG, "CONFIG_STATIC_GW_ADDRESS=[%s]",CONFIG_STATIC_GW_ADDRESS);
	ESP_LOGI(TAG, "CONFIG_STATIC_NM_ADDRESS=[%s]",CONFIG_STATIC_NM_ADDRESS);

	/* Stop DHCP client */
	ESP_ERROR_CHECK(esp_netif_dhcpc_stop(netif));
	ESP_LOGI(TAG, "Stop DHCP Services");

	/* Set STATIC IP Address */
	esp_netif_ip_info_t ip_info;
	memset(&ip_info, 0 , sizeof(esp_netif_ip_info_t));
	ip_info.ip.addr = ipaddr_addr(CONFIG_STATIC_IP_ADDRESS);
	ip_info.netmask.addr = ipaddr_addr(CONFIG_STATIC_NM_ADDRESS);
	ip_info.gw.addr = ipaddr_addr(CONFIG_STATIC_GW_ADDRESS);;
	esp_netif_set_ip_info(netif, &ip_info);

	/*
	I referred from here.
	https://www.esp32.com/viewtopic.php?t=5380
	if we should not be using DHCP (for example we are using static IP addresses),
	then we need to instruct the ESP32 of the locations of the DNS servers manually.
	Google publicly makes available two name servers with the addresses of 8.8.8.8 and 8.8.4.4.
	*/

	ip_addr_t d;
	d.type = IPADDR_TYPE_V4;
	d.u_addr.ip4.addr = 0x08080808; //8.8.8.8 dns
	dns_setserver(0, &d);
	d.u_addr.ip4.addr = 0x08080404; //8.8.4.4 dns
	dns_setserver(1, &d);

#endif // CONFIG_STATIC_IP

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_MIN_MODEM)); //TODO:

	ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
	ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

	wifi_config_t wifi_config = {
		.sta = {
			.ssid = CONFIG_ESP_WIFI_SSID,
			.password = CONFIG_ESP_WIFI_PASSWORD
		},
	};
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
	ESP_ERROR_CHECK(esp_wifi_start() );

	/* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
	 * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
	EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
		WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
		pdFALSE,			// xClearOnExit
		pdFALSE,			// xWaitForAllBits
		portMAX_DELAY);

	/* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
	 * happened. */
	if (bits & WIFI_CONNECTED_BIT) {
		ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
			 CONFIG_ESP_WIFI_SSID, CONFIG_ESP_WIFI_PASSWORD);
	} else if (bits & WIFI_FAIL_BIT) {
		ESP_LOGE(TAG, "Failed to connect to SSID:%s, password:%s",
			 CONFIG_ESP_WIFI_SSID, CONFIG_ESP_WIFI_PASSWORD);
		ret_value = ESP_FAIL;
	} else {
		ESP_LOGE(TAG, "UNEXPECTED EVENT");
		ret_value = ESP_ERR_INVALID_STATE;
	}

	ESP_LOGI(TAG, "wifi_init_sta finished.");
	ESP_LOGI(TAG, "connect to ap SSID:%s", CONFIG_ESP_WIFI_SSID);
	vEventGroupDelete(s_wifi_event_group); 
	return ret_value; 
}

#if CONFIG_FLASH
wl_handle_t mountFLASH(char * partition_label, char * mount_point) {
	ESP_LOGI(TAG, "Initializing FLASH file system");
	// To mount device we need name of device partition, define base_path
	// and allow format partition in case if it is new one and was not formated before
	const esp_vfs_fat_mount_config_t mount_config = {
		//.max_files = 4,
		.max_files = 16,
		//.max_files = 32,
		.format_if_mount_failed = true,
		.allocation_unit_size = CONFIG_WL_SECTOR_SIZE
	};
	wl_handle_t s_wl_handle;
	esp_err_t err = esp_vfs_fat_spiflash_mount(mount_point, partition_label, &mount_config, &s_wl_handle);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "Failed to mount FLASH (%s)", esp_err_to_name(err));
		return -1;
	}
	ESP_LOGI(TAG, "Mount FAT filesystem on %s", mount_point);
	ESP_LOGI(TAG, "s_wl_handle=%d",s_wl_handle);
	return s_wl_handle;
}
#endif // CONFIG_FLASH



#if CONFIG_IDF_TARGET_ESP32S2
#define SPI_DMA_CHAN host.slot
#elif CONFIG_IDF_TARGET_ESP32C3
#define SPI_DMA_CHAN SPI_DMA_CH_AUTO
#else
#define SPI_DMA_CHAN 1
#endif


#if CONFIG_SPI_SDCARD || CONFIG_MMC_SDCARD
esp_err_t mountSDCARD(char * mount_point, sdmmc_card_t * card) {
	ESP_LOGI(TAG, "Initializing SDCARD file system");
	esp_err_t ret;
	// Options for mounting the filesystem.
	// If format_if_mount_failed is set to true, SD card will be partitioned and
	// formatted in case when mounting fails.
	esp_vfs_fat_sdmmc_mount_config_t mount_config = {
		.format_if_mount_failed = true,
		//.max_files = 5,
		.max_files = 16,
		//.max_files = 32,
		.allocation_unit_size = 16 * 1024
	};
	//sdmmc_card_t* card;

#if CONFIG_MMC_SDCARD
	// Use settings defined above to initialize SD card and mount FAT filesystem.
	// Note: esp_vfs_fat_sdmmc/sdspi_mount is all-in-one convenience functions.
	// Please check its source code and implement error recovery when developing
	// production applications.

	ESP_LOGI(TAG, "Using SDMMC peripheral");
	sdmmc_host_t host = SDMMC_HOST_DEFAULT();
#if CONFIG_SDMMC_SPEED_52M
	host.max_freq_khz = SDMMC_FREQ_52M;
#elif CONFIG_SDMMC_SPEED_26M
	host.max_freq_khz = SDMMC_FREQ_26M;
#elif CONFIG_SDMMC_SPEED_HIGH
	host.max_freq_khz = SDMMC_FREQ_HIGHSPEED;
#elif CONFIG_SDMMC_SPEED_DEFAULT
	host.max_freq_khz = SDMMC_FREQ_DEFAULT;
#elif CONFIG_SDMMC_SPEED_PROBING
	host.max_freq_khz = SDMMC_FREQ_PROBING;
#endif	/* CONFIG_SDMMC_SPEED */

	// This initializes the slot without card detect (CD) and write protect (WP) signals.
	// Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
	sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

	// To use 1-line SD mode, uncomment the following line:
#if CONFIG_SDMMC_ONE_LINE_MODE
	slot_config.width = 1;
#else
	slot_config.width = 4;
#endif // CONFIG_SDMMC_ONE_LINE_MODE

	// On chips where the GPIOs used for SD card can be configured, set them in
	// the slot_config structure:
#ifdef SOC_SDMMC_USE_GPIO_MATRIX
	slot_config.clk = GPIO_NUM_14;
	slot_config.cmd = GPIO_NUM_15;
	slot_config.d0 = GPIO_NUM_2;
	slot_config.d1 = GPIO_NUM_4;
	slot_config.d2 = GPIO_NUM_12;
	slot_config.d3 = GPIO_NUM_13;
#endif // SOC_SDMMC_USE_GPIO_MATRIX

	// Enable internal pullups on enabled pins. The internal pullups
	// are insufficient however, please make sure 10k external pullups are
	// connected on the bus. This is for debug / example purpose only.
	slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;

	ret = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);

#else
	// Use settings defined above to initialize SD card and mount FAT filesystem.
	// Note: esp_vfs_fat_sdmmc/sdspi_mount is all-in-one convenience functions.
	// Please check its source code and implement error recovery when developing
	// production applications.
	ESP_LOGI(TAG, "Using SPI peripheral");

	sdmmc_host_t host = SDSPI_HOST_DEFAULT();
	spi_bus_config_t bus_cfg = {
		.mosi_io_num = PIN_NUM_MOSI,
		.miso_io_num = PIN_NUM_MISO,
		.sclk_io_num = PIN_NUM_SCLK,
		.quadwp_io_num = -1,
		.quadhd_io_num = -1,
		.max_transfer_sz = 4000,
	};
	ret = spi_bus_initialize(host.slot, &bus_cfg, SPI_DMA_CHAN);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG, "Failed to initialize bus.");
		return ret;
	}
	// This initializes the slot without card detect (CD) and write protect (WP) signals.
	// Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
	sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
	slot_config.gpio_cs = PIN_NUM_CS;
	slot_config.host_id = host.slot;

	ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);
#endif // CONFIG_MMC_SDCARD

	if (ret != ESP_OK) {
		if (ret == ESP_FAIL) {
			ESP_LOGE(TAG, "Failed to mount filesystem. "
				"If you want the card to be formatted, set format_if_mount_failed = true.");
		} else {
			ESP_LOGE(TAG, "Failed to initialize the card (%s). "
				"Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
		}
#if CONFIG_MMC_SDCARD
		ESP_LOGI(TAG, "Try setting the 1-line SD/MMC mode and lower the SD/MMC card speed.");
#endif // CONFIG_MMC_SDCARD
		return ret;
	}

	// Card has been initialized, print its properties
	sdmmc_card_print_info(stdout, card);
	ESP_LOGI(TAG, "Mounte SD card on %s", mount_point);
	return ret;
}
#endif // CONFIG_SPI_SDCARD || CONFIG_MMC_SDCARD

void initialise_mdns(void)
{
	//initialize mDNS
	ESP_ERROR_CHECK( mdns_init() );
	//set mDNS hostname (required if you want to advertise services)
	ESP_ERROR_CHECK( mdns_hostname_set(CONFIG_MDNS_HOSTNAME) );
	ESP_LOGI(TAG, "mdns hostname set to: [%s]", CONFIG_MDNS_HOSTNAME);

#if 0
	//set default mDNS instance name
	ESP_ERROR_CHECK( mdns_instance_name_set("ESP32 with mDNS") );
#endif
}

void time_sync_notification_cb(struct timeval *tv)
{
	ESP_LOGI(TAG, "Notification of a time synchronization event");
}

static void initialize_sntp(void)
{
	ESP_LOGI(TAG, "Initializing SNTP");
	sntp_setoperatingmode(SNTP_OPMODE_POLL);
	//sntp_setservername(0, "pool.ntp.org");
	ESP_LOGI(TAG, "Your NTP Server is %s", CONFIG_NTP_SERVER);
	sntp_setservername(0, CONFIG_NTP_SERVER);
	sntp_set_time_sync_notification_cb(time_sync_notification_cb);
	sntp_init();
}


static void obtain_time2(void)
{
    // ESP_ERROR_CHECK( nvs_flash_init() );
    // ESP_ERROR_CHECK(esp_netif_init());
    // ESP_ERROR_CHECK( esp_event_loop_create_default() );

    /**
     * NTP server address could be aquired via DHCP,
     * see following menuconfig options:
     * 'LWIP_DHCP_GET_NTP_SRV' - enable STNP over DHCP
     * 'LWIP_SNTP_DEBUG' - enable debugging messages
     *
     * NOTE: This call should be made BEFORE esp aquires IP address from DHCP,
     * otherwise NTP option would be rejected by default.
     */
#ifdef LWIP_DHCP_GET_NTP_SRV
    sntp_servermode_dhcp(1);      // accept NTP offers from DHCP server, if any
#endif

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    // ESP_ERROR_CHECK(example_connect());

    initialize_sntp();

    // wait for time to be set
    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;
    const int retry_count = 15;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    time(&now);
    localtime_r(&now, &timeinfo);

    // ESP_ERROR_CHECK( example_disconnect() );
}

static esp_err_t obtain_time(void)
{
	initialize_sntp();
	// wait for time to be set
	int retry = 0;
	const int retry_count = 10;
	while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
		ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
		vTaskDelay(2000 / portTICK_PERIOD_MS);
	}

	if (retry == retry_count) return ESP_FAIL;
	return ESP_OK;
}

void ftp_task (void);
void ftp__init(void);

void SDCardManager__Init(void)
{

	// Initialize NVS
	esp_err_t ret;
	ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
	if (wifi_init_sta() != ESP_OK) {
		ESP_LOGE(TAG, "Connection failed");
		// while(1) { vTaskDelay(1); }
	}

	// Initialize mDNS
	initialise_mdns();
	tcpip_adapter_ip_info_t ip_info;
	ESP_ERROR_CHECK(tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ip_info));

	/* Print the local IP address */
	ESP_LOGI(TAG, "IP Address : %s", ip4addr_ntoa(&ip_info.ip));
	ESP_LOGI(TAG, "Subnet mask: %s", ip4addr_ntoa(&ip_info.netmask));
	ESP_LOGI(TAG, "Gateway    : %s", ip4addr_ntoa(&ip_info.gw));

	// obtain time over NTP
	ESP_LOGI(TAG, "Getting time over NTP.");


#if CONFIG_FLASH
	// Mount FAT File System on FLASH
	char *partition_label = "storage";
	wl_handle_t s_wl_handle = mountFLASH(partition_label, MOUNT_POINT);
	if (s_wl_handle < 0) return;
#endif 

#if CONFIG_SPI_SDCARD
	if (PIN_POWER != -1) {
		//gpio_pad_select_gpio(PIN_POWER);
		gpio_reset_pin(PIN_POWER);
		/* Set the GPIO as a push/pull output */
		gpio_set_direction(PIN_POWER, GPIO_MODE_OUTPUT);
		ESP_LOGI(TAG, "Turning on the peripherals power using GPIO%d", PIN_POWER);
		gpio_set_level(PIN_POWER, 1);
		vTaskDelay(3000 / portTICK_PERIOD_MS);
	}
#endif
	
#if CONFIG_SPI_SDCARD || CONFIG_MMC_SDCARD
	// Mount FAT File System on SDCARD
	sdmmc_card_t card;
	ret = mountSDCARD(MOUNT_POINT, &card);
	if (ret != ESP_OK) return;
#endif 


ftp__init();

}

void SDCardManager__Handler(void)
{
    


    // for(;;)
    {
       ftp_task();

//        xEventGroupWaitBits( xEventTask,
// 		FTP_TASK_FINISH_BIT, /* The bits within the event group to wait for. */
// 		pdTRUE, /* BIT_0 should be cleared before returning. */
// 		pdFALSE, /* Don't wait for both bits, either bit will do. */
// 		portMAX_DELAY);/* Wait forever. */
// 	ESP_LOGE(TAG, "ftp_task finish");

// 	// Unmount FAT file system
// #if CONFIG_FLASH
// 	esp_vfs_fat_spiflash_unmount(MOUNT_POINT, s_wl_handle);
// 	ESP_LOGI(TAG, "FLASH unmounted");
// #endif 

// #if CONFIG_SPI_SDCARD || CONFIG_MMC_SDCARD
// 	esp_vfs_fat_sdcard_unmount(MOUNT_POINT, &card);
// 	ESP_LOGI(TAG, "SDCARD unmounted");
// #endif 

        // vTaskDelay(100 / portTICK_RATE_MS);

    }

    
 
}




// /* SD card and FAT filesystem example.
//    This example code is in the Public Domain (or CC0 licensed, at your option.)

//    Unless required by applicable law or agreed to in writing, this
//    software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
//    CONDITIONS OF ANY KIND, either express or implied.
// */

// // This example uses SPI peripheral to communicate with SD card.

// #include <string.h>
// #include <sys/unistd.h>
// #include <sys/stat.h>
// #include "esp_vfs_fat.h"
// #include "sdmmc_cmd.h"
// #include "SdCard.h"

// static const char *TAG = "example";

// #define MOUNT_POINT "/sdcard"

// // Pin mapping
// #if CONFIG_IDF_TARGET_ESP32

// #define PIN_NUM_MISO 2
// #define PIN_NUM_MOSI 15
// #define PIN_NUM_CLK  14
// #define PIN_NUM_CS   13

// #elif CONFIG_IDF_TARGET_ESP32S2

// // adapted for internal test board ESP-32-S3-USB-OTG-Ev-BOARD_V1.0 (with ESP32-S2-MINI-1 module)
// #define PIN_NUM_MISO 37
// #define PIN_NUM_MOSI 35
// #define PIN_NUM_CLK  36
// #define PIN_NUM_CS   34

// #elif CONFIG_IDF_TARGET_ESP32C3
// #define PIN_NUM_MISO 6
// #define PIN_NUM_MOSI 4
// #define PIN_NUM_CLK  5
// #define PIN_NUM_CS   1

// #endif //CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2

// #if CONFIG_IDF_TARGET_ESP32S2
// #define SPI_DMA_CHAN    host.slot
// #elif CONFIG_IDF_TARGET_ESP32C3
// #define SPI_DMA_CHAN    SPI_DMA_CH_AUTO
// #else
// #define SPI_DMA_CHAN    1
// #endif


// void sd_main(void)
// {
//     esp_err_t ret;

//     // Options for mounting the filesystem.
//     // If format_if_mount_failed is set to true, SD card will be partitioned and
//     // formatted in case when mounting fails.
//     esp_vfs_fat_sdmmc_mount_config_t mount_config = {
// #ifdef CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED
//         .format_if_mount_failed = true,
// #else
//         .format_if_mount_failed = false,
// #endif // EXAMPLE_FORMAT_IF_MOUNT_FAILED
//         .max_files = 5,
//         .allocation_unit_size = 16 * 1024
//     };
//     sdmmc_card_t *card;
//     const char mount_point[] = MOUNT_POINT;
//     ESP_LOGI(TAG, "Initializing SD card");

//     // Use settings defined above to initialize SD card and mount FAT filesystem.
//     // Note: esp_vfs_fat_sdmmc/sdspi_mount is all-in-one convenience functions.
//     // Please check its source code and implement error recovery when developing
//     // production applications.
//     ESP_LOGI(TAG, "Using SPI peripheral");

//     sdmmc_host_t host = SDSPI_HOST_DEFAULT();
//     spi_bus_config_t bus_cfg = {
//         .mosi_io_num = PIN_NUM_MOSI,
//         .miso_io_num = PIN_NUM_MISO,
//         .sclk_io_num = PIN_NUM_CLK,
//         .quadwp_io_num = -1,
//         .quadhd_io_num = -1,
//         .max_transfer_sz = 4000,
//     };
//     ret = spi_bus_initialize(host.slot, &bus_cfg, SPI_DMA_CHAN);
//     if (ret != ESP_OK) {
//         ESP_LOGE(TAG, "Failed to initialize bus.");
//         return;
//     }

//     // This initializes the slot without card detect (CD) and write protect (WP) signals.
//     // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
//     sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
//     slot_config.gpio_cs = PIN_NUM_CS;
//     slot_config.host_id = host.slot;

//     ESP_LOGI(TAG, "Mounting filesystem");
//     ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

//     if (ret != ESP_OK) {
//         if (ret == ESP_FAIL) {
//             ESP_LOGE(TAG, "Failed to mount filesystem. "
//                      "If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
//         } else {
//             ESP_LOGE(TAG, "Failed to initialize the card (%s). "
//                      "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
//         }
//         return;
//     }
//     ESP_LOGI(TAG, "Filesystem mounted");

//     // Card has been initialized, print its properties
//     sdmmc_card_print_info(stdout, card);

//     // Use POSIX and C standard library functions to work with files.

//       int mk_ret = mkdir("/sdcard/Configuration", 0775);
//       ESP_LOGI(TAG, "mkdir ret %d", mk_ret);
//       mk_ret = mkdir("/sdcard/Telementry", 0775);
//       ESP_LOGI(TAG, "mkdir ret %d", mk_ret);
//       mk_ret = mkdir("/sdcard/Diagnostics", 0775);
//       ESP_LOGI(TAG, "mkdir ret %d", mk_ret);
//       mk_ret = mkdir("/sdcard/Events", 0775);
//       ESP_LOGI(TAG, "mkdir ret %d", mk_ret);
//       mk_ret = mkdir("/sdcard/OTA", 0775);
//       ESP_LOGI(TAG, "mkdir ret %d", mk_ret);
//       mk_ret = mkdir("/sdcard/Warnings", 0775);
//       ESP_LOGI(TAG, "mkdir ret %d", mk_ret);
//       mk_ret = mkdir("/sdcard/Logs", 0775);
//       ESP_LOGI(TAG, "mkdir ret %d", mk_ret);
//       mk_ret = mkdir("/sdcard/Images", 0775);
//       ESP_LOGI(TAG, "mkdir ret %d", mk_ret);
//       mk_ret = mkdir("/sdcard/Videos", 0775);
//       ESP_LOGI(TAG, "mkdir ret %d", mk_ret);

//     // First create a file.
//     const char *file_abhed = MOUNT_POINT"/Configuration/Abhed.txt";

//     ESP_LOGI(TAG, "Opening file %s", file_abhed);
//     FILE *f = fopen(file_abhed, "w");
//     if (f == NULL) {
//         ESP_LOGE(TAG, "Failed to open file for writing");
//         return;
//     }
//     fprintf(f, "firmwareVersion=v1r1,\r\nmqtt.host= http=//abhed.nexlein.com,\r\nmqtt.port= 1883,\r\nmqtt.vhost= boot,\r\nmqtt.username= apachet20dun,\r\nmqtt.password= c3Flx12Q,\r\nmqtt.routingKey= apachet20dt,\r\nabhed.bleVisibleName=ABHED-001,\r\nabhed.telemetryFrequency=60,\r\nabhed.diagnosticFrequency=120,\r\nabhed.trustedSources=VIOLAB,\r\nmotorRotationClockwiseSteps= 4000,\r\nmotorRotationAntiClockwiseSteps= 4000");
    
//     // fprintf(f, "Hello %s!\n", card->cid.name);
//     fclose(f);
//     ESP_LOGI(TAG, "File written");

//     // const char *file_foo = MOUNT_POINT"/foo.txt";

//     // // Check if destination file exists before renaming
//     // struct stat st;
//     // if (stat(file_foo, &st) == 0) {
//     //     // Delete it if it exists
//     //     unlink(file_foo);
//     // }

//     // // Rename original file
//     // ESP_LOGI(TAG, "Renaming file %s to %s", file_hello, file_foo);
//     // if (rename(file_hello, file_foo) != 0) {
//     //     ESP_LOGE(TAG, "Rename failed");
//     //     return;
//     // }

//     // Open renamed file for reading
//     ESP_LOGI(TAG, "Reading file %s", file_abhed);
//     f = fopen(file_abhed, "r");
//     if (f == NULL) {
//         ESP_LOGE(TAG, "Failed to open file for reading");
//         return;
//     }

//     // Read a line from file
//     char line[64];
//     fgets(line, sizeof(line), f);
//     fclose(f);

//     // Strip newline
//     char *pos = strchr(line, '\n');
//     if (pos) {
//         *pos = '\0';
//     }
//     ESP_LOGI(TAG, "Read from file: '%s'", line);

//     // // All done, unmount partition and disable SPI peripheral
//     // esp_vfs_fat_sdcard_unmount(mount_point, card);
//     // ESP_LOGI(TAG, "Card unmounted");

//     // //deinitialize the bus after all devices are removed
//     // spi_bus_free(host.slot);
// }

void SdCard__RenameFile(const char *oldname, const char* newname)
{

    struct stat st;
    if (stat(newname, &st) == 0) {
        // Delete it if it exists
        unlink(newname);
    }

    // Rename original file
    ESP_LOGI(TAG, "Renaming file %s to %s", oldname, newname);
    if (rename(oldname, newname) != 0) {
        ESP_LOGE(TAG, "Rename failed");
        return;
    }

}

void SdCard__DeleteFile(const char *filename)
{

    struct stat st;
    if (stat(filename, &st) == 0) {
        // Delete it if it exists
        unlink(filename);
    }

}



void SDCard__WriteToFile(char *file_name ,char *dt)
{

    // First create a file.
    // const char *file_abhed = MOUNT_POINT"/Configuration/Abhed.txt";

    ESP_LOGI(TAG, "Opening file %s", file_name);
    FILE *f = fopen(file_name, "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }
    fprintf(f, dt);
    
    // fprintf(f, "Hello %s!\n", card->cid.name);
    fclose(f);
    ESP_LOGI(TAG, "File written");

}


void SDCard__WriteToOTPFile(char *file_name ,char *dt)
{

    // First create a file.
    // const char *file_abhed = MOUNT_POINT"/Configuration/Abhed.txt";

    ESP_LOGI(TAG, "Opening file %s", file_name);
    FILE *f = fopen(file_name, "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }
    fprintf(f, dt);
    
    // fprintf(f, "Hello %s!\n", card->cid.name);
    fclose(f);
    ESP_LOGI(TAG, "File written");

}


void SdCard__CheckRenameFile()
{
	// Check if destination file exists before renaming
    struct stat st;
    if (stat("/root/Configuration/Abhed.txt", &st) == 0) {
        // Delete it if it exists
        // unlink(file_foo);
		printf("Abhed file exists");
    }
	else{
		printf("Abhed file not exists");
	}

    if (stat("/root/Configuration/Abhed_Default.txt", &st) == 0) {
        // Delete it if it exists
        // unlink(file_foo);
		printf("Abhed_Default file exists");
    }
	else{
		printf("Abhed_Default file not exists");
	}

    // Rename original file
    ESP_LOGI(TAG, "Renaming file %s to %s", "/root/Configuration/Abhed.txt", "/root/Configuration/Abhed_Default.txt");
    if (rename("/root/Configuration/Abhed.txt", "/root/Configuration/Abhed_Default.txt") != 0) {
        ESP_LOGE(TAG, "Rename failed");
        return;
    }
}

void SDCard__WriteToConfigFile(char *file_name ,char *dt)
{

	struct stat st;

	if (stat("/root/Configuration/Abhed_Default.txt", &st) == 0) {
			// Delete it if it exists
			// unlink(file_foo);
			printf("Abhed_Default file exists");
		}
	else{
		printf("Abhed_Default file not exists");

		ESP_LOGI(TAG, "Renaming file %s to %s", "/root/Configuration/Abhed.txt", "/root/Configuration/Abhed_Default.txt");
		if (rename("/root/Configuration/Abhed.txt", "/root/Configuration/Abhed_Default.txt") != 0) {
			ESP_LOGE(TAG, "Rename failed");
			// return;
		}

	}


    ESP_LOGI(TAG, "Opening file %s", file_name);
    FILE *f = fopen(file_name, "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }
    fprintf(f, dt);
    
    // fprintf(f, "Hello %s!\n", card->cid.name);
    fclose(f);
    ESP_LOGI(TAG, "File written");
	config_file_updated = true;

}


void SDCard__WriteToSDFile_Telemetry(char* folder_name,char *file_name ,char *dt)
{

	

	// int mk_ret = mkdir("/root/Configuration123", 0775);
	// ESP_LOGI(TAG, "mkdir ret %d", mk_ret);

    // First create a folder.
	char fd_name[50]="/root/Telemetry/";
    strcat(fd_name,folder_name);

	ESP_LOGI(TAG, "Creating dir : %s",fd_name);    
	int mk_ret = mkdir(fd_name, 0775);
	ESP_LOGI(TAG, "mkdir ret %d", mk_ret);
	
    char fl_name[50];
	memset(fl_name, '\0', sizeof(fl_name));
	strcat(fd_name,"/");
    strcat(fl_name,fd_name);
	strcat(fl_name,file_name);
	strcat(fl_name,".txt");

	if(mk_ret < 0)
	{
		// folder already exist

	}

    ESP_LOGI(TAG, "Opening file %s", fl_name);
    FILE *f = fopen(fl_name, "a");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }
    fprintf(f, dt);
	fprintf(f, " | ");
	fprintf(f, "\r\n");
    
    // fprintf(f, "Hello %s!\n", card->cid.name);
    fclose(f);
    ESP_LOGI(TAG, "File written");

}

void SDCard__WriteToSDFile_Events(char* folder_name,char *file_name ,char *dt)
{


    // First create a folder.
	char fd_name[50]="/root/Events/";
    strcat(fd_name,folder_name);

	ESP_LOGI(TAG, "Creating dir : %s",fd_name);    
	int mk_ret = mkdir(fd_name, 0775);
	ESP_LOGI(TAG, "mkdir ret %d", mk_ret);
	
    char fl_name[50];
	memset(fl_name, '\0', sizeof(fl_name));
	strcat(fd_name,"/");
    strcat(fl_name,fd_name);
	strcat(fl_name,file_name);
	strcat(fl_name,".txt");

	if(mk_ret < 0)
	{
		// folder already exist

	}

    ESP_LOGI(TAG, "Opening file %s", fl_name);
    FILE *f = fopen(fl_name, "a");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }
    fprintf(f, dt);
	fprintf(f, " | ");
    
    // fprintf(f, "Hello %s!\n", card->cid.name);
    fclose(f);
    ESP_LOGI(TAG, "File written");

}

void SDCard__WriteToSDFile_Diagnostics(char* folder_name,char *file_name ,char *dt)
{

	// int mk_ret = mkdir("/root/Configuration123", 0775);
	// ESP_LOGI(TAG, "mkdir ret %d", mk_ret);

    // First create a folder.
	char fd_name[50]="/root/Diagnostics/";
    strcat(fd_name,folder_name);

	ESP_LOGI(TAG, "Creating dir : %s",fd_name);    
	int mk_ret = mkdir(fd_name, 0775);
	ESP_LOGI(TAG, "mkdir ret %d", mk_ret);
	
    char fl_name[50];
	memset(fl_name, '\0', sizeof(fl_name));
	strcat(fd_name,"/");
    strcat(fl_name,fd_name);
	strcat(fl_name,file_name);
	strcat(fl_name,".txt");

	if(mk_ret < 0)
	{
		// folder already exist

	}

    ESP_LOGI(TAG, "Opening file %s", fl_name);
    FILE *f = fopen(fl_name, "a");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }
    fprintf(f, dt);
	fprintf(f, " | ");
	fprintf(f, "\r\n");
    
    // fprintf(f, "Hello %s!\n", card->cid.name);
    fclose(f);
    ESP_LOGI(TAG, "File written");

}


void SDCard__ReadFile(const char *file_name)
{

	// xSemaphoreTake(Semaphore_sd, portMAX_DELAY);
        // Open renamed file for reading
    ESP_LOGI(TAG, "Reading file %s", file_name);
    FILE *f = fopen(file_name, "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }

    // Read a line from file
    // char line[800];
	memset(File_content, '\0', sizeof(File_content));

    fread(File_content, sizeof(File_content),1, f);
    fclose(f);

	printf("file content = %s", File_content);
	// printf("size of file content = %d", sizeof(line));

}

char *SdCard__GetFileContent()
{
	return File_content;
}

bool SdCard__GetConfigUpdateStatus()
{
	return config_file_updated;
}

void SdCard__SetConfigUpdateStatus(bool status)
{
	config_file_updated = status;
}

char *SdCard__ReadLine(char *file_name)
{
	FILE *f = fopen(file_name, "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }

	char line[64];
    fgets(line, sizeof(line), f);
    fclose(f);
}







