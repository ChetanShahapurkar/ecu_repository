/**
 *  @file       
 *
 *  @brief      System Manager Module - the main Master application module
 *
 *  @details    StepperMotorManager is the main State Machine and controls the motor
 *
 *  @section    Applicable_Documents
 *					List here all the applicable documents if needed. <tr>	
 *
 *  $Header: StepperMotorManager.c 1.6 2014/12/05 23:59:09IST Chetan S
 *
 *  @copyright  Copyright 2014-$Date: 2014/12/05 23:59:09IST $. Electrosal. All rights reserved - CONFIDENTIAL
 */
//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------


//-------------------------------------- Include Files ----------------------------------------------------------------
// #include "../../C_Extensions.h"
#include "StepperMotorManager.h" 
#include "StepperMotorEngine.h"
#include "esp_log.h"

//-------------------------------------- PUBLIC (Variables) -----------------------------------------------------------

//-------------------------------------- PRIVATE (Variables, Constants & Defines) -------------------------------------

static const char *TAG = "StepperMotorManager"; 
bool StepperMotorManagerInitStatus = false;

static struct A4988_STEPPERMOTOR_STRUCT_TYPE OPERATE_A4988_STEPPERMOTOR[MAX_A4988STEPPERMOTOR_COUNT];
static struct A4988_STEPPERMOTOR_STRUCT_TYPE OPERATE_A4988_STEPPERMOTOR_TEMP;

static EVENT_TYPE  StepperMotorInitEntry (void);
static EVENT_TYPE  StepperMotorInitDo (EVENT_TYPE event);

static EVENT_TYPE  StepperMotorStandbyEntry (void);
static EVENT_TYPE  StepperMotorBleStandbyDo (EVENT_TYPE event);

static EVENT_TYPE  StepperMotorCCWStepBasedEntry(void);
static EVENT_TYPE  StepperMotorCCWStepBasedDo (EVENT_TYPE event);

static EVENT_TYPE  StepperMotorCWStepBasedEntry (void);
static EVENT_TYPE  StepperMotorCWStepBasedDo (EVENT_TYPE event);

static EVENT_TYPE  StepperMotorDiagnosticEntry (void);
static EVENT_TYPE  StepperMotorDiagnosticDo (EVENT_TYPE event);

static EVENT_TYPE  StepperMotorBleRunningEntry (void);
static EVENT_TYPE  StepperMotorBleRunningDo (EVENT_TYPE event);

static EVENT_TYPE  StepperMotorStopEntry (void);
static EVENT_TYPE  StepperMotorStopDo (EVENT_TYPE event);

//=====================================================================================================================
//=== StepperMotorEngineCrank Auto-Generated Code for the System Manager State Machine =======================================
//=====================================================================================================================

#if (SE_EVENT_QUEUE == ENABLED)
//! Maximum number of events that can be queued up in the System Manager state machine.
//! Implementation always executes as soon as the event is posted, so there is no reason for more than one.
#define StepperMotor_Manager_QUEUE_DEPTH 1
#endif      // #if (SE_QUEUE_ENABLED == ENABLED)





//! Data structure used by the StepperMotorEngine module to run the System Manager state machine.
static STATE_CONTROL_TYPE StepperMotor_Manager;

#if (SE_EVENT_QUEUE == ENABLED)
//! Allocate memory for the System Manager state machine's event queue.
static EVENT_TYPE StepperMotor_Manager_Queue[StepperMotor_Manager_QUEUE_DEPTH];
#endif      // #if (SE_EVENT_QUEUE == ENABLED)

//! List of transitions for Initialize State of the System Manager state machine.
static STATE_TRANSITION_TYPE TRANSITIONS_FOR_STATE_STEPPERMOTOR_INITIALIZE[] =
{
//  {Event Name,         Guard Function,  Transition Action,    Next State},
    {EV_STEPPERMOTOR_INIT_COMPLETE,     GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_STEPPERMOTOR_RUNNING},
    END_OF_TRANSITION_TABLE
};

