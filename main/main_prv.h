/*
 * Main_prv.h
 *
 *  Created on: Jan 30, 2020
 *      Author: SHAHACG
 */

#ifndef COMPONENTS_SOURCE_LIBRARYPRM_MAIN_PRV_H_
#define COMPONENTS_SOURCE_LIBRARYPRM_MAIN_PRV_H_

#include "../../SdCard/include/SdCard.h"
#include "../../Led/include/LedManager.h"
#include "../../stepperMotor/include/StepperMotorManager.h"
#include "../../GsmGprs/include/GsmGprsManager.h"
#include "../../CurrentSense/include/CurrentSense.h"


//=====================================================================================================================
//-------------------------------------- PRIVATE (Variables, Constants & Defines) -------------------------------------
//=====================================================================================================================


//  --- Compiler Directives

#define MAIN_LOW_POWER_FEATURE  DISABLED

//! This definition should be set to one of the following:
//!     ENABLED = Reset the stack pointer in noise immunity section of main loop (Preferred)
//!     DISABLED = Do not reset the stack pointer in the main loop.
#define MAIN_RESETS_STACK_POINTER ENABLED

//! This definition should be set to one of the following:
//!     ENABLED = Execute a set of tasks in every interrupt as a separate group.
//!     DISABLED = Any tasks that must be executed in every interrupt should be included in every
//!                 slot.  This results in less overhead.
#define EXECUTE_EVERY_INTERRUPT_TASKS ENABLED

//! This definition should be set to one of the following:
//!     ENABLED = Execute a set of tasks in every main loop as a separate group.
//!     DISABLED = There are no tasks executed in every main loop.
#define EXECUTE_EVERY_MAIN_LOOP_TASKS ENABLED


//  --- Macro Definitions

//! Add a routine to perform noise immunity tasks such as (for example):
//!     Check stack pointer
//!     Enable interrupts
//!     Refresh static registers
//!     Check critical values
//!     Background ROM check
//!     etc.
//!     Caution: Make sure the code added to the NOISE_IMMUNITY() doesn't affect the slot timing control!!
#define NOISE_IMMUNITY()


//! Add a routine to perform timing check on Main Slots
//! This routine will be called before any instruction with the slot synchronization
#define MAIN_DEBUG_BEFORE_SLOTS()

//! Add a routine to perform timing check on Main Slots
//! This routine will be called after all instructions with the slot synchronization
#define MAIN_DEBUG_AFTER_SLOTS()


//! Add a routine to perform timing check on Timer Interruption Slots
//! This routine will be called after all instructions in the Timer Interruption
#define MAIN_DEBUG_AFTER_INTERRUPT_SLOTS()


//! Add a routine to perform timing check on Timer Interruption Slots
//! This routine will be called after all instructions in the Timer Interruption
#define EXCEPTION_MAIN_SLOT_VIOLATION(actual,expected)


//  --- Constants

//! Number of interrupts per millisecond.  This value should be a whole number.
//! Example: If interrupt period is 250 microseconds, then this value should be 4.
#define INTERRUPTS_PER_MILLISECOND 4

//! Number of milliseconds per main slot.  This value should be a whole number.
//! Example: If each main slot takes 5 milliseconds, then this value should be 5.
#define MILLISECONDS_PER_MAIN_SLOT 5


//  --- Variables

/** @brief      List of tasks executed before entering the main loop.
 *  @details    Add pointers to routines that should be called during initialization.  These
 *              routines are intended to initialize or begin the initialization process for all the
 *              modules in the application.
 */
TASK_TYPE Initialization_Tasks[] =
{
		//-------------------------------------------------------------------------
		// Mandatory NULL_TASK must be at the end of the list
        Timers__Initialize,
        SDCardManager__Init,
        LedWS2812__Init,
        A4988__Init,
        LedManager__Initialize,
        // StepperMotorManager__Initialize,
        GsmGprsManager__Initialize,
		// CurrentSense__init,
        System__Init,
		NULL_TASK
};

#if (EXECUTE_EVERY_MAIN_LOOP_TASKS == ENABLED)
/** @brief      List of tasks executed at the start of every pass of the main loop.
 *  @details    Add pointers to routines that should run every main loop.
 */
TASK_TYPE Every_Loop_Tasks[] =
{
		//-------------------------------------------------------------------------
		// Mandatory NULL_TASK must be at the end of the list
        System__BootUpLedSequence,

		NULL_TASK
};
#endif                                              // EXECUTE_EVERY_MAIN_LOOP_TASKS

/** @brief      List of tasks executed at the start of every main loop slot.
 *  @details    Add pointers to routines that should be called every main loop slot.
 *  			Runs every  5msec
 */
TASK_TYPE Every_Slot_Tasks[] =
{
		//-------------------------------------------------------------------------
		// Mandatory NULL_TASK must be at the end of the list
		//	Reveal__HandlerIncomming,

//		Reveal__HandlerOutgoing,
		NULL_TASK
};

/** @brief      List of tasks executed in SLOT #0 of the main loop.
 *  @details    Add pointers to routines that should run during this slot in the main loop.
 */
TASK_TYPE Main_Slot0_Tasks[] =
{
		//-------------------------------------------------------------------------
		// Mandatory NULL_TASK must be at the end of the list
        LedManager__Handler,
        SDCardManager__Handler,
		NULL_TASK
};

/** @brief      List of tasks executed in SLOT #1 of the main loop.
 *  @details    Add pointers to routines that should run during this slot in the main loop.
 */
