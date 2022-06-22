/**
 *  @file       
 *
 *  @brief      StepperMotorManager header with all the APIS
 *
 *  @section    Applicable_Documents
 *					List here all the applicable documents if needed. <tr>	
 *
 *  $Header: StepperMotorManager.h 1.4 2014/12/05 23:59:15IST Chetan S
 *
 *  @copyright  Copyright 2014-$Date: 2014/12/05 23:59:15IST $. Electrosal. All rights reserved - CONFIDENTIAL
 */
//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------
#ifndef _STEPPERMOTORMANAGER_H
#define _STEPPERMOTORMANAGER_H

#include "sdkconfig.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "StepperMotorEngine_prm.h"
#include "A4988.h"

//=====================================================================================================================
#define BLENDER_MODEL								GEE_NUMBER_OF_SPEED_POSITIONS_0
#define BLENDER_ANDROMEDA							65 // Decimal value for A, ie 'A'
#define BLENDER_PANDORA								80 // Decimal value for P, ie 'P'

#define BOARD_MODEL									BOARD_DV3

#define BOARD_DV1									0 // Use PV1 or DV3
#define BOARD_DV2									1 // Use PV1 or DV3
#define BOARD_DV3									2 // Use PV1 or DV3
#define BOARD_PV1									3 // Use PV1 or DV3
//-------------------------------------- PUBLIC (Extern Variables, Constants & Defines) -------------------------------
//=====================================================================================================================
//! Unique names used to identify states of the System Manager state machine.
typedef enum StepperMotor_Manager_STATE_ENUM
{
    STATE_STEPPERMOTOR_INITIALIZE       = 0,
    STATE_STEPPERMOTOR_STAND_BY         = 1,
    STATE_STEPPERMOTOR_CW_STEPBASED     = 2,
    STATE_STEPPERMOTOR_CCW_STEPBASED    = 3,
    STATE_STEPPERMOTOR_RUNNING          = 4,
    STATE_STEPPERMOTOR_DIAGNOSTIC       = 5,
    STATE_STEPPERMOTOR_STOP             = 6
} StepperMotor_Manager_STATE_TYPE;

//! Unique names used to identify events used in the System Manager state machine.
typedef enum StepperMotor_Manager_EVENT_ENUM
{
    EV_STEPPERMOTOR_INIT_COMPLETE       = 0,
    EV_STEPPERMOTOR_CW_STEPBASED        = 1,
    EV_STEPPERMOTOR_CCW_STEPBASED       = 2,
    EV_STEPPERMOTOR_STANDBY             = 3,
    EV_STEPPERMOTOR_RUN                 = 4,
    EV_STEPPERMOTOR_DIAGNOSTIC          = 5,
    EV_STEPPERMOTOR_STOP                = 6,
} StepperMotor_Manager_EVENT_TYPE;

//=====================================================================================================================
//-------------------------------------- PUBLIC (Function Prototypes) -------------------------------------------------
//=====================================================================================================================
void StepperMotorManager__Initialize(void);
StepperMotor_Manager_STATE_TYPE StepperMotorManager__GetState(void);
void StepperMotorManager__Handler(void);
void StepperMotorManager__PostEvent(StepperMotor_Manager_EVENT_TYPE event);
void StepperMotorManager__ExternalProcess(uint8_t hw_id, A4988_STEPPERMOTOR_OPEARTION_TYPE opr_type, uint32_t steps, uint32_t speed);
bool StepperMotorManager__initStatus(void);
#endif // StepperMotorManager_H_