//! List of transitions for Stand By State of the System Manager state machine.
static STATE_TRANSITION_TYPE TRANSITIONS_FOR_STATE_STEPPERMOTOR_STAND_BY[] =
{
//  {Event Name,          Guard Function,  Transition Action,    Next State},
    {EV_STEPPERMOTOR_CCW_STEPBASED,            GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_STEPPERMOTOR_CW_STEPBASED},
    {EV_STEPPERMOTOR_STANDBY,     GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_STEPPERMOTOR_CCW_STEPBASED},
    END_OF_TRANSITION_TABLE
};

//! List of transitions for Error State of the System Manager state machine.
static STATE_TRANSITION_TYPE TRANSITIONS_FOR_STATE_STEPPERMOTOR_CW_STEPBASED[] =
{
//  {Event Name,          Guard Function,  Transition Action,    Next State},
    {EV_STEPPERMOTOR_CCW_STEPBASED,         GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_STEPPERMOTOR_CCW_STEPBASED},
    {EV_STEPPERMOTOR_RUN,                   GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_STEPPERMOTOR_RUNNING},
    {EV_STEPPERMOTOR_STOP,                  GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_STEPPERMOTOR_STOP},
    {EV_STEPPERMOTOR_DIAGNOSTIC,       GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_STEPPERMOTOR_DIAGNOSTIC},
    END_OF_TRANSITION_TABLE
};

//! List of transitions for Ready State of the System Manager state machine.
static STATE_TRANSITION_TYPE TRANSITIONS_FOR_STATE_STEPPERMOTOR_CCW_STEPBASED[] =
{
//  {Event Name,          Guard Function,  Transition Action,    Next State},
    {EV_STEPPERMOTOR_CW_STEPBASED,         GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_STEPPERMOTOR_CW_STEPBASED},
    {EV_STEPPERMOTOR_RUN,                   GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_STEPPERMOTOR_RUNNING},
    {EV_STEPPERMOTOR_STOP,                  GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_STEPPERMOTOR_STOP},
    {EV_STEPPERMOTOR_DIAGNOSTIC,       GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_STEPPERMOTOR_DIAGNOSTIC},
    END_OF_TRANSITION_TABLE
};

//! List of transitions for Running State of the System Manager state machine.
static STATE_TRANSITION_TYPE TRANSITIONS_FOR_STATE_STEPPERMOTOR_RUNNING[] =
{
//  {Event Name,          Guard Function,  Transition Action,    Next State},
    {EV_STEPPERMOTOR_CW_STEPBASED,         GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_STEPPERMOTOR_CW_STEPBASED},
    {EV_STEPPERMOTOR_CCW_STEPBASED,         GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_STEPPERMOTOR_CCW_STEPBASED},
    {EV_STEPPERMOTOR_STOP,                  GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_STEPPERMOTOR_STOP},
    {EV_STEPPERMOTOR_DIAGNOSTIC,       GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_STEPPERMOTOR_DIAGNOSTIC},
    END_OF_TRANSITION_TABLE
};

//! List of transitions for Diagnostic State of the System Manager state machine.
static STATE_TRANSITION_TYPE TRANSITIONS_FOR_STATE_STEPPERMOTOR_DIAGNOSTIC[] =
{
//  {Event Name,          Guard Function,  Transition Action,    Next State},
    {EV_STEPPERMOTOR_CW_STEPBASED,         GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_STEPPERMOTOR_CW_STEPBASED},
    {EV_STEPPERMOTOR_CCW_STEPBASED,         GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_STEPPERMOTOR_CCW_STEPBASED},
    {EV_STEPPERMOTOR_STOP,                  GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_STEPPERMOTOR_STOP},
    {EV_STEPPERMOTOR_RUN,           GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_STEPPERMOTOR_RUNNING},
 
    END_OF_TRANSITION_TABLE
};

