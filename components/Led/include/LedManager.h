/**
 *  @file      
 *
 *  @brief      LedManager header with all the APIS
 *
 *  @section    Applicable_Documents
 *                  List here all the applicable documents if needed. <tr>  
 *
 *  $Header: LedManager.h 1.4 2014/12/05 23:59:15IST Chetan S
 *
 *  @copyright  Copyright 2014-$Date: 2014/12/05 23:59:15IST $. Electrosal. All rights reserved - CONFIDENTIAL
 */
//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------
#include "sdkconfig.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "LedEngine_prm.h"
#include "LedWS2812.h"
//=====================================================================================================================
#define MAX_LED_COUNT                               10

//-------------------------------------- PUBLIC (Extern Variables, Constants & Defines) -------------------------------
//=====================================================================================================================
//! Unique names used to identify states of the System Manager state machine.
typedef enum LED_MANAGER_STATE_ENUM
{
    //This is Linked with "OPEARTION_TYPE", do not change the sequence
    STATE_LED_INITIALIZE    = 0,
    STATE_LED_TURNON        = 1,
    STATE_LED_TURNOFF       = 2,
    STATE_LED_FLASH         = 3,
    STATE_LED_RUNNING       = 4,
    STATE_LED_FADEIN        = 5,
    STATE_LED_FADOUT        = 6,
    STATE_LED_IDLE          = 7
} LED_MANAGER_STATE_TYPE;
 
//! Unique names used to identify events used in the System Manager state machine.
typedef enum LED_MANAGER_EVENT_ENUM
{
    EV_LEDINIT_COMPLETE     = 0,
    EV_LED_ON               = 1,
    EV_LED_OFF              = 2,
    EV_LED_FLASH            = 3,
    EV_LED_RUN              = 4,
    EV_LED_FADEIN           = 5,
    EV_LED_FADEOUT                 = 6,
    EV_LED_DIAGNOSTIC      = 7,
    EV_LED_IDLE          = 8
} LED_MANAGER_EVENT_TYPE;
 
//=====================================================================================================================
//-------------------------------------- PUBLIC (Function Prototypes) -------------------------------------------------
//=====================================================================================================================
void LedManager__Initialize(void);
LED_MANAGER_STATE_TYPE LedManager__GetState(void);
void LedManager__Handler(void);
void LedManager__PostEvent(LED_MANAGER_EVENT_TYPE event);
void LedManager__ExternalProcess(uint8_t hw_id, OPEARTION_TYPE opr_type, uint32_t t_on, uint32_t t_off, uint8_t Color_R, uint8_t Color_G , uint8_t Color_B);
bool LedManager__initStatus(void);
 
 
 