TASK_TYPE Main_Slot1_Tasks[] =
{
		//-------------------------------------------------------------------------
		// Mandatory NULL_TASK must be at the end of the list
        GsmGprsManager__Handler,
		NULL_TASK
};

/** @brief      List of tasks executed in SLOT #2 of the main loop.
 *  @details    Add pointers to routines that should run during this slot in the main loop.
 */
TASK_TYPE Main_Slot2_Tasks[] =
{
		//-------------------------------------------------------------------------
		// Mandatory NULL_TASK must be at the end of the list
        system__Handler,
		NULL_TASK
};

/** @brief      List of tasks executed in SLOT #3 of the main loop.
 *  @details    Add pointers to routines that should run during this slot in the main loop.
 */
TASK_TYPE Main_Slot3_Tasks[] =
{
		//-------------------------------------------------------------------------
		// Mandatory NULL_TASK must be at the end of the list
        // StepperMotorManager__Handler,
		// CurrentSense_Handler,
		NULL_TASK
};

/** @brief      List of tasks executed in SLOT #4 of the main loop.
 *  @details    Add pointers to routines that should run during this slot in the main loop.
 */
TASK_TYPE Main_Slot4_Tasks[] =
{
		//-------------------------------------------------------------------------
		// Mandatory NULL_TASK must be at the end of the list
        Timers__ServiceSeconds,
		NULL_TASK
};

/** @brief      Collection of arrays of function pointers.  Each element represents a single slot
 *              in the main loop.
 *  @details    Each element of this array represents a slot in the main loop.  Each element is a
 *              list of tasks to execute in the main loop slot.  Add all the arrays of tasks that
 *              make up each slot in the application.  The size of this array determines the number
 *              of slots in the main loop.
 *
 *              Runs every 25 msec
 */
TASK_LIST_TYPE Main_Tasks[] =
{
		Main_Slot0_Tasks,
		Main_Slot1_Tasks,
		Main_Slot2_Tasks,
		Main_Slot3_Tasks,
		Main_Slot4_Tasks

		//-------------------------------------------------------------------------
		// Do not place a terminating NULL_TASK in this list.
};

#if (EXECUTE_EVERY_INTERRUPT_TASKS == ENABLED)
/** @brief      List of tasks executed at the start of every timer interrupt.
 *  @details    Add pointers to routines that should run every timer interrupt.
 *  			Runs every 250usec
 */
TASK_TYPE Every_Interrupt_Tasks[] =
{
		//-------------------------------------------------------------------------
		// Mandatory NULL_TASK must be at the end of the list
		NULL_TASK
};
#endif                                              // EXECUTE_EVERY_INTERRUPT_TASKS

/** @brief      List of tasks executed in SLOT #0 of the timer interrupt.
 *  @details    Add pointers to routines that should run during this slot in the timer interrupt.
 *              Tasks that should be executed in every interrupt slot should be included here if
 *              EVERY_INTERRUPT_TASKS_ENABLED is not defined.
 *              Runs every 1msec
 */
TASK_TYPE Interrupt_Slot0_Tasks[] =
{
		//-------------------------------------------------------------------------
		// Mandatory NULL_TASK must be at the end of the list
        Timers__ServiceMs,
		NULL_TASK
};

/** @brief      List of tasks executed in SLOT #1 of the timer interrupt.
 *  @details    Add pointers to routines that should run during this slot in the timer interrupt.
 *              Tasks that should be executed in every interrupt slot should be included here if
 *              EVERY_INTERRUPT_TASKS_ENABLED is not defined.
 *              Runs every 1msec
 */
TASK_TYPE Interrupt_Slot1_Tasks[] =
{
		//-------------------------------------------------------------------------
		// Mandatory NULL_TASK must be at the end of the list
        
		NULL_TASK
};

/** @brief      List of tasks executed in SLOT #2 of the timer interrupt.
 *  @details    Add pointers to routines that should run during this slot in the timer interrupt.
 *              Tasks that should be executed in every interrupt slot should be included here if
 *              EVERY_INTERRUPT_TASKS_ENABLED is not defined.
 *              Runs every 1msec
 */
TASK_TYPE Interrupt_Slot2_Tasks[] =
{
		//-------------------------------------------------------------------------
		// Mandatory NULL_TASK must be at the end of the list
		NULL_TASK
};

/** @brief      List of tasks executed in SLOT #3 of the timer interrupt.
 *  @details    Add pointers to routines that should run during this slot in the timer interrupt.
 *              Tasks that should be executed in every interrupt slot should be included here if
 *              EVERY_INTERRUPT_TASKS_ENABLED is not defined.
 *              Runs every 1msec
 */
TASK_TYPE Interrupt_Slot3_Tasks[] =
{
		//-------------------------------------------------------------------------
		// Mandatory NULL_TASK must be at the end of the list
		NULL_TASK
};

/** @brief      Collection of array of function pointers to execute in the interrupt slots.
 *  @details    Each element of this array represents a slot in the interrupt service routine.
 *              Each element is a list of tasks to execute in the interrupt slot.  Add all the
 *              arrays of tasks that make up each slot in the interrupt.  The size of this array
 *              determines the number of slots in the interrupt service routine.
 *              Each Task Runs every 1msec
 *
 */
TASK_LIST_TYPE Interrupt_Tasks[] =
{
		Interrupt_Slot0_Tasks,
		Interrupt_Slot1_Tasks,
		Interrupt_Slot2_Tasks,
		Interrupt_Slot3_Tasks

		//-------------------------------------------------------------------------
		// Do not place a terminating NULL_TASK in this list.
};

#endif /* COMPONENTS_SOURCE_LIBRARYPRM_MAIN_PRV_H_ */