//! List of transitions for Stopping State of the System Manager state machine.
static STATE_TRANSITION_TYPE TRANSITIONS_FOR_STATE_STEPPERMOTOR_STOP[] =
{
//  {Event Name,         Guard Function,  Transition Action,    Next State},
    {EV_STEPPERMOTOR_STOP,     GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_STEPPERMOTOR_STOP},
    END_OF_TRANSITION_TABLE
};

//! State table for the System Manager state machine.
static STATE_TABLE_TYPE StepperMotor_Manager_STATE_TABLE[] =
{
//  {State Name,       {Enter Action,    Do Action,            Exit Action},    Transition Table},
    {STATE_STEPPERMOTOR_INITIALIZE,     {StepperMotorInitEntry,         StepperMotorInitDo,         EXIT_FUNC_NONE}, TRANSITIONS_FOR_STATE_STEPPERMOTOR_INITIALIZE},
    {STATE_STEPPERMOTOR_STAND_BY,       {StepperMotorStandbyEntry,      StepperMotorBleStandbyDo,      EXIT_FUNC_NONE}, TRANSITIONS_FOR_STATE_STEPPERMOTOR_STAND_BY},
    {STATE_STEPPERMOTOR_CW_STEPBASED,   {StepperMotorCWStepBasedEntry,  StepperMotorCWStepBasedDo,  EXIT_FUNC_NONE}, TRANSITIONS_FOR_STATE_STEPPERMOTOR_CW_STEPBASED},
    {STATE_STEPPERMOTOR_CCW_STEPBASED,  {StepperMotorCCWStepBasedEntry, StepperMotorCCWStepBasedDo, EXIT_FUNC_NONE}, TRANSITIONS_FOR_STATE_STEPPERMOTOR_CCW_STEPBASED},
    {STATE_STEPPERMOTOR_RUNNING,        {StepperMotorBleRunningEntry,      StepperMotorBleRunningDo,      EXIT_FUNC_NONE}, TRANSITIONS_FOR_STATE_STEPPERMOTOR_RUNNING},
    {STATE_STEPPERMOTOR_DIAGNOSTIC,     {StepperMotorDiagnosticEntry,   StepperMotorDiagnosticDo,   EXIT_FUNC_NONE}, TRANSITIONS_FOR_STATE_STEPPERMOTOR_DIAGNOSTIC},
    {STATE_STEPPERMOTOR_STOP,           {StepperMotorStopEntry,         StepperMotorStopDo,         EXIT_FUNC_NONE}, TRANSITIONS_FOR_STATE_STEPPERMOTOR_STOP}
};

//! State machine definition structure for the System Manager state machine.
static STATE_MACHINE_DEFINITION_TYPE StepperMotor_Manager_MACHINE =
{
    STATE_STEPPERMOTOR_INITIALIZE,                               // Initial state
    StepperMotor_Manager_STATE_TABLE,                     // Table defining state behaviors
    &StepperMotor_Manager,                                // Control structure
#if (SE_EVENT_QUEUE == ENABLED)
    StepperMotor_Manager_Queue,                           // Pointer to the event queue buffer
    StepperMotor_Manager_QUEUE_DEPTH,                     // Number of events that the event queue can hold
#endif      // #if (SE_EVENT_QUEUE == ENABLED)
};

// End of StepperMotorEngineCrank Auto-Generated Code.

//=====================================================================================================================
//-------------------------------------- Public Functions -------------------------------------------------------------
//=====================================================================================================================

//---------------------------------------------------------------------------------------------------------------------
/**
 *  @brief      It Initializes the module StepperMotorManager and its variables
 *
 */
void StepperMotorManager__Initialize(void)
{
    // Initialize the StepperMotorManager state-machine.
    (void)StepperMotorEngine__Configure(&StepperMotor_Manager_MACHINE);
}

/**
 *  @brief      Executes an iteration of the SysMan state machine
 *
 */
void StepperMotorManager__Handler(void)
{
    (void)StepperMotorEngine__Execute(&StepperMotor_Manager); 
}

