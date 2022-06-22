#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "system.h"
#include "system.h"
#include "system_prv.h"
#include "esp_log.h"
#include "cJSON.h"
#include "LedWs2812.h"

#include "LedManager.h"
#include "StepperMotorManager.h"
#include "BleManager.h"
#include "GsmGprsManager.h"
#include "Mac.h"
#include "libGSM.h"

#include "sim7600.h"
#include "As2315.h"
#include "SdCard.h"
#include "EspBle.h"
#include "CurrentSense.h"

#include "../../SoftwareConfig.h"


bool test_bit = false;
//=========================================== CONSTANTS ==============================================================
const char *USER_P1 = "<U>";
const char *USER_P2 = "</U>";
const char *SERVICE_P1 = "<S>";
const char *SERVICE_P2 = "</S>";
const char *CMD_P1 = "<C>";
const char *CMD_P2 = "</C>";
const char *QUEUE_P1 = "<Q>";
const char *QUEUE_P2 = "</Q>";
// ========================================== SD FILE CONSTANTS =======================================================
const char *FIRMWARE_VERSION_P1 = "<FirmwareVersion>";
const char *FIRMWARE_VERSION_P2 = "</FirmwareVersion>";
const char *MQTT_HOST_P1 = "<mqtt.host>";
const char *MQTT_HOST_P2 = "</mqtt.host>";
const char *MQTT_PORT_P1 = "<mqtt.port>";
const char *MQTT_PORT_P2 = "</mqtt.port>";
const char *MQTT_VHOST_P1 = "<mqtt.vhost>";
const char *MQTT_VHOST_P2 = "</mqtt.vhost>";
const char *MQTT_USERNAME_P1 = "<mqtt.username>";
const char *MQTT_USERNAME_P2 = "</mqtt.username>";
const char *MQTT_PASSWORD_P1 = "<mqtt.password>";
const char *MQTT_PASSWORD_P2 = "</mqtt.password>";
const char *MQTT_ROUTING_KEY_P1 = "<mqtt.routingKey>";
const char *MQTT_ROUTING_KEY_P2 = "</mqtt.routingKey>";
const char *BLENAME_P1 = "<abhed.bleVisibleName>";
const char *BLENAME_P2 = "</abhed.bleVisibleName>";
const char *MQTT_TELEMETRY_FREQ_P1 = "<abhed.telemetryFrequency>";
const char *MQTT_TELEMETRY_FREQ_P2 = "</abhed.telemetryFrequency>";
const char *MQTT_DIAGNO_FREQ_P1 = "<abhed.diagnosticFrequency>";
const char *MQTT_DIAGNO_FREQ_P2 = "</abhed.diagnosticFrequency>";
const char *TRUSTED_SOURCE_P1 = "<abhed.trustedSources>";
const char *TRUSTED_SOURCE_P2 = "</abhed.trustedSources>";
const char *MOTOR_CW_STEPS_P1 = "<motorRotationClockwiseSteps>";
const char *MOTOR_CW_STEPS_P2 = "</motorRotationClockwiseSteps>";
const char *MOTOR_CCW_STEPS_P1 = "<motorRotationAntiClockwiseSteps>";
const char *MOTOR_CCW_STEPS_P2 = "</motorRotationAntiClockwiseSteps>";
const char *OTP_P1 = "<OTP>";
const char *OTP_P2 = "</OTP>";


bool sms_ble_user_extract_success = false;
bool sms_ble_srn_extract_success = false;
bool sms_ble_cmd_extract_success = false;
bool sms_ble_queue_extract_success = false;
bool sms_ble_otp_extract_success = false;



//============================================================= DEFINITIONS ===================================================

#define AUTH_PHONE_NUM	"+919028795409"
#define CMD_MTR_CW	"MTRC"
#define CMD_MTR_CCW	"MTRA"
#define CMD_GTEL	"GTEL"
#define CMD_GDIG	"GDIG"
#define CMD_SCFG	"SCFG"
#define CMD_GCFG	"GCFG"
#define CMD_SOTP	"SOTP"
#define CMD_GOTP	"GOTP"
#define CMD_GSRC	"GSRC"

#define FLOW_GTEL	CMD_GTEL
#define FLOW_GDIG	CMD_GDIG
#define FLOW_MTRC	CMD_MTR_CW
#define FLOW_MTRA	CMD_MTR_CCW
#define FLOW_SCFG	CMD_SCFG
#define FLOW_GCFG	CMD_GCFG
#define FLOW_SOTP	CMD_SOTP
#define FLOW_GSRC	CMD_GSRC


#define CHANNEL_BLE		"BLE"
#define CHANNEL_AUTO	"AUTO"
#define CHANNEL_SMS		"SMS"
#define CHANNEL_SYS		"SYS"
#define CHANNEL_BOOT	"BOOT"

//=========================================================  SD DIRECTORY STRUCTURE ==========================================


//=========================================== PRIVATE VARIABLES =======================================================
static bool system_boot = false;
static const char *TAG = "system"; 
static char * user_id;
static char * service_id;
static char * command_req;
static char * new_queue;
char otp[10];
static char *BleMsg;
static char * received_sms;
static char * sms_sender_num;
static bool motor_position_cw = false;
static bool motor_position_ccw = false;
char *_json_msg;
char *_json_msg2;
static char _mac[20];
static bool _boot_up_done = false;

char _Abhed_TextFile_Content[1024];
char _OTP_TextFileContent[1024];
char *_Firmware_Version;
char *_Mqtt_Host;
char *_Mqtt_Port;
char *_Mqtt_Vhost;
char *_Mqtt_Username;
char *_Mqtt_Password;
char *_Mqtt_RoutingKey;
char *_BleName;
char *_Telemetry_Freq;
char *_Diagnostic_Freq;
char *_Trusted_Source;
char *_Motor_CW_Steps;
char *_Motor_CCW_Steps;

char *_Imei_1;
char *_Imei_2;
char *_Imsi_1;
char *_Imsi_2;
char *_Rssi_1;
char *_Rssi_2;
char *_Operator_1;
char *_Operator_2;
char _timestamp[30];
char _ts[30];
char File_N[15];
char Folder_N[15];
char Motor_CW_Steps[10];
char Motor_CCW_Steps[10];
char OTP_STR[15];

char message_to_send_1[1024];
char message_to_send_2[1024];




//=====================================================================================================================

//=========================================== SYSTEM INPUTS INCLUSION (Header Files) ==================================

//=====================================================================================================================


//=========================================== PRIVATE (Function Prototypes) ===========================================

static void System__InitInputs(void);
static void System__InitInputOutputs(void);
static void System__InitOutputs(void);
static void system__extractsms(char *msg);
static void system__extractConfigTextFile(char *msg, char* delim1, char* delim2, char **buf_out);

char *system__CreateJson(JSON_MSG__TYPE msg_type, char *event, char *msg_channel);


static void Application__Init(void);
static void Application__UpdateConfigurations(void);
static void Application__GetImei1(char ** buff);
static void Application__GetImsi1(char ** buff);
static void Application__GetRssi1(char ** buff);
static void Application__GetOperator1(char ** buff);
static void Application__SetMqtt_url(const char *url);
static void Application__SetMqtt_port(char *port);
static void Application__SetMqttRouting_Key(char *routing_key);
static void Application__SetMqtt_Username(char *username);
static void Application__SetMqtt_Password(char *password);
static void Application__SetMqtt_Vhost(char *vhost);
static void Application__SetBleDevice_Name(char *blename);
static void Application__SetSDFolder_File_Names(char * ts, char * channel, char *flowid);
bool Appllication__CheckOTP(char * otp_file_content, char *OTP_STR);
char *Application__strremove(char *str, const char *sub);

static void Application__StepperClockwise(uint32_t steps);
static void Application__StepperAntiClockwise(uint32_t steps);
//=====================================================================================================================


static void System__InitInputs(void)
{
    // Initialize input Taks

	
    
}
static void System__InitInputOutputs(void)
{
    // Initialize input output Taks

	// BaseType_t xTaskCreatePinnedToCore(TaskFunction_t pvTaskCode, const char *constpcName, const uint32_t usStackDepth, void *constpvParameters, UBaseType_t uxPriority, TaskHandle_t *constpvCreatedTask, const BaseType_t xCoreID)
	
	// xTaskCreate(&GsmGprsManager__Handler, "GsmGprs_Handler", 1024*7 ,NULL,5,NULL );
	// vTaskDelay(1000 / portTICK_RATE_MS);
	// xTaskCreatePinnedToCore(&SDCardManager__Handler, "SD_Handler", 1024* 6, NULL,2,NULL,1 );
	// vTaskDelay(1000 / portTICK_RATE_MS);
	
	
   
}
static void System__InitOutputs(void)
{
    // Initialize output Taks
	// xTaskCreate(&LedManager__Handler, "Led_Handler", 4096 ,NULL,5,NULL );
	// vTaskDelay(1000 / portTICK_RATE_MS);
	// xTaskCreate(&StepperMotorManager__Handler, "StepperMotor_Handler", 4096 ,NULL,5,NULL );
	// vTaskDelay(1000 / portTICK_RATE_MS);
    
}


#define BUF_SIZE (1024)

void System__Init(void)
{
	
}

