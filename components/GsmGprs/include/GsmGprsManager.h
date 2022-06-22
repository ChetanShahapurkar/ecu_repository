/**
 *  @file       
 *
 *  @brief      GsmGprsManager header with all the APIS
 *
 *  @section    Applicable_Documents
 *					List here all the applicable documents if needed. <tr>	
 *
 *  $Header: GsmGprsManager.h 1.4 2014/12/05 23:59:15IST Chetana S Bhat (BHATCS) Exp  $
 *
 *  @copyright  Copyright 2014-$Date: 2014/12/05 23:59:15IST $. Whirlpool Corporation. All rights reserved - CONFIDENTIAL
 */
//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------
#ifndef GsmGprsManager_H_
#define GsmGprsManager_H_
#include "sdkconfig.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "GsmGprsEngine_prm.h"
#include "system.h"
//=====================================================================================================================
#define MQTT_DISCONNECT_WAIT_TIME               1500
#define MQTT_TEMP_TOPIC_DISCONNECT_WAIT_TIME    90000
#define GSM_NETWORK_CHECK_TIME                  5000
#define HANDSHAKE_MSG                           "WAITING FOR DATA"
#define CONFIGURATION_UPDATE_MSG                "CONFIG SUCCESS"
//=====================================================================================================================
//! Unique names used to identify states of the System Manager state machine.
typedef enum SYSTEM_MANAGER_STATE_ENUM
{
    STATE_GSMGPRS_INITIALIZE,
    STATE_GSMGPRS_STANDBY,
    STATE_GSMGPRS_CHECKNETWORK         ,
    STATE_GSMGPRS_SETUP_NETWORK_TIME,
    STATE_GSMGPRS_RUNNING,
    STATE_GSMGPRS_MODEMINIT            ,
    STATE_GSMGPRS_MQTT_CONNECT         ,
    STATE_GSMGPRS_MQTT_DISCONNECT      ,
    STATE_GSMGPRS_MQTT_RECONNECT       ,
    STATE_GSMGPRS_MQTT_SEND_MESSAGE    ,
    STATE_GSMGPRS_NEWSMS_RECEIVED      ,
    STATE_GSMGPRS_CONNECT_NEW_TOPIC    ,
} SYSTEM_MANAGER_STATE_TYPE;

//! Unique names used to identify events used in the System Manager state machine.
typedef enum SYSTEM_MANAGER_EVENT_ENUM
{
    EV_GSMGPRS_INIT_COMPLETE       ,
    EV_GSMGPRS_STANDBY             ,
    EV_GSMGPRS_CHECKNETWORK        ,
    EV_GSMGPRS_SETUP_NETWORK_TIME,
    EV_GSMGPRS_RUN                 ,
    EV_GSMGPRS_MODEMINIT           ,
    
    EV_GSMGPRS_MQTTCONNECT         ,
    EV_GSMGPRS_MQTTDISCONNECT      ,
    EV_GSMGPRS_MQTTRECONNECT       ,
    EV_GSMGPRS_MQTTSEND_MESSAGE    ,
    EV_GSMGPRS_NEWSMS_RECEIVED     ,
    EV_GSMGPRS_CONNECT_NEW_TOPIC    
} SYSTEM_MANAGER_EVENT_TYPE;

//=====================================================================================================================
//-------------------------------------- PUBLIC (Function Prototypes) -------------------------------------------------
//=====================================================================================================================
void GsmGprsManager__Initialize(void);
SYSTEM_MANAGER_STATE_TYPE GsmGprsManager__GetState(void);
void GsmGprsManager__Handler(void);
bool GsmGprsManager__initStatus(void);
void GsmGprsManager__PostEvent(SYSTEM_MANAGER_EVENT_TYPE event);
void GsmGprsManager__ProcessExternalEvent(SYSTEM_MANAGER_EVENT_TYPE ev,char *msg1, char *msg2, char *tp1, char *tp2, char *mqtthost, char *mqttVhost, char *mqttport, char *mqttusername, char *mqtthostpassword);
bool GsmGprsManager__GetMsgSendStatus();
void GsmGprsManager__GetMsgToSend(char **msg1, char ** msg2);
void GsmGprsManager__GetTopicsSend(char **topic1, char ** topic2) ;
void GsmGprsManager__GetMQTTConfiguration(char **mqtthost, char ** mqttVhost, char ** mqttport, char ** mqttusername, char ** mqttpass) ;
char* GsmGprsManager__GetSecondMsgToSend(void ) ;

const char * GsmGprsManager__GetLattitude(void);
const char * GsmGprsManager__GetLongitude(void);
void GsmGprsManager__ConnectToQueue(void);
char * GsmGprsManager__GetEpochTime(void);

void GsmGprsManager__GetImei_1(char **buff);
char * GsmGprsManager__GetImei_2(void);
uint32_t GsmGprsManager__GetRssi_1(void);
void GsmGprsManager__GetOperator_1(char **buff);
void GsmGprsManager__GetImsi_1(char **buff);


char * GsmGprsManager__GetRssi_2(void);

void GsmGprs__SetCurrentCMD(char * cmd);
void GsmGprs__GetCurrentCMD(char **buff);

bool GsmGprs__GetInitStatus();
void GsmGprsManager__GetNetworkTime(char **network_time);
void GsmGprsManager__SetNetworkStatus(bool status);
bool GsmGprsManager__GetNetworkStatus(void);

#endif // GsmGprsManager_H_