/**
 *  @brief      reports the current state of the SysMan state machine
 *
 *  @return SYSMAN_STATE_TYPE
 */
StepperMotor_Manager_STATE_TYPE StepperMotorManager__GetState(void)
{
    return ((StepperMotor_Manager_STATE_TYPE)(StepperMotor_Manager.Cur_State));
}

/**
 *  @brief      posts events to the SysMan state machine
 *
 *  @param event StepperMotor_Manager_EVENT_TYPE
 */
void StepperMotorManager__PostEvent(StepperMotor_Manager_EVENT_TYPE event)
{
    StepperMotorEngine__PostEvent(&StepperMotor_Manager, (EVENT_TYPE) event);
}


//=====================================================================================================================
//-------------------------------------- Private Functions ------------------------------------------------------------
//=====================================================================================================================
/**
 *  @brief     StepperMotorInitEntry: entry into the Initialize state of the StepperMotorManager state machine
 *
 */
static EVENT_TYPE  StepperMotorInitEntry (void)
{

    
    //Initialize the LED structure 
    for(int index=0; index < MAX_A4988STEPPERMOTOR_COUNT;  index++)
    {
        OPERATE_A4988_STEPPERMOTOR[index].Hardware_Id = 0xFF;  // 0xFF is a null or Empty Hw_Id and can be used for operation
        OPERATE_A4988_STEPPERMOTOR[index].Operation = OPERATION__STEPPER_NULL;
        OPERATE_A4988_STEPPERMOTOR[index].MotorSteps = 0;
        OPERATE_A4988_STEPPERMOTOR[index].MotorSpeed = 0;
        OPERATE_A4988_STEPPERMOTOR[index].Stop = true;
        OPERATE_A4988_STEPPERMOTOR[index].PendingReq = false; // Clear the past pending request on initialize

    }

    OPERATE_A4988_STEPPERMOTOR_TEMP.Hardware_Id = 0xFF;  // 0xFF is a null or Empty Hw_Id and can be used for operation
    OPERATE_A4988_STEPPERMOTOR_TEMP.Operation = OPERATION__STEPPER_NULL;
    OPERATE_A4988_STEPPERMOTOR_TEMP.MotorSteps = 0;
    OPERATE_A4988_STEPPERMOTOR_TEMP.MotorSpeed = 0;
    OPERATE_A4988_STEPPERMOTOR_TEMP.Stop = true;
    OPERATE_A4988_STEPPERMOTOR_TEMP.PendingReq = false; // Clear the past pending request on initialize
    
    // ESP_LOGI(TAG,"Stepper Motor Init Entry");
    A4988__Init();
    return (EVENT_NONE);
}

/**
 *  @brief      StepperMotorInitDo: do func for the Initialize state of the StepperMotorManager state machine
 *
 *  @param event EVENT_TYPE
 */
static EVENT_TYPE  StepperMotorInitDo (EVENT_TYPE event)
{
    StepperMotorManager__PostEvent(EV_STEPPERMOTOR_INIT_COMPLETE);
    return (EVENT_NONE);
}

/**
 *  @brief      StepperMotorStandbyEntry: entry into the Standby state of the StepperMotorManager state machine
 *
 */
static EVENT_TYPE  StepperMotorStandbyEntry (void)
{
  
    return (EVENT_NONE);
}

/**
 *  @brief      StepperMotorBleStandbyDo: do func for the Standby state of the StepperMotorManager state machine
 *
 *  @param event EVENT_TYPE
 */
static EVENT_TYPE  StepperMotorBleStandbyDo (EVENT_TYPE event)
{
  
    return (EVENT_NONE);
}

/**
 *  @brief      StepperMotorCCWStepBasedEntry: entry into the Ready state of the StepperMotorManager state machine
 *
 */