void system__Handler(void)
{

	//Wait till all module initialization is complete
	if((LedManager__initStatus() == true) && (GsmGprsManager__initStatus() == true))
	{

		static int x;
		// ESP_LOGI(TAG, "Running system");

		// SdCard__CheckRenameFile();

		int level = gpio_get_level(16);
		// ESP_LOGI(TAG, "GPIO LEVEL = %d", level);

		if((level == 0) && (test_bit == false))
		{
			system_boot = true;
			Application__UpdateConfigurations();
			Application__BleEnable();
			test_bit = true;
		}

		if(system_boot == false)
		{

				
				Application__UpdateConfigurations();

				Application__StepperClockwise(atoi(_Motor_CW_Steps));
				Application__StepperAntiClockwise(atoi(_Motor_CCW_Steps));

				Application__BleEnable();

				system_boot = true;
				GsmGprsManager__GetOperator_1(&_Operator_1);
				// GsmGprsManager__GetNetworkTime(&_timestamp);
				gsm_readTime();
				vTaskDelay(2000 / portTICK_RATE_MS); //RTOS TASK SYNC DELAY, Dont remove this

				strcpy(_timestamp, gsm__getTimeBufferVal());

				// remove unwanted characters for Date/Time field
				const char *i1_1 = strstr(_timestamp, "\"");
				if(i1_1 != NULL){const size_t pl1 = strlen("\"");const char *i2 = strstr(i1_1 + pl1, "\"");if("\"" != NULL){/* Found both markers, extract text. */const size_t mlen = i2 - (i1_1 + pl1);char *ret = malloc(mlen + 1);if(ret != NULL){memcpy(ret, i1_1 + pl1, mlen);ret[mlen] = '\0';strcpy(_timestamp,ret);}}}

				// memset(_ts, '\0', sizeof(_ts));
				strcpy(_ts, _timestamp);
				ESP_LOGI(TAG, "time is : %s", _ts);
				
				if(!strcmp(_Operator_1, ""))
				{
					// No network
					
					strcpy(message_to_send_1, system__CreateJson(JSON_BOOTUP_TELEMETRY_MESSAGE,"GTEL","BOOT"));
					
					ESP_LOGI(TAG, "No network but time is : %s", _timestamp);
					Application__SetSDFolder_File_Names(_timestamp, CHANNEL_BOOT, FLOW_GTEL);
					ESP_LOGI(TAG, "FOLDERNAME --- : %s",Folder_N);
					ESP_LOGI(TAG, "FL_NAME--- : %s",File_N);
					strcpy(File_N,Folder_N);
					SDCard__WriteToSDFile_Telemetry(Folder_N,File_N,message_to_send_1);

					strcpy(message_to_send_2, system__CreateJson(JSON_BOOTUP_DISGNOSTIC_MESSAGE,"GDIG","BOOT"));
					ESP_LOGI(TAG, "No network but time is : %s", _ts);
					Application__SetSDFolder_File_Names(_ts, CHANNEL_BOOT, FLOW_GDIG);
					ESP_LOGI(TAG, "FOLDERNAME --- : %s",Folder_N);
					ESP_LOGI(TAG, "FL_NAME--- : %s",File_N);
					strcpy(File_N,Folder_N);
					SDCard__WriteToSDFile_Diagnostics(Folder_N,File_N,message_to_send_2);

					
				}
				else
				{
					Application__BleDisable();
					
					printf("sending boot msg ---");

					strcpy(message_to_send_1, system__CreateJson(JSON_BOOTUP_TELEMETRY_MESSAGE,"GTEL","BOOT"));
					// printf("Gtel json created --- %s", message_to_send_1);

					strcpy(message_to_send_2, system__CreateJson(JSON_BOOTUP_DISGNOSTIC_MESSAGE,"GDIG","BOOT"));
					// printf("Gdig json created ---%s", message_to_send_2);

					GsmGprsManager__ProcessExternalEvent(EV_GSMGPRS_MQTTSEND_MESSAGE ,message_to_send_1, message_to_send_2,_Mqtt_RoutingKey, "NULL", _Mqtt_Host, _Mqtt_Vhost, _Mqtt_Port, _Mqtt_Username, _Mqtt_Password);

					

				}
			
			
		}
		
		bool lock_auto_publish = false;
		uint8_t lock_ble_state = 0;

		

			

		// StepperMotorManager__ExternalProcess(5, OPERATION__STEPPER_CW_STEPBASED, 10000,15);

		// for(;;)
		{

			// Adc_manager();
		
		
			if(((SdCard__GetConfigUpdateStatus() ==true) && (GsmGprsManager__GetState() == STATE_GSMGPRS_RUNNING)) || (ftp__GetConfigurationFileUopdatedStatus() == true))
			{
				vTaskDelay(1000 / portTICK_RATE_MS);
				lock_auto_publish = true;
				SdCard__SetConfigUpdateStatus(false);
				ftp__SetConfigurationFileUopdatedStatus(false);
				Application__UpdateConfigurations();

				Timers__SecondsExpire(SECONDS__TIMER_PUBLISH_TELEMETRY_MSG_INTERVAL);
				Timers__SecondsSet(SECONDS__TIMER_PUBLISH_TELEMETRY_MSG_INTERVAL,atoi(_Telemetry_Freq));

				lock_auto_publish = false;
				
			}

			// ===================================== Auto Publish on time interval set in SD file ==================

			if((Timers__SecondsGetStatus(SECONDS__TIMER_PUBLISH_TELEMETRY_MSG_INTERVAL) == TIMERS_COMPLETED) && (GsmGprsManager__GetState() != STATE_GSMGPRS_NEWSMS_RECEIVED) && (lock_auto_publish == false))
			{
				//check network here, and if not availble then save the data in to sd card
				ESP_LOGI(TAG,".....Telemetry & Diagnostics auto messages....");
				
				
				GsmGprsManager__GetOperator_1(&_Operator_1);
				// GsmGprsManager__GetNetworkTime(&_timestamp);
				gsm_readTime();
				vTaskDelay(2000 / portTICK_RATE_MS); //RTOS TASK SYNC DELAY, Dont remove this

				strcpy(_timestamp, gsm__getTimeBufferVal());

				memset(message_to_send_1, '\0', sizeof(message_to_send_1));
				memset(message_to_send_2, '\0', sizeof(message_to_send_2));

				// remove unwanted characters for Date/Time field
				const char *i1_1 = strstr(_timestamp, "\"");
				if(i1_1 != NULL){const size_t pl1 = strlen("\"");const char *i2 = strstr(i1_1 + pl1, "\"");if("\"" != NULL){/* Found both markers, extract text. */const size_t mlen = i2 - (i1_1 + pl1);char *ret = malloc(mlen + 1);if(ret != NULL){memcpy(ret, i1_1 + pl1, mlen);ret[mlen] = '\0';strcpy(_timestamp,ret);}}}
				memset(_ts, '\0', sizeof(_ts));
				strcpy(_ts, _timestamp);
				ESP_LOGI(TAG, "time is : %s", _ts);

				if(!strcmp(_Operator_1, ""))
				{
					// No network
					printf("bootmessagessent");
					char *_msg_to_send; char *_msg_to_send2;
					strcpy(message_to_send_1, system__CreateJson(JSON_BOOTUP_TELEMETRY_MESSAGE,"GTEL","AUTO"));
					ESP_LOGI(TAG, "No network but time is : %s", _timestamp);
					Application__SetSDFolder_File_Names(_timestamp, CHANNEL_AUTO, FLOW_GTEL);
					ESP_LOGI(TAG, "FOLDERNAME --- : %s",Folder_N);
					ESP_LOGI(TAG, "FL_NAME--- : %s",File_N);
					strcpy(File_N,Folder_N);
					// strcat(_msg_to_send," | ");
					SDCard__WriteToSDFile_Telemetry(Folder_N,File_N,message_to_send_1);

					strcpy(message_to_send_2, system__CreateJson(JSON_BOOTUP_DISGNOSTIC_MESSAGE,"GDIG","AUTO"));
					ESP_LOGI(TAG, "No network but time is : %s", _ts);
					Application__SetSDFolder_File_Names(_ts ,CHANNEL_AUTO, FLOW_GDIG);
					ESP_LOGI(TAG, "FOLDERNAME --- : %s",Folder_N);
					ESP_LOGI(TAG, "FL_NAME--- : %s",File_N);
					strcpy(File_N,Folder_N);
					// strcat(_msg_to_send," | ");
					SDCard__WriteToSDFile_Diagnostics(Folder_N,File_N,message_to_send_2);

					memset(_ts, '\0', sizeof(_ts));
					memset(_timestamp, '\0', sizeof(_timestamp));

				}
				else
				{
					Application__BleDisable();			
					strcpy(message_to_send_1, system__CreateJson(JSON_BOOTUP_TELEMETRY_MESSAGE,"GTEL","AUTO"));
					strcpy(message_to_send_2, system__CreateJson(JSON_BOOTUP_DISGNOSTIC_MESSAGE,"GDIG","AUTO"));
					GsmGprsManager__ProcessExternalEvent(EV_GSMGPRS_MQTTSEND_MESSAGE ,message_to_send_1, message_to_send_2,_Mqtt_RoutingKey, "NULL", _Mqtt_Host, _Mqtt_Vhost, _Mqtt_Port, _Mqtt_Username, _Mqtt_Password);
			
					//reset the timer here after sending data
					Timers__SecondsSet(SECONDS__TIMER_PUBLISH_TELEMETRY_MSG_INTERVAL,atoi(_Telemetry_Freq));

				}

				
			}

			if(Timers__SecondsGetStatus(SECONDS__TIMER_PUBLISH_TELEMETRY_MSG_INTERVAL)!= TIMERS_RUNNING)
			{
				Timers__SecondsSet(SECONDS__TIMER_PUBLISH_TELEMETRY_MSG_INTERVAL,atoi(_Telemetry_Freq));
			}

			if(Timers__MsGetStatus(MS__LOG2) == TIMERS_COMPLETED){ESP_LOGI(TAG, "Auto Publish will arrive after : %d Seconds", Timers__SecondsGet(SECONDS__TIMER_PUBLISH_TELEMETRY_MSG_INTERVAL));}
			if(Timers__MsGetStatus(MS__LOG2)!= TIMERS_RUNNING){Timers__MsSet(MS__LOG2,1000);}

			//==========================================================================================================
		


			if((GsmGprsManager__GetState() == STATE_GSMGPRS_NEWSMS_RECEIVED) && (libGSM__GetSmSDecodestatus() == true))
			{
				Timers__SecondsPause(SECONDS__TIMER_PUBLISH_TELEMETRY_MSG_INTERVAL); // pause the timed to pause the auto message transfer
				Timers__SecondsPause(SECONDS__TIMER_PUBLISH_DIAGNOSTICS_MSG_INTERVAL);// pause the timed to pause the auto message transfer
				
				// vTaskDelay(2000 / portTICK_RATE_MS);

				Application__BleDisable();
				
				GsmGprsManager__GetOperator_1(&_Operator_1);
				// GsmGprsManager__GetNetworkTime(&_timestamp);
				gsm_readTime();
				// vTaskDelay(2000 / portTICK_RATE_MS); //RTOS TASK SYNC DELAY, Dont remove this

				strcpy(_timestamp, gsm__getTimeBufferVal());


				memset(message_to_send_1, '\0', sizeof(message_to_send_1));
				memset(message_to_send_2, '\0', sizeof(message_to_send_2));

				// remove unwanted characters for Date/Time field
				const char *i1_1 = strstr(_timestamp, "\"");
				if(i1_1 != NULL){const size_t pl1 = strlen("\"");const char *i2 = strstr(i1_1 + pl1, "\"");if("\"" != NULL){/* Found both markers, extract text. */const size_t mlen = i2 - (i1_1 + pl1);char *ret = malloc(mlen + 1);if(ret != NULL){memcpy(ret, i1_1 + pl1, mlen);ret[mlen] = '\0';strcpy(_timestamp,ret);}}}
				memset(_ts, '\0', sizeof(_ts));
				strcpy(_ts, _timestamp);
				ESP_LOGI(TAG, "time is : %s", _ts);


				if(LibGSM__GetSMS(&received_sms, &sms_sender_num) == true)
				{
					
					// message will be deleted automatically once read
					ESP_LOGI(TAG, "SMS BODY = %s   SMS SENDER = %s  TRUSTED SOURCE = %s", received_sms,LibGsm__getNum(),_Trusted_Source );
					libGSM__SetSmSDecodestatus(false);


						system__extractsms(received_sms); 

						if(!strcmp(sms_sender_num, _Trusted_Source))
						{
							ESP_LOGI(TAG, "Authorized sender");
							// system__extractsms(received_sms);


							if((sms_ble_user_extract_success == true) && (sms_ble_srn_extract_success == true) && (sms_ble_cmd_extract_success == true))
							{ 

								if(!strcmp(command_req,CMD_MTR_CW))
								{
									if(motor_position_cw == true)
									{
										// Motor is already rotated clockwise, Dont move now
										ESP_LOGI(TAG, "Motor already in clockwise position");

										strcpy(message_to_send_1, system__CreateJson(JSON_SMS_MTRC__ALREADY_MOVED_MESSAGE,"MTRC","SMS"));
										// Rotate stepper motor
										GsmGprsManager__ProcessExternalEvent(EV_GSMGPRS_MQTTSEND_MESSAGE ,message_to_send_1, "NULL",_Mqtt_RoutingKey, "NULL", _Mqtt_Host, _Mqtt_Vhost, _Mqtt_Port, _Mqtt_Username, _Mqtt_Password);
										sim7600__SetNewSmsReceivedStatus(false);
										

									}
									else
									{
										// Clockwise rotation
										// StepperMotorManager__ExternalProcess(5, OPERATION__STEPPER_CW_STEPBASED, atoi(_Motor_CW_Steps),15);

										Application__StepperClockwise(atoi(_Motor_CW_Steps));

										motor_position_cw = true;
										motor_position_ccw = false;
										strcpy(message_to_send_1, system__CreateJson(JSON_SMS_MTRC_MESSAGE,"MTRC","SMS"));
										// Send MQtt Message
										GsmGprsManager__ProcessExternalEvent(EV_GSMGPRS_MQTTSEND_MESSAGE ,message_to_send_1, "NULL",_Mqtt_RoutingKey, "NULL", _Mqtt_Host, _Mqtt_Vhost, _Mqtt_Port, _Mqtt_Username, _Mqtt_Password);
										sim7600__SetNewSmsReceivedStatus(false);
									

									}
									

								}
								else if(!strcmp(command_req,CMD_MTR_CCW))
								{
									if(motor_position_ccw == true)
									{
										// Motor is already rotated clockwise, Dont move now
										ESP_LOGI(TAG, "Motor already in Anticlockwise position");

										strcpy(message_to_send_1, system__CreateJson(JSON_SMS_MTRA__ALREADY_MOVED_MESSAGE,"MTRA","SMS"));
										// Rotate stepper motor
										GsmGprsManager__ProcessExternalEvent(EV_GSMGPRS_MQTTSEND_MESSAGE ,message_to_send_1, "NULL",_Mqtt_RoutingKey, "NULL", _Mqtt_Host, _Mqtt_Vhost, _Mqtt_Port, _Mqtt_Username, _Mqtt_Password);
										sim7600__SetNewSmsReceivedStatus(false);
										

									}
									else
									{
										// Anticlockwise rotation
										// StepperMotorManager__ExternalProcess(5, OPERATION__STEPPER_CCW_STEPBASED, atoi(_Motor_CCW_Steps),15);

										Application__StepperAntiClockwise(atoi(_Motor_CCW_Steps));

										ESP_LOGI(TAG, "Motor Anticlockwise rotation");
										motor_position_ccw = true;
										motor_position_cw = false;

										strcpy(message_to_send_1, system__CreateJson(JSON_SMS_MTRA_MESSAGE,"MTRA","SMS"));
										// Send MQtt Message
										GsmGprsManager__ProcessExternalEvent(EV_GSMGPRS_MQTTSEND_MESSAGE ,message_to_send_1, "NULL",_Mqtt_RoutingKey, "NULL", _Mqtt_Host, _Mqtt_Vhost, _Mqtt_Port, _Mqtt_Username, _Mqtt_Password);
										sim7600__SetNewSmsReceivedStatus(false);
												

									}

								}
								else if(!strcmp(command_req,CMD_GTEL))
								{
									
									ESP_LOGI(TAG,"Command GTEL detcted");

									strcpy(message_to_send_1, system__CreateJson(JSON_SMS_GTEL_MESSAGE,FLOW_GTEL,CHANNEL_SMS));
									// Send MQtt Message
									GsmGprsManager__ProcessExternalEvent(EV_GSMGPRS_MQTTSEND_MESSAGE ,message_to_send_1, "NULL",_Mqtt_RoutingKey, "NULL", _Mqtt_Host, _Mqtt_Vhost, _Mqtt_Port, _Mqtt_Username, _Mqtt_Password);
									sim7600__SetNewSmsReceivedStatus(false);

								}
								else if(!strcmp(command_req,CMD_GDIG))
								{

									
									strcpy(message_to_send_1, system__CreateJson(JSON_SMS_GDIG_MESSAGE,CMD_GDIG,CHANNEL_SMS));
									// Send MQtt Message
									GsmGprsManager__ProcessExternalEvent(EV_GSMGPRS_MQTTSEND_MESSAGE ,message_to_send_1, "NULL",_Mqtt_RoutingKey, "NULL", _Mqtt_Host, _Mqtt_Vhost, _Mqtt_Port, _Mqtt_Username, _Mqtt_Password);
									sim7600__SetNewSmsReceivedStatus(false);
									

								}
								else if(!strcmp(command_req,CMD_SCFG))
								{
									strcpy(message_to_send_1, system__CreateJson(JSON_SMS_SCFG_MESSAGE,CMD_SCFG,CHANNEL_SMS));
									
									GsmGprs__SetCurrentCMD(CMD_SCFG);
									GsmGprsManager__ProcessExternalEvent(EV_GSMGPRS_CONNECT_NEW_TOPIC ,message_to_send_1, "NULL",_Mqtt_RoutingKey, new_queue, _Mqtt_Host, _Mqtt_Vhost, _Mqtt_Port, _Mqtt_Username, _Mqtt_Password);
									sim7600__SetNewSmsReceivedStatus(false);
									
								}
								else if(!strcmp(command_req,CMD_GCFG))
								{
									strcpy(message_to_send_1, system__CreateJson(JSON_SMS_GCFG_MESSAGE,CMD_GCFG,CHANNEL_SMS));
								
									GsmGprs__SetCurrentCMD(CMD_GCFG);
									GsmGprsManager__ProcessExternalEvent(EV_GSMGPRS_MQTTSEND_MESSAGE ,message_to_send_1, "NULL",_Mqtt_RoutingKey, "NULL", _Mqtt_Host, _Mqtt_Vhost, _Mqtt_Port, _Mqtt_Username, _Mqtt_Password);
									sim7600__SetNewSmsReceivedStatus(false);
									
								}
								else if(!strcmp(command_req,CMD_SOTP))
								{

									strcpy(message_to_send_1, system__CreateJson(JSON_SMS_SOTP_MESSAGE,CMD_SOTP,CHANNEL_SMS));

									GsmGprs__SetCurrentCMD(CMD_SOTP);
									GsmGprsManager__ProcessExternalEvent(EV_GSMGPRS_CONNECT_NEW_TOPIC ,message_to_send_1, "NULL",_Mqtt_RoutingKey, new_queue, _Mqtt_Host, _Mqtt_Vhost, _Mqtt_Port, _Mqtt_Username, _Mqtt_Password);
									sim7600__SetNewSmsReceivedStatus(false);
								}
								else if(!strcmp(command_req,CMD_GOTP))
								{
									strcpy(message_to_send_1, system__CreateJson(JSON_SMS_GOTP_MESSAGE,CMD_GOTP,CHANNEL_SMS));
									GsmGprs__SetCurrentCMD(CMD_GOTP);
									GsmGprsManager__ProcessExternalEvent(EV_GSMGPRS_MQTTSEND_MESSAGE ,message_to_send_1, "NULL",_Mqtt_RoutingKey, "NULL", _Mqtt_Host, _Mqtt_Vhost, _Mqtt_Port, _Mqtt_Username, _Mqtt_Password);
									sim7600__SetNewSmsReceivedStatus(false);
									
								}
								else
								{
									//Authorized sender but invalid command
									sim7600__SetNewSmsReceivedStatus(false);
									sim7600__EnableRingInterrupt();
									GsmGprsManager__PostEvent(EV_GSMGPRS_RUN);
								}

								

								//Enable the timer to enable to auto transmision of telemetry and disgnostic messages
								Timers__SecondsResume(SECONDS__TIMER_PUBLISH_TELEMETRY_MSG_INTERVAL);
								Timers__SecondsResume(SECONDS__TIMER_PUBLISH_DIAGNOSTICS_MSG_INTERVAL);
								Timers__SecondsSet(SECONDS__TIMER_PUBLISH_TELEMETRY_MSG_INTERVAL,atoi(_Telemetry_Freq));
								Timers__SecondsSet(SECONDS__TIMER_PUBLISH_DIAGNOSTICS_MSG_INTERVAL,atoi(_Diagnostic_Freq));

							}
							else{

								//invalid message format
								sim7600__SetNewSmsReceivedStatus(false);
								sim7600__EnableRingInterrupt();
								GsmGprsManager__PostEvent(EV_GSMGPRS_RUN);

							}



						}
						else
						{

							if((sms_ble_user_extract_success == true) && (sms_ble_srn_extract_success == true) && (sms_ble_cmd_extract_success == true))
							{ 
								// Send MQtt Message

								ESP_LOGI(TAG, "Un-Authorized sender");
								strcpy(message_to_send_1, system__CreateJson(JSON_UNAUTHORIZED_SENDER,FLOW_GTEL,CHANNEL_SMS));
								GsmGprsManager__ProcessExternalEvent(EV_GSMGPRS_MQTTSEND_MESSAGE ,message_to_send_1, "NULL",_Mqtt_RoutingKey, "NULL", _Mqtt_Host, _Mqtt_Vhost, _Mqtt_Port, _Mqtt_Username, _Mqtt_Password);
								sim7600__SetNewSmsReceivedStatus(false);
							}
							else
							{
								ESP_LOGI(TAG, "Un-Authorized sender");
								sim7600__SetNewSmsReceivedStatus(false);
								sim7600__EnableRingInterrupt();
								GsmGprsManager__PostEvent(EV_GSMGPRS_RUN);
								ESP_LOGI(TAG, "Interrupt enabled..");

							}

									
									

			
								
				

						}
			


				}
				else
				{
					Timers__SecondsResume(SECONDS__TIMER_PUBLISH_TELEMETRY_MSG_INTERVAL);
					Timers__SecondsResume(SECONDS__TIMER_PUBLISH_DIAGNOSTICS_MSG_INTERVAL);
				}


			}


			

			if(EspBle__NewMsgReceived(&BleMsg) == true) // when the new msg is read, the BleManager event will be changed to running automatically
			{
				

				printf("Ble  msg detected  = %s", BleMsg);
				system__extractsms(BleMsg);
				EspBle__Clearbuffers();

				if (Appllication__CheckOTP(_OTP_TextFileContent, otp) == true) // otp found
				{

					Application__BleDisable();


						if(!strcmp(command_req,CMD_MTR_CW))
						{


							GsmGprsManager__GetOperator_1(&_Operator_1);
							// GsmGprsManager__GetNetworkTime(&_timestamp);
							gsm_readTime();
							vTaskDelay(2000 / portTICK_RATE_MS); //RTOS TASK SYNC DELAY, Dont remove this

							strcpy(_timestamp, gsm__getTimeBufferVal());

							// remove unwanted characters for Date/Time field
							const char *i1_1 = strstr(_timestamp, "\"");
							if(i1_1 != NULL){const size_t pl1 = strlen("\"");const char *i2 = strstr(i1_1 + pl1, "\"");if("\"" != NULL){/* Found both markers, extract text. */const size_t mlen = i2 - (i1_1 + pl1);char *ret = malloc(mlen + 1);if(ret != NULL){memcpy(ret, i1_1 + pl1, mlen);ret[mlen] = '\0';strcpy(_timestamp,ret);}}}
							strcpy(_ts, _timestamp);
							
							if(!strcmp(_Operator_1, ""))
							{
								if(motor_position_cw == true)
								{
									// Motor is already rotated clockwise, Dont move now
									ESP_LOGI(TAG, "Motor already in clockwise position");

									strcpy(message_to_send_1, system__CreateJson(JSON_BLE_MTRC__ALREADY_MOVED_MESSAGE,CMD_MTR_CW,CHANNEL_BLE));
									// save event to sd card events folder
									Application__SetSDFolder_File_Names(_timestamp, CHANNEL_BLE, FLOW_MTRC);
									strcpy(File_N,Folder_N);
									SDCard__WriteToSDFile_Events(Folder_N,File_N,BleMsg);
									// Rotate stepper motor
									// save MQtt Message
									strcpy(File_N,Folder_N);
			
									SDCard__WriteToSDFile_Telemetry(Folder_N,File_N,message_to_send_1);

								}
								else
								{

									// Clockwise rotation
									ESP_LOGI(TAG, "Motor running clockwise");
									// StepperMotorManager__ExternalProcess(5, OPERATION__STEPPER_CW_STEPBASED, atoi(_Motor_CW_Steps),15);

									Application__StepperClockwise(atoi(_Motor_CW_Steps));

									motor_position_cw = true;
									motor_position_ccw = false;
									strcpy(message_to_send_1,system__CreateJson(JSON_BLE_MTRC_MESSAGE,CMD_MTR_CW,CHANNEL_BLE));
									// save event to sd card events folder
									Application__SetSDFolder_File_Names(_timestamp, CHANNEL_BLE, FLOW_MTRC);
									strcpy(File_N,Folder_N);
									SDCard__WriteToSDFile_Events(Folder_N,File_N,BleMsg);
									// save MQtt Message
									SDCard__WriteToSDFile_Telemetry(Folder_N,File_N,message_to_send_1);
									

								}

							}
							else
							{


								if(motor_position_cw == true)
								{
									// Motor is already rotated clockwise, Dont move now
									ESP_LOGI(TAG, "Motor already in clockwise position");
									char *_msg_to_send;
									strcpy(message_to_send_1,system__CreateJson(JSON_BLE_MTRC__ALREADY_MOVED_MESSAGE,CMD_MTR_CW,CHANNEL_BLE));
									// Rotate stepper motor
									GsmGprsManager__ProcessExternalEvent(EV_GSMGPRS_MQTTSEND_MESSAGE ,message_to_send_1, "NULL",_Mqtt_RoutingKey, "NULL", _Mqtt_Host, _Mqtt_Vhost, _Mqtt_Port, _Mqtt_Username, _Mqtt_Password);

								}
								else
								{
				
									// Clockwise rotation
									ESP_LOGI(TAG, "Motor running clockwise");
									// StepperMotorManager__ExternalProcess(5, OPERATION__STEPPER_CW_STEPBASED, atoi(_Motor_CW_Steps),15);

									Application__StepperClockwise(atoi(_Motor_CW_Steps));

									motor_position_cw = true;
									motor_position_ccw = false;
									strcpy(message_to_send_1,system__CreateJson(JSON_BLE_MTRC_MESSAGE,CMD_MTR_CW,CHANNEL_BLE));
									// Send MQtt Message
									GsmGprsManager__ProcessExternalEvent(EV_GSMGPRS_MQTTSEND_MESSAGE ,message_to_send_1, "NULL",_Mqtt_RoutingKey, "NULL", _Mqtt_Host, _Mqtt_Vhost, _Mqtt_Port, _Mqtt_Username, _Mqtt_Password);

								}

							}
							

						}
						else if(!strcmp(command_req,CMD_MTR_CCW))
						{


							GsmGprsManager__GetOperator_1(&_Operator_1);
							// GsmGprsManager__GetNetworkTime(&_timestamp);
							gsm_readTime();
							vTaskDelay(2000 / portTICK_RATE_MS); //RTOS TASK SYNC DELAY, Dont remove this

							strcpy(_timestamp, gsm__getTimeBufferVal());

							// remove unwanted characters for Date/Time field
							const char *i1_1 = strstr(_timestamp, "\"");
							if(i1_1 != NULL){const size_t pl1 = strlen("\"");const char *i2 = strstr(i1_1 + pl1, "\"");if("\"" != NULL){/* Found both markers, extract text. */const size_t mlen = i2 - (i1_1 + pl1);char *ret = malloc(mlen + 1);if(ret != NULL){memcpy(ret, i1_1 + pl1, mlen);ret[mlen] = '\0';strcpy(_timestamp,ret);}}}
							strcpy(_ts, _timestamp);
							
							if(!strcmp(_Operator_1, ""))
							{
								if(motor_position_ccw == true)
								{
									// Motor is already rotated clockwise, Dont move now
									ESP_LOGI(TAG, "Motor already in Anticlockwise position");
									char *_msg_to_send;
									strcpy(message_to_send_1,system__CreateJson(JSON_BLE_MTRA__ALREADY_MOVED_MESSAGE,CMD_MTR_CCW,CHANNEL_BLE));
									// save event to sd card events folder
									Application__SetSDFolder_File_Names(_timestamp, CHANNEL_BLE, FLOW_MTRA);
									strcpy(File_N,Folder_N);
									SDCard__WriteToSDFile_Events(Folder_N,File_N,BleMsg);
									// Rotate stepper motor
									SDCard__WriteToSDFile_Telemetry(Folder_N,File_N,message_to_send_1);

								}
								else
								{

									// Clockwise rotation
									ESP_LOGI(TAG, "Motor running Anticlockwise");
									// StepperMotorManager__ExternalProcess(5, OPERATION__STEPPER_CCW_STEPBASED, atoi(_Motor_CCW_Steps),15);

									Application__StepperAntiClockwise(atoi(_Motor_CCW_Steps));

									motor_position_cw = false;
									motor_position_ccw = true;
									strcpy(message_to_send_1,system__CreateJson(JSON_BLE_MTRA_MESSAGE,CMD_MTR_CCW,CHANNEL_BLE));
									// save event to sd card events folder
									Application__SetSDFolder_File_Names(_timestamp, CHANNEL_BLE, FLOW_MTRA);
									strcpy(File_N,Folder_N);
									SDCard__WriteToSDFile_Events(Folder_N,File_N,BleMsg);
									// save MQtt Message
									SDCard__WriteToSDFile_Telemetry(Folder_N,File_N,message_to_send_1);
									

								}

							}
							else
							{


								if(motor_position_ccw == true)
								{
									// Motor is already rotated clockwise, Dont move now
									ESP_LOGI(TAG, "Motor already in Anticlockwise position");
									strcpy(message_to_send_1,system__CreateJson(JSON_BLE_MTRA__ALREADY_MOVED_MESSAGE,CMD_MTR_CCW,CHANNEL_BLE));
									// save event to sd card events folder
									Application__SetSDFolder_File_Names(_timestamp, CHANNEL_BLE, FLOW_MTRA);
									strcpy(File_N,Folder_N);
									SDCard__WriteToSDFile_Events(Folder_N,File_N,BleMsg);
									// Rotate stepper motor
									GsmGprsManager__ProcessExternalEvent(EV_GSMGPRS_MQTTSEND_MESSAGE ,message_to_send_1, "NULL",_Mqtt_RoutingKey, "NULL", _Mqtt_Host, _Mqtt_Vhost, _Mqtt_Port, _Mqtt_Username, _Mqtt_Password);


								}
								else
								{
				
									// Clockwise rotation
									ESP_LOGI(TAG, "Motor running Anticlockwise");
									// StepperMotorManager__ExternalProcess(5, OPERATION__STEPPER_CCW_STEPBASED, atoi(_Motor_CCW_Steps),15);
									Application__StepperAntiClockwise(atoi(_Motor_CCW_Steps));

									motor_position_cw = false;
									motor_position_ccw = true;
									char *_msg_to_send;
									strcpy(message_to_send_1,system__CreateJson(JSON_BLE_MTRA_MESSAGE,CMD_MTR_CCW,CHANNEL_BLE));
									// Send MQtt Message
									GsmGprsManager__ProcessExternalEvent(EV_GSMGPRS_MQTTSEND_MESSAGE ,message_to_send_1, "NULL",_Mqtt_RoutingKey, "NULL", _Mqtt_Host, _Mqtt_Vhost, _Mqtt_Port, _Mqtt_Username, _Mqtt_Password);


								}

							}
						}
						
						else{
							//Authorized sender but invalid command
							ESP_LOGI(TAG, "INVALID BLE COMMAND");
						}


						Application__BleEnable();

				}

				
			}	


			// vTaskDelay(500 / portTICK_RATE_MS);
		}

	}
	else
	{
		
	}

}





