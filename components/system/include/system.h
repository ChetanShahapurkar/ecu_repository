#ifndef _SYSTEM_H
#define _SYSTEM_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void System__Init(void);
void system__Handler(void);
void System__BootUpLedSequence(void);


const char * Application__GetMQTTHostName(void);
const char * Application__GetMQTTPortNum(void);
const char * Application__GetMQTTUsername(void);
const char * Application__GetMQTTPassword(void);
const char * Application__GetMQTTVHost(void);

typedef enum JSON_MSG_ENUM
{
    JSON_BOOTUP_TELEMETRY_MESSAGE           = 0,
    JSON_BOOTUP_DISGNOSTIC_MESSAGE          = 1,
    JSON_SMS_GTEL_MESSAGE                   = 2,
    JSON_SMS_GDIG_MESSAGE                   = 3,
    JSON_SMS_MTRC_MESSAGE                   = 4,
    JSON_SMS_MTRA_MESSAGE                   = 5,
    JSON_SMS_SCFG_MESSAGE                   = 6,
    JSON_SMS_SOTP_MESSAGE                   = 7,

    JSON_SMS_GCFG_MESSAGE                   = 8,
    JSON_SMS_GOTP_MESSAGE                   = 9,

    JSON_SMS_MTRC__ALREADY_MOVED_MESSAGE    = 10,
    JSON_SMS_MTRA__ALREADY_MOVED_MESSAGE    = 11,

    JSON_UNAUTHORIZED_SENDER                   = 12,

    JSON_BLE_MTRC_MESSAGE                   =13,
    JSON_BLE_MTRA_MESSAGE                   =14,
    JSON_BLE_MTRC__ALREADY_MOVED_MESSAGE    = 15,
    JSON_BLE_MTRA__ALREADY_MOVED_MESSAGE    = 16,

    JSON_SMS_GSRC_MESSAGE                   = 17,
    


    
} JSON_MSG__TYPE;


void Application__BleEnable();
void Application__BleDisable();


#endif