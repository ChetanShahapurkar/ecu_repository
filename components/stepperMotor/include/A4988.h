
#ifndef _A4988_H
#define _A4988_H

#include "Timers.h"

//=========================================== PUBLIC (Function Prototypes) ============================================
void A4988__Init(void);
void A4988__StepIn(void);
void A4988__SetDir(bool dir);
void A4988__StepPulse(bool pulse);
void A4988__StepPulseHIGH(void);
void A4988__StepPulseLOW(void);

//===================================================== DEFINES =======================================================
#define MAX_A4988STEPPERMOTOR_COUNT     2

//=====================================================================================================================

typedef enum
{
    //This is Linked with "StepperMotor_Manager_STATE_ENUM", do not change the sequence or number
	
	STEPPER_COUNTERCLOCKWISE    = 0,	
    STEPPER_CLOCKWISE           = 1,	
} A4988_STEPPERMOTOR_DIRECTION_BIT_TYPE;


typedef enum
{
    //This is Linked with "StepperMotor_Manager_STATE_ENUM", do not change the sequence or number
	OPERATION__STEPPER_CW_STEPBASED       = 1,
	OPERATION__STEPPER_CCW_STEPBASED      = 2,
    OPERATION__STEPPER_STOP               = 6,		
	OPERATION__STEPPER_NULL                 // dont use this operation from external module, not even from system, this is internally used by StepperMotorManager             
} A4988_STEPPERMOTOR_OPEARTION_TYPE;


struct A4988_STEPPERMOTOR_STRUCT_TYPE
{
    uint8_t Hardware_Id;
    MS_TIMER_LABEL Timer_num;
    A4988_STEPPERMOTOR_OPEARTION_TYPE Operation;
    uint32_t MotorSteps;
    uint32_t MotorSpeed;
    bool Stop;
    bool Direction;
    bool ToogleBit;
    bool PendingReq;
    bool MotorLock; // Locks the motor after finishing the rotation
   
};


#endif