//=====================================================================================================================
//-------------------------------------- Private Functions ------------------------------------------------------------
//=====================================================================================================================


static void system__init(void)
{

	// ExecuteTasks(SystemInitialization_Tasks);
    // xTaskCreate(&system__Handler, "system", STACK_SYSTEM_TASK_SIZE ,NULL,5,NULL );

	while(1) {

		printf("\nGoing to read sms");
		vTaskDelay(1000 / portTICK_RATE_MS);

	}

	vTaskDelay(1000 / portTICK_RATE_MS);
    
}
static void system__extractsms(char *msg)
{
	// memset(otp, '\0', sizeof(otp));
	const char *i1_1 = strstr(msg, USER_P1);
	const char *i2_1 = strstr(msg, SERVICE_P1);
	const char *i3_1 = strstr(msg, CMD_P1);
	const char *i4_1 = strstr(msg, QUEUE_P1);

	const char *i5_1 = strstr(msg, OTP_P1);


	sms_ble_user_extract_success = false;
	sms_ble_srn_extract_success = false;
	sms_ble_cmd_extract_success = false;
	sms_ble_queue_extract_success = false;
	sms_ble_otp_extract_success = false;
	
	if(i1_1 != NULL)
	{
		const size_t pl1 = strlen(USER_P1);
		const char *i2 = strstr(i1_1 + pl1, USER_P2);
		if(USER_P2 != NULL)
		{
			/* Found both markers, extract text. */
			const size_t mlen = i2 - (i1_1 + pl1);
			char *ret = malloc(mlen + 1);
			if(ret != NULL)
			{
				memcpy(ret, i1_1 + pl1, mlen);
				ret[mlen] = '\0';
				user_id = ret;

				ESP_LOGI(TAG, "extracted user id : %s", user_id);
				sms_ble_user_extract_success = true;

			}
		}
	}
	if(i2_1 != NULL)
	{
		const size_t pl1 = strlen(SERVICE_P1);
		const char *i2 = strstr(i2_1 + pl1, SERVICE_P2);
		if(SERVICE_P2 != NULL)
		{
			/* Found both markers, extract text. */
			const size_t mlen = i2 - (i2_1 + pl1);
			char *ret = malloc(mlen + 1);
			if(ret != NULL)
			{
				memcpy(ret, i2_1 + pl1, mlen);
				ret[mlen] = '\0';
				service_id = ret;

				ESP_LOGI(TAG, "extracted Service Id  : %s", service_id);
				sms_ble_srn_extract_success = true;

			}
		}
	}
	if(i3_1 != NULL)
	{
		const size_t pl1 = strlen(CMD_P1);
		const char *i2 = strstr(i3_1 + pl1, CMD_P2);
		if(CMD_P2 != NULL)
		{
			/* Found both markers, extract text. */
			const size_t mlen = i2 - (i3_1 + pl1);
			char *ret = malloc(mlen + 1);
			if(ret != NULL)
			{
				memcpy(ret, i3_1 + pl1, mlen);
				ret[mlen] = '\0';
				command_req = ret;

				ESP_LOGI(TAG, "extracted Command  : %s", command_req);
			
				sms_ble_cmd_extract_success = true;

			}
		}
	}

	if(i4_1 != NULL)
	{
		const size_t pl1 = strlen(QUEUE_P1);
		const char *i2 = strstr(i4_1 + pl1, QUEUE_P2);
		if(QUEUE_P2 != NULL)
		{
			/* Found both markers, extract text. */
			const size_t mlen = i2 - (i4_1 + pl1);
			char *ret = malloc(mlen + 1);
			if(ret != NULL)
			{
				memcpy(ret, i4_1 + pl1, mlen);
				ret[mlen] = '\0';
				new_queue = ret;

				ESP_LOGI(TAG, "extracted Queue  : %s", new_queue);
				sms_ble_queue_extract_success = true;

			}
		}
	}

	if(i5_1 != NULL)
	{
		const size_t pl1 = strlen(OTP_P1);
		const char *i2 = strstr(i5_1 + pl1, OTP_P2);
		if(OTP_P2 != NULL)
		{
			/* Found both markers, extract text. */
			const size_t mlen = i2 - (i5_1 + pl1);
			char *ret = malloc(mlen + 1);
			if(ret != NULL)
			{
				memcpy(ret, i5_1 + pl1, mlen);
				ret[mlen] = '\0';
				strcpy(otp,ret);

				ESP_LOGI(TAG, "extracted Otp  : %s", otp);
				sms_ble_otp_extract_success = true;

			}
		}
	}




}

 char *system__CreateJson(JSON_MSG__TYPE msg_type, char *event, char *msg_channel)
 {

    static cJSON *root;
    static cJSON *details;
	char * device_id;
	char * lattitude;
	char * longitude;
	char * Imei1;
	char * Rssi1;
	char * Imei2;
	char * Rssi2 ;
	char * MotorStatus;
	char MotorSteps[8];
	char Temperature[10] ;
	char Humidity[10] ;
	char * BatLevel;
	char * BleStatus;

	//============Diafnostics =============
	char * OverTemp_Status;
	char * UnderTemp_Status;
	char * OverHumid_Status;
	char * UnderHumid_Status ;
	char * BatteryCondition ;
	char * CurrentConsumption;
	char * MotorStall;

	OverTemp_Status = "No";
	UnderTemp_Status = "No";
	OverHumid_Status = "No";
	UnderHumid_Status = "No";
	BatteryCondition = "Normal";
	CurrentConsumption = "Normal";
	MotorStall = "No";


	
	device_id = _mac;
	lattitude = "17.5814";
	longitude = "73.8207";
	Application__GetImei1(&Imei1);
	// Imei1 = _Imei_1;
	Rssi1 = "22.9";
	Imei2 = "";
	Rssi2 = "";

	sprintf(Temperature, "%f",As2315__GetTemperatureNew());
	sprintf(Humidity, "%f",As2315__GetHumidityNew());

	if(motor_position_cw == true)
	{
		MotorStatus = "Clockwise";
		strcpy(MotorSteps, _Motor_CW_Steps);
	}
	else
	{
		MotorStatus = "AntiClockwise";
		strcpy(MotorSteps, _Motor_CCW_Steps);
	}
	

	BatLevel = "FULL";
	BleStatus = "ENABLED";

	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "Event", event);
	cJSON_AddStringToObject(root, "Type", msg_channel);
	cJSON_AddStringToObject(root, "Timestamp", _timestamp);
	cJSON_AddStringToObject(root, "DeviceId", device_id);
	cJSON_AddStringToObject(root, "Lattitude", lattitude);
	cJSON_AddStringToObject(root, "Longitude", longitude);

	switch(msg_type)
	{
		case JSON_BOOTUP_TELEMETRY_MESSAGE:
				
				details = cJSON_AddObjectToObject(root, "Details");
			 	cJSON_AddStringToObject(details, "Imei1",Imei1);
				cJSON_AddStringToObject(details, "SignalStrength1",Rssi1);
				cJSON_AddStringToObject(details, "Operator1", _Operator_1);
				cJSON_AddStringToObject(details, "Imei2",Imei2);
				cJSON_AddStringToObject(details, "SignalStrength2",Rssi2);
				cJSON_AddStringToObject(details, "Operator2", _Operator_2);
				cJSON_AddStringToObject(details, "MotorRotationStatus", MotorStatus);
				cJSON_AddStringToObject(details, "Temperature", Temperature);
				cJSON_AddStringToObject(details, "Humidity", Humidity);
				cJSON_AddStringToObject(details, "AuxillaryBatLevel", BatLevel);
				cJSON_AddStringToObject(details, "BleStatus", BleStatus);
				
			 break;
		case JSON_BOOTUP_DISGNOSTIC_MESSAGE:
				details = cJSON_AddObjectToObject(root, "Details");
			 	cJSON_AddStringToObject(details, "OverTemperature",OverTemp_Status);
				cJSON_AddStringToObject(details, "UnderTemperature",UnderTemp_Status);
				cJSON_AddStringToObject(details, "OverHumidity",OverHumid_Status);
				cJSON_AddStringToObject(details, "UnderHumidity",UnderHumid_Status);
				cJSON_AddStringToObject(details, "BatteryCondition", BatteryCondition);
				cJSON_AddStringToObject(details, "CurrentConsumption", CurrentConsumption);
				cJSON_AddStringToObject(details, "MotorStall", MotorStall);
	
			 break;
		case JSON_SMS_GTEL_MESSAGE:
				details = cJSON_AddObjectToObject(root, "Details");
				cJSON_AddStringToObject(details, "UserId",user_id);
				cJSON_AddStringToObject(details, "Srn",service_id);
				cJSON_AddStringToObject(details, "Status","OK");
			 	cJSON_AddStringToObject(details, "Imei1",Imei1);
				cJSON_AddStringToObject(details, "SignalStrength1",Rssi1);
				cJSON_AddStringToObject(details, "Imei2",Imei2);
				cJSON_AddStringToObject(details, "SignalStrength2",Rssi2);
				cJSON_AddStringToObject(details, "MotorRotationStatus", MotorStatus);
				cJSON_AddStringToObject(details, "Temperature", Temperature);
				cJSON_AddStringToObject(details, "Humidity", Humidity);
				cJSON_AddStringToObject(details, "AuxillaryBatLevel", BatLevel);
				cJSON_AddStringToObject(details, "BleStatus", BleStatus);

			 break;
		case JSON_SMS_GDIG_MESSAGE:
				details = cJSON_AddObjectToObject(root, "Details");
			 	cJSON_AddStringToObject(details, "UserId",user_id);
				cJSON_AddStringToObject(details, "Srn",service_id);
				cJSON_AddStringToObject(details, "Status","OK");
			 	cJSON_AddStringToObject(details, "OverTemperature",OverTemp_Status);
				cJSON_AddStringToObject(details, "UnderTemperature",UnderTemp_Status);
				cJSON_AddStringToObject(details, "OverHumidity",OverHumid_Status);
				cJSON_AddStringToObject(details, "UnderHumidity",UnderHumid_Status);
				cJSON_AddStringToObject(details, "BatteryCondition", BatteryCondition);
				cJSON_AddStringToObject(details, "CurrentConsumption", CurrentConsumption);
				cJSON_AddStringToObject(details, "MotorStall", MotorStall);

			 break; 

		case JSON_SMS_MTRC_MESSAGE:
				details = cJSON_AddObjectToObject(root, "Details");
				cJSON_AddStringToObject(details, "UserId",user_id);
				cJSON_AddStringToObject(details, "Srn",service_id);
				cJSON_AddStringToObject(details, "Status","OK");
				cJSON_AddStringToObject(details, "Imei1",Imei1);
				cJSON_AddStringToObject(details, "SignalStrength1",Rssi1);
				cJSON_AddStringToObject(details, "Imei2",Imei2);
				cJSON_AddStringToObject(details, "SignalStrength2",Rssi2);
				cJSON_AddStringToObject(details, "MotorRotationStatus", MotorStatus);
				cJSON_AddStringToObject(details, "MotorRotationSteps", MotorSteps);
				cJSON_AddStringToObject(details, "Temperature", Temperature);
				cJSON_AddStringToObject(details, "Humidity", Humidity);
				cJSON_AddStringToObject(details, "AuxillaryBatLevel", BatLevel);
				cJSON_AddStringToObject(details, "BleStatus", BleStatus);

			 break;
		
		case JSON_SMS_MTRC__ALREADY_MOVED_MESSAGE:
				details = cJSON_AddObjectToObject(root, "Details");
				cJSON_AddStringToObject(details, "UserId",user_id);
				cJSON_AddStringToObject(details, "Srn",service_id);
				cJSON_AddStringToObject(details, "Status","OK");
				cJSON_AddStringToObject(details, "Imei1",Imei1);
				cJSON_AddStringToObject(details, "SignalStrength1",Rssi1);
				cJSON_AddStringToObject(details, "Imei2",Imei2);
				cJSON_AddStringToObject(details, "SignalStrength2",Rssi2);
				cJSON_AddStringToObject(details, "MotorRotationStatus", "ALREADY ROTATED CLOCKWISE");
				cJSON_AddStringToObject(details, "MotorRotationSteps", MotorSteps);
				cJSON_AddStringToObject(details, "Temperature", Temperature);
				cJSON_AddStringToObject(details, "Humidity", Humidity);
				cJSON_AddStringToObject(details, "AuxillaryBatLevel", BatLevel);
				cJSON_AddStringToObject(details, "BleStatus", BleStatus);

			 break;

		case JSON_SMS_MTRA_MESSAGE:
				details = cJSON_AddObjectToObject(root, "Details");
				cJSON_AddStringToObject(details, "UserId",user_id);
				cJSON_AddStringToObject(details, "Srn",service_id);
				cJSON_AddStringToObject(details, "Status","OK");
				cJSON_AddStringToObject(details, "Imei1",Imei1);
				cJSON_AddStringToObject(details, "SignalStrength1",Rssi1);
				cJSON_AddStringToObject(details, "Imei2",Imei2);
				cJSON_AddStringToObject(details, "SignalStrength2",Rssi2);
				cJSON_AddStringToObject(details, "MotorRotationStatus", MotorStatus);
				cJSON_AddStringToObject(details, "MotorRotationSteps", MotorSteps);
				cJSON_AddStringToObject(details, "Temperature", Temperature);
				cJSON_AddStringToObject(details, "Humidity", Humidity);
				cJSON_AddStringToObject(details, "AuxillaryBatLevel", BatLevel);
				cJSON_AddStringToObject(details, "BleStatus", BleStatus);

			 break;

		case JSON_SMS_MTRA__ALREADY_MOVED_MESSAGE:
				details = cJSON_AddObjectToObject(root, "Details");
				cJSON_AddStringToObject(details, "UserId",user_id);
				cJSON_AddStringToObject(details, "Srn",service_id);
				cJSON_AddStringToObject(details, "Status","OK");
				cJSON_AddStringToObject(details, "Imei1",Imei1);
				cJSON_AddStringToObject(details, "SignalStrength1",Rssi1);
				cJSON_AddStringToObject(details, "Imei2",Imei2);
				cJSON_AddStringToObject(details, "SignalStrength2",Rssi2);
				cJSON_AddStringToObject(details, "MotorRotationStatus", "ALREADY MOVED ANTICLOCKWISE");
				cJSON_AddStringToObject(details, "MotorRotationSteps", MotorSteps);
				cJSON_AddStringToObject(details, "Temperature", Temperature);
				cJSON_AddStringToObject(details, "Humidity", Humidity);
				cJSON_AddStringToObject(details, "AuxillaryBatLevel", BatLevel);
				cJSON_AddStringToObject(details, "BleStatus", BleStatus);

		break;

		case JSON_SMS_SCFG_MESSAGE:
				details = cJSON_AddObjectToObject(root, "Details");
				cJSON_AddStringToObject(details, "UserId",user_id);
				cJSON_AddStringToObject(details, "Srn",service_id);
				cJSON_AddStringToObject(details, "Status","SUCCESS");

			 	break;

		case JSON_SMS_SOTP_MESSAGE:
				details = cJSON_AddObjectToObject(root, "Details");
				cJSON_AddStringToObject(details, "UserId",user_id);
				cJSON_AddStringToObject(details, "Srn",service_id);
				cJSON_AddStringToObject(details, "Status","SUCCESS");

			 break;

		case JSON_SMS_GCFG_MESSAGE:
				details = cJSON_AddObjectToObject(root, "Details");
				cJSON_AddStringToObject(details, "Firmware_Version",_Firmware_Version);
				cJSON_AddStringToObject(details, "Mqtt_Host",_Mqtt_Host);
				cJSON_AddStringToObject(details, "Mqtt_Port",_Mqtt_Port);
				cJSON_AddStringToObject(details, "Mqtt_Vhost",_Mqtt_Vhost);
				cJSON_AddStringToObject(details, "Mqtt_Username",_Mqtt_Username);
				cJSON_AddStringToObject(details, "Mqtt_Password",_Mqtt_Password);
				cJSON_AddStringToObject(details, "Mqtt_RoutingKey",_Mqtt_RoutingKey);
				cJSON_AddStringToObject(details, "BleName",_BleName);
				cJSON_AddStringToObject(details, "Telemetry_Freq",_Telemetry_Freq);
				cJSON_AddStringToObject(details, "Diagnostic_Freq",_Diagnostic_Freq);
				cJSON_AddStringToObject(details, "Trusted_Source",_Trusted_Source);
				cJSON_AddStringToObject(details, "Motor_CW_Steps",_Motor_CW_Steps);
				cJSON_AddStringToObject(details, "Motor_CCW_Steps",_Motor_CCW_Steps);

			 break;

		case JSON_SMS_GOTP_MESSAGE:
				// sd card otp content here in json format
				details = cJSON_AddObjectToObject(root, "Details");
				cJSON_AddStringToObject(details, "Otp", _OTP_TextFileContent);
			 break;
		case JSON_BLE_MTRC_MESSAGE:
				details = cJSON_AddObjectToObject(root, "Details");
				cJSON_AddStringToObject(details, "UserId",user_id);
				cJSON_AddStringToObject(details, "Srn",service_id);
				cJSON_AddStringToObject(details, "Status","OK");
				cJSON_AddStringToObject(details, "Imei1",Imei1);
				cJSON_AddStringToObject(details, "SignalStrength1",Rssi1);
				cJSON_AddStringToObject(details, "Imei2",Imei2);
				cJSON_AddStringToObject(details, "Signalstrength2",Rssi2);
				cJSON_AddStringToObject(details, "MotorRotationStatus", MotorStatus);
				cJSON_AddStringToObject(details, "MotorRotationSteps", MotorSteps);
				cJSON_AddStringToObject(details, "Temperature", Temperature);
				cJSON_AddStringToObject(details, "Humidity", Humidity);
				cJSON_AddStringToObject(details, "AuxillaryBatLevel", BatLevel);
				cJSON_AddStringToObject(details, "BleStatus", BleStatus);
				cJSON_AddStringToObject(details, "BleName", _BleName);
				cJSON_AddStringToObject(details, "UsedOtp", otp);
			break;

		case JSON_BLE_MTRA_MESSAGE :
				details = cJSON_AddObjectToObject(root, "Details");
				cJSON_AddStringToObject(details, "UserId",user_id);
				cJSON_AddStringToObject(details, "Srn",service_id);
				cJSON_AddStringToObject(details, "Status","OK");
				cJSON_AddStringToObject(details, "Imei1",Imei1);
				cJSON_AddStringToObject(details, "SignalStrength1",Rssi1);
				cJSON_AddStringToObject(details, "Imei2",Imei2);
				cJSON_AddStringToObject(details, "Signalstrength2",Rssi2);
				cJSON_AddStringToObject(details, "MotorRotationStatus", MotorStatus);
				cJSON_AddStringToObject(details, "MotorRotationSteps", MotorSteps);
				cJSON_AddStringToObject(details, "Temperature", Temperature);
				cJSON_AddStringToObject(details, "Humidity", Humidity);
				cJSON_AddStringToObject(details, "AuxillaryBatLevel", BatLevel);
				cJSON_AddStringToObject(details, "BleStatus", BleStatus);
				cJSON_AddStringToObject(details, "BleName", _BleName);
				cJSON_AddStringToObject(details, "UsedOtp", otp);
			break;

		case JSON_BLE_MTRC__ALREADY_MOVED_MESSAGE:
				details = cJSON_AddObjectToObject(root, "Details");
				cJSON_AddStringToObject(details, "UserId",user_id);
				cJSON_AddStringToObject(details, "SRrn",service_id);
				cJSON_AddStringToObject(details, "Status","OK");
				cJSON_AddStringToObject(details, "Imei1",Imei1);
				cJSON_AddStringToObject(details, "SignalStrength1",Rssi1);
				cJSON_AddStringToObject(details, "Imei2",Imei2);
				cJSON_AddStringToObject(details, "SignalStrength2",Rssi2);
				cJSON_AddStringToObject(details, "MotorRotationStatus", "Alreaddy Moved Clockwise");
				cJSON_AddStringToObject(details, "MotorRotationSteps", MotorSteps);
				cJSON_AddStringToObject(details, "Temperature", Temperature);
				cJSON_AddStringToObject(details, "Humidity", Humidity);
				cJSON_AddStringToObject(details, "AuxillaryBatLevel", BatLevel);
				cJSON_AddStringToObject(details, "BleStatus", BleStatus);
				cJSON_AddStringToObject(details, "BleName", _BleName);
				cJSON_AddStringToObject(details, "UsedOtp", otp);
		break;


		case JSON_BLE_MTRA__ALREADY_MOVED_MESSAGE:
				details = cJSON_AddObjectToObject(root, "Details");
				cJSON_AddStringToObject(details, "UserId",user_id);
				cJSON_AddStringToObject(details, "Srn",service_id);
				cJSON_AddStringToObject(details, "Status","OK");
				cJSON_AddStringToObject(details, "Imei1",Imei1);
				cJSON_AddStringToObject(details, "SignalStrength1",Rssi1);
				cJSON_AddStringToObject(details, "Imei2",Imei2);
				cJSON_AddStringToObject(details, "SignalStrength2",Rssi2);
				cJSON_AddStringToObject(details, "MotorRotationStatus", "Alreaddy Moved AntiClockwise");
				cJSON_AddStringToObject(details, "MotorRotationSteps", MotorSteps);
				cJSON_AddStringToObject(details, "Temperature", Temperature);
				cJSON_AddStringToObject(details, "Humidity", Humidity);
				cJSON_AddStringToObject(details, "AuxillaryBatLevel", BatLevel);
				cJSON_AddStringToObject(details, "BleStatus", BleStatus);
				cJSON_AddStringToObject(details, "BleName", _BleName);
				cJSON_AddStringToObject(details, "UsedOtp", otp);
		break;


		case JSON_SMS_GSRC_MESSAGE:
				details = cJSON_AddObjectToObject(root, "Details");
				cJSON_AddStringToObject(details, "UserId",user_id);
				cJSON_AddStringToObject(details, "Srn",service_id);
				cJSON_AddStringToObject(details, "Status","SUCCESS");
				cJSON_AddStringToObject(details, "SenderID",sms_sender_num);
		break;

		case JSON_UNAUTHORIZED_SENDER:
				details = cJSON_AddObjectToObject(root, "Details");
				cJSON_AddStringToObject(details, "UserId",user_id);
				cJSON_AddStringToObject(details, "Srn",service_id);
				cJSON_AddStringToObject(details, "Command",command_req);
				cJSON_AddStringToObject(details, "Status","REJECTED");
				cJSON_AddStringToObject(details, "UnAuthSenderID",sms_sender_num);	
		break;

		

		default :
			break;

	}


	_json_msg = cJSON_PrintUnformatted(root);
	// *message = _json_msg;
	cJSON_Delete(root);
	return (_json_msg);
	
 }


 static void Application__SetSDFolder_File_Names(char *ts, char *channel, char *flowid)
 {

	 // Extract the first token

		char *folder_name;
		char *file_name;
		folder_name = strtok(ts, ",");
		printf( " %s\n", folder_name ); //printing the token
		file_name = strtok(NULL, " ");
		printf( " %s\n", file_name ); //printing each token
		
		char ch = '-';  
		
		//Replace space with specific character ch  
		char FD[15] ="";
		char FL[100]="";
		char FL_TEMP[100];

	

		strcat(FL_TEMP, channel);
		strcat(FL_TEMP, "_");
		strcat(FL_TEMP, _mac);
		strcat(FL_TEMP, "_");
		strcat(FL_TEMP, flowid);
		strcat(FL_TEMP, "_");

		strcpy(FD,folder_name);
		strcpy(FL,file_name);

		for(int i = 0; i < strlen(FD); i++){  
			if(FD[i] == '/')  
				FD[i] = ch;  
		} 

		for(int i = 0; i < strlen(FL); i++){  
			if(FL[i] == ':')  
				FL[i] = ch;  
		}  

		strcat(FL_TEMP,FL);

		strcpy(File_N,FL_TEMP);
		strcpy(Folder_N,FD);

		// ESP_LOGI(TAG,"FILENAME ===%s",File_N);
		// ESP_LOGI(TAG,"FOLDER ===%s",FD);


		
		// *fl_name_out = FL_TEMP;
		// *fd_name_out = FD;


 }