static EVENT_TYPE  StepperMotorCCWStepBasedEntry (void)
{

    // ESP_LOGI(TAG, "CCW Entry" );

    if(OPERATE_A4988_STEPPERMOTOR_TEMP.Operation == OPERATION__STEPPER_CCW_STEPBASED)
    {
        for(int index = 0; index < MAX_A4988STEPPERMOTOR_COUNT; index++)
        {
            if((OPERATE_A4988_STEPPERMOTOR[index].Hardware_Id == 0xFF) ) // 0xFF is a null id, which can be used for operation
            {
                OPERATE_A4988_STEPPERMOTOR[index].Hardware_Id = OPERATE_A4988_STEPPERMOTOR_TEMP.Hardware_Id;
                OPERATE_A4988_STEPPERMOTOR[index].Operation = OPERATE_A4988_STEPPERMOTOR_TEMP.Operation;
                OPERATE_A4988_STEPPERMOTOR[index].MotorSteps = OPERATE_A4988_STEPPERMOTOR_TEMP.MotorSteps;
                OPERATE_A4988_STEPPERMOTOR[index].MotorSpeed = OPERATE_A4988_STEPPERMOTOR_TEMP.MotorSpeed;
                OPERATE_A4988_STEPPERMOTOR[index].Stop = OPERATE_A4988_STEPPERMOTOR_TEMP.Stop;
                

                for(int timer_num = MS__TIMER_A4988MOTOR_1; timer_num < MS__TIMER_A4988MOTOR_MAX; timer_num ++)
                {
                    if(Timers__MsGetStatus(timer_num) == TIMERS_IDLE)
                    {
                        OPERATE_A4988_STEPPERMOTOR[index].Timer_num = timer_num;
                        Timers__MsSet(OPERATE_A4988_STEPPERMOTOR[index].Timer_num, OPERATE_A4988_STEPPERMOTOR[index].MotorSpeed);//Load the timer with initial T_ON value
                        break;
                    }

                }
                break;
            }
            else if((OPERATE_A4988_STEPPERMOTOR[index].Hardware_Id == OPERATE_A4988_STEPPERMOTOR_TEMP.Hardware_Id))
            {

                OPERATE_A4988_STEPPERMOTOR[index].Hardware_Id = OPERATE_A4988_STEPPERMOTOR_TEMP.Hardware_Id;
                OPERATE_A4988_STEPPERMOTOR[index].Operation = OPERATE_A4988_STEPPERMOTOR_TEMP.Operation;
                OPERATE_A4988_STEPPERMOTOR[index].MotorSteps = OPERATE_A4988_STEPPERMOTOR_TEMP.MotorSteps;
                OPERATE_A4988_STEPPERMOTOR[index].MotorSpeed = OPERATE_A4988_STEPPERMOTOR_TEMP.MotorSpeed;
                OPERATE_A4988_STEPPERMOTOR[index].Stop = OPERATE_A4988_STEPPERMOTOR_TEMP.Stop;

                if(OPERATE_A4988_STEPPERMOTOR[index].Timer_num == MS__TIMER_NULL)
                {
                    for(int timer_num = MS__TIMER_A4988MOTOR_1; timer_num < MS__TIMER_A4988MOTOR_MAX; timer_num ++)
                    {
                        if(Timers__MsGetStatus(timer_num) == TIMERS_IDLE)
                        {
                            OPERATE_A4988_STEPPERMOTOR[index].Timer_num = timer_num;
                            Timers__MsSet(OPERATE_A4988_STEPPERMOTOR[index].Timer_num, OPERATE_A4988_STEPPERMOTOR[index].MotorSpeed);//Load the timer with initial T_ON value
                            break;
                        }

                    }

                }
                
                break;

            }
        }

    }
   
    return (EVENT_NONE);
}

/**
 *  @brief      StepperMotorCCWStepBasedDo: do func for the Ready state of the StepperMotorManager state machine
 *
 *  @param event EVENT_TYPE
 */
static EVENT_TYPE  StepperMotorCCWStepBasedDo (EVENT_TYPE event)
{
    
    StepperMotorManager__PostEvent(EV_STEPPERMOTOR_RUN);
    return (EVENT_NONE);
}

