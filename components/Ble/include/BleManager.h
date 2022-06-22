/**
 *  @file       
 *
 *  @brief      BleManager header with all the APIS
 *
 *  @section    Applicable_Documents
 *					List here all the applicable documents if needed. <tr>	
 *
 *  $Header: BleManager.h 1.4 2014/12/05 23:59:15IST Chetan S
 *
 *  @copyright  Copyright 2014-$Date: 2014/12/05 23:59:15IST $. Electrosal. All rights reserved - CONFIDENTIAL
 */
//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------
#ifndef BleManager_H_
#define BleManager_H_
#include "BleEngine_prm.h"

#include "sdkconfig.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"


//=====================================================================================================================
//! Unique names used to identify states of the System Manager state machine.
typedef enum BLE_MANAGER_STATE_ENUM
{
    STATE_BLE_INITIALIZE = 0,
    STATE_BLE_STAND_BY   = 1,
    STATE_BLE_PUBLISH_MSG      = 2,
    STATE_BLE_GET_MSG      = 3,
    STATE_BLE_RUNNING    = 4,
    STATE_BLE_DIAGNOSTIC = 5,
    STATE_BLE_STOPPING   = 6,
    STATE_BLE_NEWMSG_RECEIVED = 7,
} BLE_MANAGER_STATE_TYPE;

//! Unique names used to identify events used in the System Manager state machine.
typedef enum BLE_MANAGER_EVENT_ENUM
{
    EV_BLE_INIT_COMPLETE        = 0,
    EV_BLE_INT_ERROR            = 1,
    EV_BLE_PUBLISH_MSG          = 2,
    EV_BLE_GET_MSG              = 3,
    EV_BLE_RUN                  = 4,
    EV_BLE_DIAGNOSTIC           = 5,
    EV_BLE_STOP                 = 6,
    EV_BLE_NEWMSG_RECEIVED      = 7,
} BLE_MANAGER_EVENT_TYPE;

//=====================================================================================================================
//-------------------------------------- PUBLIC (Function Prototypes) -------------------------------------------------
//=====================================================================================================================
void BleManager__Initialize(void);
BLE_MANAGER_STATE_TYPE BleManager__GetState(void);
void BleManager__Handler(void *pvParameter);
void BleManager__PostEvent(BLE_MANAGER_EVENT_TYPE event);
BLE_MANAGER_STATE_TYPE BleManager__EventToSystem(char **buf);
#endif // BleManager_H_