static void system__extractConfigTextFile(char *msg, char* delim1, char* delim2, char **buf_out)
{

	const char *i1_1 = strstr(msg, delim1);
	if(i1_1 != NULL)
	{
		const size_t pl1 = strlen(delim1);
		const char *i2 = strstr(i1_1 + pl1, delim2);
		if(delim2 != NULL)
		{
			/* Found both markers, extract text. */
			const size_t mlen = i2 - (i1_1 + pl1);
			char *ret = malloc(mlen + 1);
			if(ret != NULL)
			{
				memcpy(ret, i1_1 + pl1, mlen);
				ret[mlen] = '\0';

				*buf_out = ret;
				
				// user_id = ret;

				// ESP_LOGI(TAG, "extracted user id : %s", user_id);

			}
		}
	}
	

}

static void Application__Init(void)
{
	memset(_Abhed_TextFile_Content, '\0', sizeof(_Abhed_TextFile_Content));
	memset(_OTP_TextFileContent, '\0', sizeof(_OTP_TextFileContent));

	SDCard__ReadFile(Abhed_Text_File);
	vTaskDelay(1000 / portTICK_RATE_MS);
	strcpy(_Abhed_TextFile_Content,SdCard__GetFileContent());
	SDCard__ReadFile(OTP_Text_File);
	vTaskDelay(1000 / portTICK_RATE_MS);
	strcpy(_OTP_TextFileContent,SdCard__GetFileContent());

	

	system__extractConfigTextFile(_Abhed_TextFile_Content,	FIRMWARE_VERSION_P1,	FIRMWARE_VERSION_P2, 	&_Firmware_Version);
	system__extractConfigTextFile(_Abhed_TextFile_Content,	MQTT_HOST_P1,			MQTT_HOST_P2, 			&_Mqtt_Host);
	system__extractConfigTextFile(_Abhed_TextFile_Content,	MQTT_PORT_P1,			MQTT_PORT_P2, 			&_Mqtt_Port);
	system__extractConfigTextFile(_Abhed_TextFile_Content,	MQTT_VHOST_P1,			MQTT_VHOST_P2, 			&_Mqtt_Vhost);
	system__extractConfigTextFile(_Abhed_TextFile_Content,	MQTT_USERNAME_P1,		MQTT_USERNAME_P2, 		&_Mqtt_Username);
	system__extractConfigTextFile(_Abhed_TextFile_Content,	MQTT_PASSWORD_P1,		MQTT_PASSWORD_P2, 		&_Mqtt_Password);
	system__extractConfigTextFile(_Abhed_TextFile_Content,	MQTT_ROUTING_KEY_P1,	MQTT_ROUTING_KEY_P2, 	&_Mqtt_RoutingKey);
	system__extractConfigTextFile(_Abhed_TextFile_Content,	BLENAME_P1,				BLENAME_P2, 			&_BleName);
	system__extractConfigTextFile(_Abhed_TextFile_Content,	MQTT_TELEMETRY_FREQ_P1,	MQTT_TELEMETRY_FREQ_P2, &_Telemetry_Freq);
	system__extractConfigTextFile(_Abhed_TextFile_Content,	MQTT_DIAGNO_FREQ_P1,	MQTT_DIAGNO_FREQ_P2, 	&_Diagnostic_Freq);
	system__extractConfigTextFile(_Abhed_TextFile_Content,	TRUSTED_SOURCE_P1,		TRUSTED_SOURCE_P2, 		&_Trusted_Source);
	system__extractConfigTextFile(_Abhed_TextFile_Content,	MOTOR_CW_STEPS_P1,		MOTOR_CW_STEPS_P2, 		&_Motor_CW_Steps);
	system__extractConfigTextFile(_Abhed_TextFile_Content,	MOTOR_CCW_STEPS_P1,		MOTOR_CCW_STEPS_P2, 	&_Motor_CCW_Steps);


	memset(_Abhed_TextFile_Content, '\0', sizeof(_Abhed_TextFile_Content));

	// printf("URL = %s", _Mqtt_Host);
	// printf("PORT = %s", _Mqtt_Port);
	// printf("USERNAME = %s", _Mqtt_Username);
	// printf("PASSWORD = %s", _Mqtt_Password);
	// printf("TOPIC = %s", _Mqtt_RoutingKey);

	printf("CW STEPS = %s", _Motor_CW_Steps);
	printf("CCW_STEPS = %s", _Motor_CCW_Steps);

}

