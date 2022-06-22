/**
 *  @file
 *
 *  @brief      Parameterization of the State Engine
 *
 *
 *  $Header: StepperMotorEngine_prm.h 1.3 2014/12/01 14:17:48IST Chetan S
 *
 *  @copyright  *****  Copyright 2011.  Electrosal.  All rights reserved - CONFIDENTIAL  *****
 */
//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------
#ifndef StepperMotorEngine_PRM_H_
#define StepperMotorEngine_PRM_H_


//=====================================================================================================================
//-------------------------------------- PUBLIC (Extern Variables, Constants & Defines) -------------------------------
//=====================================================================================================================
#define SE_EVENT_QUEUE ENABLED

//! Define to allow threadsafe queue handling - if enabled it allows events to be safely posted from an interrupt
//#define SE_INTERRUPT_SAFE_EXECUTE ENABLED
#define SE_INTERRUPT_SAFE_EXECUTE DISABLED

//! Define to allow large event collection - if enabled it allows events to be safely posted from an interrupt
//#define SE_LARGE_EVENT_POOL ENABLED
#define SE_LARGE_EVENT_POOL DISABLED

#ifndef END_OF_TRANSITION_TABLE
//! A definition to be used as a terminator in transition tables.
//! It can be placed in StepperMotorEngine_prm.h until the issue is resolved.
#define END_OF_TRANSITION_TABLE     {EVENT_NONE, GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_NONE}
#endif

//=====================================================================================================================
//-------------------------------------- PUBLIC (Function Prototypes) -------------------------------------------------
//=====================================================================================================================


#endif /* StepperMotorEngine_PRM_H_ */