/**
 *  @brief      Action function: entry into the Error state of the StepperMotorManager state machine
 *
 */
static EVENT_TYPE  StepperMotorCWStepBasedEntry (void)
{

    // ESP_LOGI(TAG, "CW Entry" );

    if(OPERATE_A4988_STEPPERMOTOR_TEMP.Operation == OPERATION__STEPPER_CW_STEPBASED)
    {
        for(int index = 0; index < MAX_A4988STEPPERMOTOR_COUNT; index++)
        {
            if((OPERATE_A4988_STEPPERMOTOR[index].Hardware_Id == 0xFF) ) // 0xFF is a null id, which can be used for operation
            {
                OPERATE_A4988_STEPPERMOTOR[index].Hardware_Id = OPERATE_A4988_STEPPERMOTOR_TEMP.Hardware_Id;
                OPERATE_A4988_STEPPERMOTOR[index].Operation = OPERATE_A4988_STEPPERMOTOR_TEMP.Operation;
                OPERATE_A4988_STEPPERMOTOR[index].MotorSteps = OPERATE_A4988_STEPPERMOTOR_TEMP.MotorSteps;
                OPERATE_A4988_STEPPERMOTOR[index].MotorSpeed = OPERATE_A4988_STEPPERMOTOR_TEMP.MotorSpeed;
                OPERATE_A4988_STEPPERMOTOR[index].Stop = OPERATE_A4988_STEPPERMOTOR_TEMP.Stop;
                

                for(int timer_num = MS__TIMER_A4988MOTOR_1; timer_num < MS__TIMER_A4988MOTOR_MAX; timer_num ++)
                {
                    if(Timers__MsGetStatus(timer_num) == TIMERS_IDLE)
                    {
                        OPERATE_A4988_STEPPERMOTOR[index].Timer_num = timer_num;
                        Timers__MsSet(OPERATE_A4988_STEPPERMOTOR[index].Timer_num, OPERATE_A4988_STEPPERMOTOR[index].MotorSpeed);//Load the timer with initial T_ON value
                        break;
                    }

                }
                break;
            }
            else if((OPERATE_A4988_STEPPERMOTOR[index].Hardware_Id == OPERATE_A4988_STEPPERMOTOR_TEMP.Hardware_Id))
            {

                OPERATE_A4988_STEPPERMOTOR[index].Hardware_Id = OPERATE_A4988_STEPPERMOTOR_TEMP.Hardware_Id;
                OPERATE_A4988_STEPPERMOTOR[index].Operation = OPERATE_A4988_STEPPERMOTOR_TEMP.Operation;
                OPERATE_A4988_STEPPERMOTOR[index].MotorSteps = OPERATE_A4988_STEPPERMOTOR_TEMP.MotorSteps;
                OPERATE_A4988_STEPPERMOTOR[index].MotorSpeed = OPERATE_A4988_STEPPERMOTOR_TEMP.MotorSpeed;
                OPERATE_A4988_STEPPERMOTOR[index].Stop = OPERATE_A4988_STEPPERMOTOR_TEMP.Stop;

                if(OPERATE_A4988_STEPPERMOTOR[index].Timer_num == MS__TIMER_NULL)
                {
                    for(int timer_num = MS__TIMER_A4988MOTOR_1; timer_num < MS__TIMER_A4988MOTOR_MAX; timer_num ++)
                    {
                        if(Timers__MsGetStatus(timer_num) == TIMERS_IDLE)
                        {
                            OPERATE_A4988_STEPPERMOTOR[index].Timer_num = timer_num;
                            Timers__MsSet(OPERATE_A4988_STEPPERMOTOR[index].Timer_num, OPERATE_A4988_STEPPERMOTOR[index].MotorSpeed);//Load the timer with initial T_ON value
                            break;
                        }

                    }

                }
                
                break;

            }
        }

    }


   
    return (EVENT_NONE);
}

/**
 *  @brief      Action function: do func for the Error state of the StepperMotorManager state machine
 *
 *  @param event EVENT_TYPE
 */