static void Application__UpdateConfigurations(void)
{
	
	Application__Init();

	Application__GetImei1(&_Imei_1);
	Application__GetImsi1(&_Imsi_1);
	// Application__GetRssi1(&_Rssi_1);
	Application__GetOperator1(&_Operator_1);
	// Application__SetMqtt_url(_Mqtt_Host);
	// Application__SetMqtt_port(_Mqtt_Port);
	// Application__SetMqttRouting_Key(_Mqtt_RoutingKey);
	// Application__SetMqtt_Username(_Mqtt_Username);
  	// Application__SetMqtt_Password(_Mqtt_Password);
	// Application__SetMqtt_Vhost(_Mqtt_Vhost);
	Application__SetBleDevice_Name(_BleName);

	sprintf(_mac,"%d%d%d%d%d%d",MAC__GetBaseMACAddress(0),MAC__GetBaseMACAddress(1), MAC__GetBaseMACAddress(2),MAC__GetBaseMACAddress(3), MAC__GetBaseMACAddress(4),MAC__GetBaseMACAddress(5) );

}

static void Application__GetImei1(char ** buff)
{
	char *temp;
	
	GsmGprsManager__GetImei_1(&temp);
	*buff = temp;
	

}

static void Application__GetImsi1(char ** buff)
{
	char *temp;
	
	GsmGprsManager__GetImsi_1(&temp);
	*buff = temp;

}

static void Application__GetRssi1(char ** buff)
{

	uint32_t rssi_level;
	rssi_level = GsmGprsManager__GetRssi_1();
	
}

static void Application__GetOperator1(char ** buff)
{
	char *temp;
	GsmGprsManager__GetOperator_1(&temp);
	*buff = temp;

}

// static void Application__SetMqtt_url(const char *url)
// {
// 	GsmGprsManager__SetMqttBroker_URL(url);
// }

// static void Application__SetMqtt_port(char *port)
// {
// 	uint32_t port_num = atoi(port);
// 	GsmGprsManager__SetMqtt_PORT(port_num);

// }

// static void Application__SetMqttRouting_Key(char *routing_key)
// {
// 	GsmGprsManager__SetMqttRouting_Key(routing_key);

// }

// static void Application__SetMqtt_Username(char *username)
// {
// 	GsmGprsManager__SetMqttUsername(username);

// }

// static void Application__SetMqtt_Password(char *password)
// {
// 	GsmGprsManager__SetMqttPassword(password);

// }

// static void Application__SetMqtt_Vhost(char *vhost)
// {
// 	GsmGprsManager__SetMqttVhost(vhost);

// }
static void Application__SetBleDevice_Name(char *blename)
{
	// BleManager__SetBleName(blename);
}