static EVENT_TYPE  StepperMotorCWStepBasedDo (EVENT_TYPE event)
{
    // ESP_LOGI(TAG, "CW Do" );
    StepperMotorManager__PostEvent(EV_STEPPERMOTOR_RUN);
    return (EVENT_NONE);
}

/**
 *  @brief      StepperMotorDiagnosticEntry: entry into the Diagnostic state of the StepperMotorManager state machine
 *
 */
static EVENT_TYPE  StepperMotorDiagnosticEntry (void)
{
  
    return (EVENT_NONE);
}

/**
 *  @brief      StepperMotorDiagnosticDo: do func for the Diagnostic state of the StepperMotorManager state machine
 *
 *  @param event EVENT_TYPE
 */
static EVENT_TYPE  StepperMotorDiagnosticDo (EVENT_TYPE event)
{
    
    return (EVENT_NONE);
}

/**
 *  @brief      StepperMotorBleRunningEntry: entry into the Running state of the StepperMotorManager state machine
 *
 */
static EVENT_TYPE  StepperMotorBleRunningEntry (void)
{
    // ESP_LOGI(TAG, "StepperMotor RUNNING ENTRY");
    return (EVENT_NONE);
}

/**
 *  @brief      StepperMotorBleRunningDo: do func for the Running state of the StepperMotorManager state machine
 *
 *  @param event EVENT_TYPE
 */
static EVENT_TYPE  StepperMotorBleRunningDo (EVENT_TYPE event)
{

    // ESP_LOGI(TAG,"Stepper Motor Running Do");
    StepperMotorManagerInitStatus = true;

    static uint32_t step_counts;

    for (uint8_t index = 0; index < MAX_A4988STEPPERMOTOR_COUNT; index ++ )
    {

        bool bit = OPERATE_A4988_STEPPERMOTOR[index].ToogleBit;

        switch(OPERATE_A4988_STEPPERMOTOR[index].Operation) // Check what operatiuon needs to be performed on LED
        {
            case OPERATION__STEPPER_CW_STEPBASED : 
                A4988__SetDir(STEPPER_CLOCKWISE);

                // ESP_LOGI(TAG, "FLASH OPEARTION DETECTED");
                
                if(Timers__MsGetStatus(OPERATE_A4988_STEPPERMOTOR[index].Timer_num) == TIMERS_COMPLETED)
                {
                    bit = !bit;
                    OPERATE_A4988_STEPPERMOTOR[index].ToogleBit = bit; // save the bit
                    A4988__StepPulse(bit);
                    if(bit)
                    {
                        // ESP_LOGI(TAG,"bit is %d", bit);
                        Timers__MsSet(OPERATE_A4988_STEPPERMOTOR[index].Timer_num , OPERATE_A4988_STEPPERMOTOR[index].MotorSpeed);
                    }
                    else
                    {
                        // ESP_LOGI(TAG,"bit is %d", bit);
                        Timers__MsSet(OPERATE_A4988_STEPPERMOTOR[index].Timer_num , OPERATE_A4988_STEPPERMOTOR[index].MotorSpeed);
                    }
                }

                step_counts ++;

                if(step_counts >= OPERATE_A4988_STEPPERMOTOR[index].MotorSteps)
                {
                    step_counts = 0;
                    Timers__MsDispose(OPERATE_A4988_STEPPERMOTOR[index].Timer_num );//if timer is assigned from previous task, then dispose the timer and make it availble for others
                    OPERATE_A4988_STEPPERMOTOR[index].Timer_num = MS__TIMER_NULL;

                    OPERATE_A4988_STEPPERMOTOR[index].Operation = OPERATION__STEPPER_NULL;
                }


                break;


            case OPERATION__STEPPER_CCW_STEPBASED : 

                A4988__SetDir(STEPPER_COUNTERCLOCKWISE);

                // ESP_LOGI(TAG, "FLASH OPEARTION DETECTED");
                
                if(Timers__MsGetStatus(OPERATE_A4988_STEPPERMOTOR[index].Timer_num) == TIMERS_COMPLETED)
                {
                    bit = !bit;
                    OPERATE_A4988_STEPPERMOTOR[index].ToogleBit = bit; // save the bit
                    A4988__StepPulse(bit);
                    if(bit)
                    {
                        // ESP_LOGI(TAG,"bit is %d", bit);
                        Timers__MsSet(OPERATE_A4988_STEPPERMOTOR[index].Timer_num , OPERATE_A4988_STEPPERMOTOR[index].MotorSpeed);
                    }
                    else
                    {
                        // ESP_LOGI(TAG,"bit is %d", bit);
                        Timers__MsSet(OPERATE_A4988_STEPPERMOTOR[index].Timer_num , OPERATE_A4988_STEPPERMOTOR[index].MotorSpeed);
                    }
                }

                step_counts ++;

                if(step_counts >= OPERATE_A4988_STEPPERMOTOR[index].MotorSteps)
                {
                    step_counts = 0;
                    Timers__MsDispose(OPERATE_A4988_STEPPERMOTOR[index].Timer_num );//if timer is assigned from previous task, then dispose the timer and make it availble for others
                    OPERATE_A4988_STEPPERMOTOR[index].Timer_num = MS__TIMER_NULL;

                    OPERATE_A4988_STEPPERMOTOR[index].Operation = OPERATION__STEPPER_NULL;
                }
                break;

            

            case OPERATION__STEPPER_STOP:
                if(OPERATE_A4988_STEPPERMOTOR[index].Stop == true)
                {
                    step_counts = 0;
                    Timers__MsDispose(OPERATE_A4988_STEPPERMOTOR[index].Timer_num );//if timer is assigned from previous task, then dispose the timer and make it availble for others
                    OPERATE_A4988_STEPPERMOTOR[index].Timer_num = MS__TIMER_NULL;
                    
                }
                
                break;

        
            default:
                break;

        }
       
    

    }

    return (EVENT_NONE);
}