//=================================== globals =================================
void Application__BleEnable()
{
	EspBle__SetDeviceName(_BleName);
	EspBle__Enable();
}

void Application__BleDisable()
{
	EspBle__Disable();
}

bool Appllication__CheckOTP(char *otp_file_content, char *OTP_STR)
{
	ESP_LOGI(TAG, "OTP FILE CONTENT === %s", otp_file_content);
	bool state = false;
	char str_otp[20];
	memset(str_otp, '\0', sizeof(str_otp));

	strcat(str_otp,"<");strcat(str_otp,OTP_STR);strcat(str_otp,">");
	
	const char *strcheck = strstr(otp_file_content, str_otp);
	if(strcheck != NULL)
	{
		ESP_LOGI(TAG,"--OTP found--");
		state = true;

		size_t len = strlen(str_otp);
		if (len > 0) {
			char *p = otp_file_content;
			while ((p = strstr(p, str_otp)) != NULL) {
				memmove(p, p + len, strlen(p + len) + 1);
			}
		}

		strcat(otp_file_content,"<#");strcat(otp_file_content,OTP_STR);strcat(otp_file_content,">");
		ESP_LOGI(TAG,"File To Write out : %s", otp_file_content);
		//write to OTP file
		SDCard__WriteToFile(OTP_Text_File,otp_file_content);

	}
	else{
		ESP_LOGI(TAG,"--OTP NOT found--");
		state = false;
	}
	return state;
}

char *strremove(char *str, const char *sub) {
    size_t len = strlen(sub);
    if (len > 0) {
        char *p = str;
        while ((p = strstr(p, sub)) != NULL) {
            memmove(p, p + len, strlen(p + len) + 1);
        }
    }
    return str;
}

char *Application__strremove(char *str, const char *sub) {
    

	char substr[20] = "1234", replace[20] = "#1234 ", output[20];
       
	    int i = 0, j = 0, flag = 0, start = 0;
        
        str[strlen(str) - 1] = '\0';
        substr[strlen(substr) - 1] = '\0';
        replace[strlen(replace) - 1] =  '\0';

        // check whether the substring to be replaced is present 
        while (str[i] != '\0')
        {
                if (str[i] == substr[j]) 
                {
                        if (!flag)
                                start = i;
                        j++;
                        if (substr[j] == '\0')
                                break;
                        flag = 1;
                } 
                else 
                {
                        flag = start = j = 0;
                }
                i++;
        }
        if (substr[j] == '\0' && flag)
        {
                for (i = 0; i < start; i++)
                        output[i] = str[i];

                // replace substring with another string 
                for (j = 0; j < strlen(replace); j++) 
                {
                        output[i] = replace[j];
                        i++;
                }
                // copy remaining portion of the input string "str" 
                for (j = start + strlen(substr); j < strlen(str); j++)
                {
                        output[i] = str[j];
                        i++;
                }
                // print the final string 
                output[i] = '\0';
                printf("Output: %s\n", output);
        } else {
                printf("%s is not a substring of %s\n", substr, str);
        }
    return output;
}


void System__BootUpLedSequence(void)
{
	led_fashion();
}


static void Application__StepperClockwise(uint32_t steps)
{
	A4988__SetDir(1);
	for(uint32_t stp = 0; stp < steps; stp++)
	{
		A4988__StepPulseHIGH();
		vTaskDelay(10 / portTICK_RATE_MS);
		A4988__StepPulseLOW();
		vTaskDelay(10 / portTICK_RATE_MS);

	}

}

static void Application__StepperAntiClockwise(uint32_t steps)
{

	A4988__SetDir(0);
	for(uint32_t stp = 0; stp < steps; stp++)
	{
		A4988__StepPulseHIGH();
		vTaskDelay(10 / portTICK_RATE_MS);
		A4988__StepPulseLOW();
		vTaskDelay(10 / portTICK_RATE_MS);

	}

}