/**
 *  @brief      StepperMotorStopEntry: entry into the Stopping state of the StepperMotorManager state machine
 *
 */
static EVENT_TYPE  StepperMotorStopEntry (void)
{
  

    return (EVENT_NONE);
}

/**
 *  @brief      StepperMotorStopDo: do func for the Stopping state of the StepperMotorManager state machine
 *
 *  @param event EVENT_TYPE
 */
static EVENT_TYPE  StepperMotorStopDo (EVENT_TYPE event)
{
    
    StepperMotorManager__PostEvent(EV_STEPPERMOTOR_RUN);
    return (EVENT_NONE);
}

// ==================================================== EXTERNAL INTERFACE FUNCTIONS ==========================================
/**
 *  @brief      posts events to the SysManager state machine
 *
 *  @param event STEPPERMOTOR_MANAGER_EVENT_TYPE
 */
void StepperMotorManager__ExternalProcess(uint8_t hw_id, A4988_STEPPERMOTOR_OPEARTION_TYPE opr_type, uint32_t steps, uint32_t speed)
{
    
    OPERATE_A4988_STEPPERMOTOR_TEMP.Hardware_Id = hw_id;
    OPERATE_A4988_STEPPERMOTOR_TEMP.Operation = opr_type;
    OPERATE_A4988_STEPPERMOTOR_TEMP.MotorSteps = steps;
    OPERATE_A4988_STEPPERMOTOR_TEMP.MotorSpeed = speed;

    if(opr_type == OPERATION__STEPPER_STOP)
    {
        OPERATE_A4988_STEPPERMOTOR_TEMP.Stop = true;
    }
    else{
        OPERATE_A4988_STEPPERMOTOR_TEMP.Stop = false;
    }
    

    StepperMotorManager__PostEvent(opr_type);
    // ESP_LOGI(TAG,"Posted Event = %d", opr_type);
}


bool StepperMotorManager__initStatus(void)
{
    return StepperMotorManagerInitStatus;
}