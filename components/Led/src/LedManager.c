/**
 *  @file      
 *
 *  @brief      System Manager Module - the main Master application module
 *
 *  @details    LedManager is the main State Machine and controls the motor
 *
 *  @section    Applicable_Documents
 *                  List here all the applicable documents if needed. <tr>  
 *
 *  $Header: LedManager.c 1.6 2014/12/05 23:59:09IST Chetan S
 *
 *  @copyright  Copyright 2014-$Date: 2014/12/05 23:59:09IST $. Electrosal. All rights reserved - CONFIDENTIAL
 */
//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------
 
 
//-------------------------------------- Include Files ----------------------------------------------------------------
// #include "../../C_Extensions.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "LedManager.h"
#include "LedEngine.h"
// #include "LedWS2812.h"

 
//-------------------------------------- PUBLIC (Variables) -----------------------------------------------------------

//-------------------------------------- PRIVATE (Variables, Constants & Defines) -------------------------------------

static const char *TAG = "LedManager"; 
bool LedManagerInitStatus = false;

static struct LED_STRUCT_TYPE OPERATE_LED[MAX_LED_COUNT];
static struct LED_STRUCT_TYPE OPERATE_LED_TEMP;
 
static EVENT_TYPE  LedInitEntry (void);
static EVENT_TYPE  LedInitDo (EVENT_TYPE event);
 
static EVENT_TYPE  LedTurnOnEntry (void);
static EVENT_TYPE  LedTurnOnDo (EVENT_TYPE event);
 
static EVENT_TYPE  LedFlashEntry(void);
static EVENT_TYPE  LedFlashDo (EVENT_TYPE event);
 
static EVENT_TYPE  LedTurnOffEntry (void);
static EVENT_TYPE  LedTurnOffDo (EVENT_TYPE event);
 
static EVENT_TYPE  LedFadeInEntry (void);
static EVENT_TYPE  LedFadeInDo (EVENT_TYPE event);
 
static EVENT_TYPE  LedBleRunningEntry (void);
static EVENT_TYPE  LedBleRunningDo (EVENT_TYPE event);
 
static EVENT_TYPE  LedIdleEntry (void);
static EVENT_TYPE  LedIdleDo (EVENT_TYPE event);
 
//=====================================================================================================================
//=== LedEngineCrank Auto-Generated Code for the System Manager State Machine =======================================
//=====================================================================================================================
 
#if (SE_EVENT_QUEUE == ENABLED)
//! Maximum number of events that can be queued up in the System Manager state machine.
//! Implementation always executes as soon as the event is posted, so there is no reason for more than one.
#define LED_MANAGER_QUEUE_DEPTH 1
#endif      // #if (SE_QUEUE_ENABLED == ENABLED)
 
 
 
 
 
//! Data structure used by the LedEngine module to run the System Manager state machine.
static STATE_CONTROL_TYPE Led_Manager;
 
#if (SE_EVENT_QUEUE == ENABLED)
//! Allocate memory for the System Manager state machine's event queue.
static EVENT_TYPE Led_Manager_Queue[LED_MANAGER_QUEUE_DEPTH];
#endif      // #if (SE_EVENT_QUEUE == ENABLED)
 
//! List of transitions for Initialize State of the System Manager state machine.
static STATE_TRANSITION_TYPE TRANSITIONS_FOR_STATE_LED_INITIALIZE[] =
{
//  {Event Name,         Guard Function,  Transition Action,    Next State},
    {EV_LEDINIT_COMPLETE,   GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_LED_RUNNING},
    END_OF_TRANSITION_TABLE
};
 
//! List of transitions for Stand By State of the System Manager state machine.
static STATE_TRANSITION_TYPE TRANSITIONS_FOR_STATE_LED_TURNON[] =
{
//  {Event Name,          Guard Function,  Transition Action,    Next State},
    {EV_LED_OFF,            GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_LED_TURNOFF},
    {EV_LED_FLASH,          GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_LED_FLASH},
    {EV_LED_FADEIN,         GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_LED_FADEIN},
    {EV_LED_FADEOUT,        GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_LED_FADOUT},
    {EV_LED_RUN,            GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_LED_RUNNING},
    END_OF_TRANSITION_TABLE
};
 
//! List of transitions for Error State of the System Manager state machine.
static STATE_TRANSITION_TYPE TRANSITIONS_FOR_STATE_LED_TURNOFF[] =
{
//  {Event Name,          Guard Function,  Transition Action,    Next State},
    {EV_LED_ON,             GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_LED_TURNON},
    {EV_LED_FLASH,          GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_LED_FLASH},
    {EV_LED_FADEIN,         GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_LED_FADEIN},
    {EV_LED_FADEOUT,        GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_LED_FADOUT},
    {EV_LED_RUN,            GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_LED_RUNNING},
    END_OF_TRANSITION_TABLE
};
 
//! List of transitions for Ready State of the System Manager state machine.
static STATE_TRANSITION_TYPE TRANSITIONS_FOR_STATE_LED_FLASH[] =
{
//  {Event Name,          Guard Function,  Transition Action,    Next State},
    {EV_LED_OFF,            GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_LED_TURNOFF},
    {EV_LED_ON,             GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_LED_TURNON},
    {EV_LED_RUN,            GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_LED_RUNNING},
    {EV_LED_FADEIN,         GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_LED_FADEIN},
    {EV_LED_FADEOUT,        GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_LED_FADOUT},
    END_OF_TRANSITION_TABLE
};
 
//! List of transitions for Running State of the System Manager state machine.
static STATE_TRANSITION_TYPE TRANSITIONS_FOR_STATE_LED_RUNNING[] =
{
//  {Event Name,          Guard Function,  Transition Action,    Next State},
    {EV_LED_OFF,            GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_LED_TURNOFF},
    {EV_LED_ON,             GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_LED_TURNON},
    {EV_LED_FLASH,          GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_LED_FLASH},
    {EV_LED_FADEIN,         GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_LED_FADEIN},
    {EV_LED_FADEOUT,        GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_LED_FADOUT},
    END_OF_TRANSITION_TABLE
};
 
//! List of transitions for Diagnostic State of the System Manager state machine.
static STATE_TRANSITION_TYPE TRANSITIONS_FOR_STATE_LED_FADEIN[] =
{
//  {Event Name,          Guard Function,  Transition Action,    Next State},
    {EV_LED_OFF,            GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_LED_TURNOFF},

    END_OF_TRANSITION_TABLE
};
 
//! List of transitions for Stopping State of the System Manager state machine.
static STATE_TRANSITION_TYPE TRANSITIONS_FOR_STATE_LED_IDLE[] =
{
//  {Event Name,         Guard Function,  Transition Action,    Next State},
    {EV_LED_IDLE,     GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_LED_TURNON},
    END_OF_TRANSITION_TABLE
};
 
//! State table for the System Manager state machine.
static STATE_TABLE_TYPE LED_MANAGER_STATE_TABLE[] =
{
//  {State Name,       {Enter Action,    Do Action,            Exit Action},    Transition Table},
    {STATE_LED_INITIALIZE,  {LedInitEntry,      LedInitDo,            EXIT_FUNC_NONE}, TRANSITIONS_FOR_STATE_LED_INITIALIZE},
    {STATE_LED_TURNON,      {LedTurnOnEntry,    LedTurnOnDo,          EXIT_FUNC_NONE}, TRANSITIONS_FOR_STATE_LED_TURNON},
    {STATE_LED_TURNOFF,     {LedTurnOffEntry,   LedTurnOffDo,         EXIT_FUNC_NONE}, TRANSITIONS_FOR_STATE_LED_TURNOFF},
    {STATE_LED_FLASH,       {LedFlashEntry,     LedFlashDo,           EXIT_FUNC_NONE}, TRANSITIONS_FOR_STATE_LED_FLASH},
    {STATE_LED_RUNNING,     {LedBleRunningEntry,   LedBleRunningDo,      EXIT_FUNC_NONE}, TRANSITIONS_FOR_STATE_LED_RUNNING},
    {STATE_LED_FADEIN,      {LedFadeInEntry,    LedFadeInDo,           EXIT_FUNC_NONE}, TRANSITIONS_FOR_STATE_LED_FADEIN},
    {STATE_LED_IDLE,        {LedIdleEntry,      LedIdleDo,             EXIT_FUNC_NONE}, TRANSITIONS_FOR_STATE_LED_IDLE}
};
 
//! State machine definition structure for the System Manager state machine.
static STATE_MACHINE_DEFINITION_TYPE LED_MANAGER_MACHINE =
{
    STATE_LED_INITIALIZE,                               // Initial state
    LED_MANAGER_STATE_TABLE,                     // Table defining state behaviors
    &Led_Manager,                                // Control structure
#if (SE_EVENT_QUEUE == ENABLED)
    Led_Manager_Queue,                           // Pointer to the event queue buffer
    LED_MANAGER_QUEUE_DEPTH,                     // Number of events that the event queue can hold
#endif      // #if (SE_EVENT_QUEUE == ENABLED)
};
 
// End of LedEngineCrank Auto-Generated Code.
 
//=====================================================================================================================
//-------------------------------------- Public Functions -------------------------------------------------------------
//=====================================================================================================================
 
//---------------------------------------------------------------------------------------------------------------------
/**
 *  @brief      It Initializes the module LedManager and its variables
 *
 */
void LedManager__Initialize(void)
{
    // Initialize the LedManager state-machine.
    (void)LedEngine__Configure(&LED_MANAGER_MACHINE);
}
 
/**
 *  @brief      Executes an iteration of the SysMan state machine
 *
 */
void LedManager__Handler(void)
{
    (void)LedEngine__Execute(&Led_Manager);   
 
}
 
/**
 *  @brief      reports the current state of the SysMan state machine
 *
 *  @return SYSMAN_STATE_TYPE
 */
LED_MANAGER_STATE_TYPE LedManager__GetState(void)
{
    return ((LED_MANAGER_STATE_TYPE)(Led_Manager.Cur_State));
}
 
/**
 *  @brief      posts events to the SysMan state machine
 *
 *  @param event LED_MANAGER_EVENT_TYPE
 */
void LedManager__PostEvent(LED_MANAGER_EVENT_TYPE event)
{
    LedEngine__PostEvent(&Led_Manager, (EVENT_TYPE) event);
}


/**
 *  @brief      posts events to the SysMan state machine
 *
 *  @param event LED_MANAGER_EVENT_TYPE
 */
void LedManager__ExternalProcess(uint8_t hw_id, OPEARTION_TYPE opr_type, uint32_t t_on, uint32_t t_off, uint8_t Color_R, uint8_t Color_G , uint8_t Color_B)
{
    OPERATE_LED_TEMP.Hardware_Id = hw_id;
    OPERATE_LED_TEMP.Operation = opr_type;
    OPERATE_LED_TEMP.OnTime = t_on;
    OPERATE_LED_TEMP.OffTime = t_off;
    OPERATE_LED_TEMP.Color_R = Color_R;
    OPERATE_LED_TEMP.Color_G = Color_G;
    OPERATE_LED_TEMP.Color_B = Color_B;

    // LedManager__PostEvent(EV_LED_FLASH);
    LedManager__PostEvent(opr_type);
    ESP_LOGI(TAG,"Posted Event = %d", opr_type);
}
//=====================================================================================================================
//-------------------------------------- Private Functions ------------------------------------------------------------
//=====================================================================================================================
/**
 *  @brief     LedInitEntry: entry into the Initialize state of the LedManager state machine
 *
 */
static EVENT_TYPE  LedInitEntry (void)
{
    // LEDMANAGER_INITIALIZE_ENTRY();
    //Initialize the LED structure 
    for(int index=0; index < MAX_LED_COUNT;  index++)
    {
        OPERATE_LED[index].Hardware_Id = 0xFF;  // 0xFF is a null or Empty Hw_Id and can be used for operation
        OPERATE_LED[index].Operation = OPERATION__NULL;
        OPERATE_LED[index].OnTime = 0x00;
        OPERATE_LED[index].OffTime = 0x00;
        OPERATE_LED[index].PendingReq = false; // Clear the past pending request on initialize

    }

    OPERATE_LED_TEMP.Hardware_Id = 0xFF;  // 0xFF is a null or Empty Hw_Id and can be used for operation
    OPERATE_LED_TEMP.Operation = OPERATION__NULL;
    OPERATE_LED_TEMP.OnTime = 0x00;
    OPERATE_LED_TEMP.OffTime = 0x00;
    OPERATE_LED_TEMP.PendingReq = false; // Clear the past pending request on initialize

    return (EVENT_NONE);
}
 
/**
 *  @brief      LedInitDo: do func for the Initialize state of the LedManager state machine
 *
 *  @param event EVENT_TYPE
 */
static EVENT_TYPE  LedInitDo (EVENT_TYPE event)
{
    // LEDMANAGER_INITIALIZE_DO(event);
    
    LedManager__PostEvent(EV_LEDINIT_COMPLETE);
    
    return (EVENT_NONE);
}
 
/**
 *  @brief      LedTurnOnEntry: entry into the Standby state of the LedManager state machine
 *
 */
static EVENT_TYPE  LedTurnOnEntry (void)
{
    if(OPERATE_LED_TEMP.Operation == OPERATION__LED_ON)
    {
        for(int index = 0; index < MAX_LED_COUNT; index++)
        {
            if((OPERATE_LED[index].Hardware_Id == 0xFF) ) // 0xFF is a null id, which can be used for operation
            {
                OPERATE_LED[index].Hardware_Id = OPERATE_LED_TEMP.Hardware_Id;
                OPERATE_LED[index].Operation = OPERATE_LED_TEMP.Operation;
                OPERATE_LED[index].Color_R = OPERATE_LED_TEMP.Color_R;
                OPERATE_LED[index].Color_G = OPERATE_LED_TEMP.Color_G;
                OPERATE_LED[index].Color_B = OPERATE_LED_TEMP.Color_B;
                OPERATE_LED[index].Timer_num = MS__TIMER_NULL;

                break;
            }
            else if((OPERATE_LED[index].Hardware_Id == OPERATE_LED_TEMP.Hardware_Id))
            {
                OPERATE_LED[index].Hardware_Id = OPERATE_LED_TEMP.Hardware_Id;
                OPERATE_LED[index].OnTime = OPERATE_LED_TEMP.OnTime;
                OPERATE_LED[index].OffTime = OPERATE_LED_TEMP.OffTime;
                OPERATE_LED[index].Operation = OPERATE_LED_TEMP.Operation;
                OPERATE_LED[index].Color_R = OPERATE_LED_TEMP.Color_R;
                OPERATE_LED[index].Color_G = OPERATE_LED_TEMP.Color_G;
                OPERATE_LED[index].Color_B = OPERATE_LED_TEMP.Color_B;

                Timers__MsDispose(OPERATE_LED[index].Timer_num );//if timer is assigned from previous task, then dispose the timer and make it availble for others
                OPERATE_LED[index].Timer_num = MS__TIMER_NULL;
                break;

            }
        }

    }
    return (EVENT_NONE);
}
 
/**
 *  @brief      LedTurnOnDo: do func for the Standby state of the LedManager state machine
 *
 *  @param event EVENT_TYPE
 */
static EVENT_TYPE  LedTurnOnDo (EVENT_TYPE event)
{
    LedManager__PostEvent(EV_LED_RUN);
    return (EVENT_NONE);
}
 
/**
 *  @brief      LedFlashEntry: entry into the Ready state of the LedManager state machine
 *
 */
static EVENT_TYPE  LedFlashEntry (void)
{
    // LEDMANAGER_READY_ENTRY();
    ESP_LOGI(TAG,"Led Flash Entry");


    if(OPERATE_LED_TEMP.Operation == OPERATION__LED_FLASH)
    {
        for(int index = 0; index < MAX_LED_COUNT; index++)
        {
            if((OPERATE_LED[index].Hardware_Id == 0xFF) ) // 0xFF is a null id, which can be used for operation
            {
                OPERATE_LED[index].Hardware_Id = OPERATE_LED_TEMP.Hardware_Id;
                OPERATE_LED[index].Operation = OPERATE_LED_TEMP.Operation;
                OPERATE_LED[index].Color_R = OPERATE_LED_TEMP.Color_R;
                OPERATE_LED[index].Color_G = OPERATE_LED_TEMP.Color_G;
                OPERATE_LED[index].Color_B = OPERATE_LED_TEMP.Color_B;


                for(int timer_num = MS__TIMER_LED_1; timer_num < MS__TIMER_LED_MAX; timer_num ++)
                {
                    if(Timers__MsGetStatus(timer_num) == TIMERS_IDLE)
                    {
                        OPERATE_LED[index].Timer_num = timer_num;
                        OPERATE_LED[index].OnTime = OPERATE_LED_TEMP.OnTime;
                        OPERATE_LED[index].OffTime = OPERATE_LED_TEMP.OffTime;
                        Timers__MsSet(OPERATE_LED[index].Timer_num, OPERATE_LED[index].OnTime);//Load the timer with initial T_ON value
                        break;
                    }

                }
                break;
            }
            else if((OPERATE_LED[index].Hardware_Id == OPERATE_LED_TEMP.Hardware_Id))
            {

                OPERATE_LED[index].Hardware_Id = OPERATE_LED_TEMP.Hardware_Id;
                OPERATE_LED[index].Operation = OPERATE_LED_TEMP.Operation;
                OPERATE_LED[index].Color_R = OPERATE_LED_TEMP.Color_R;
                OPERATE_LED[index].Color_G = OPERATE_LED_TEMP.Color_G;
                OPERATE_LED[index].Color_B = OPERATE_LED_TEMP.Color_B;


                if(OPERATE_LED[index].Timer_num == MS__TIMER_NULL)
                {
                    for(int timer_num = MS__TIMER_LED_1; timer_num < MS__TIMER_LED_MAX; timer_num ++)
                    {
                        if(Timers__MsGetStatus(timer_num) == TIMERS_IDLE)
                        {
                            OPERATE_LED[index].Timer_num = timer_num;
                            OPERATE_LED[index].OnTime = OPERATE_LED_TEMP.OnTime;
                            OPERATE_LED[index].OffTime = OPERATE_LED_TEMP.OffTime;
                            Timers__MsSet(OPERATE_LED[index].Timer_num, OPERATE_LED[index].OnTime);//Load the timer with initial T_ON value
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
 *  @brief      LedFlashDo: do func for the Ready state of the LedManager state machine
 *
 *  @param event EVENT_TYPE
 */
static EVENT_TYPE  LedFlashDo (EVENT_TYPE event)
{
    // LEDMANAGER_READY_DO(event);

    ESP_LOGI(TAG,"Led Flash Do");
    LedManager__PostEvent(EV_LED_RUN);
 
    return (EVENT_NONE);
}
 
/**
 *  @brief      Action function: entry into the Error state of the LedManager state machine
 *
 */
static EVENT_TYPE  LedTurnOffEntry (void)
{
    if(OPERATE_LED_TEMP.Operation == OPERATION__LED_OFF)
    {
        for(int index = 0; index < MAX_LED_COUNT; index++)
        {
            if((OPERATE_LED[index].Hardware_Id == 0xFF) ) // 0xFF is a null id, which can be used for operation
            {
                OPERATE_LED[index].Hardware_Id = OPERATE_LED_TEMP.Hardware_Id;
                OPERATE_LED[index].Operation = OPERATE_LED_TEMP.Operation;
                OPERATE_LED[index].Color_R = 0x00;
                OPERATE_LED[index].Color_G = 0x00;
                OPERATE_LED[index].Color_B = 0x00;
                OPERATE_LED[index].Timer_num = MS__TIMER_NULL;

                break;
            }
            else if((OPERATE_LED[index].Hardware_Id == OPERATE_LED_TEMP.Hardware_Id))
            {
                OPERATE_LED[index].Hardware_Id = OPERATE_LED_TEMP.Hardware_Id;
                OPERATE_LED[index].OnTime = OPERATE_LED_TEMP.OnTime;
                OPERATE_LED[index].OffTime = OPERATE_LED_TEMP.OffTime;
                OPERATE_LED[index].Operation = OPERATE_LED_TEMP.Operation;
                OPERATE_LED[index].Color_R = 0x00;
                OPERATE_LED[index].Color_G = 0x00;
                OPERATE_LED[index].Color_B = 0x00;

                Timers__MsDispose(OPERATE_LED[index].Timer_num );//if timer is assigned from previous task, then dispose the timer and make it availble for others
                OPERATE_LED[index].Timer_num = MS__TIMER_NULL;
                break;

            }
        }

    }
    return (EVENT_NONE);
}
 
/**
 *  @brief      Action function: do func for the Error state of the LedManager state machine
 *
 *  @param event EVENT_TYPE
 */
static EVENT_TYPE  LedTurnOffDo (EVENT_TYPE event)
{
    LedManager__PostEvent(EV_LED_RUN);
    return (EVENT_NONE);
}
 
/**
 *  @brief      LedFadeInEntry: entry into the Diagnostic state of the LedManager state machine
 *
 */
static EVENT_TYPE  LedFadeInEntry (void)
{
    // LEDMANAGER_DIAGNOSTIC_ENTRY();
    return (EVENT_NONE);
}
 
/**
 *  @brief      LedFadeInDo: do func for the Diagnostic state of the LedManager state machine
 *
 *  @param event EVENT_TYPE
 */
static EVENT_TYPE  LedFadeInDo (EVENT_TYPE event)
{
    // LEDMANAGER_DIAGNOSTIC_DO(event);
    return (EVENT_NONE);
}
 
/**
 *  @brief      LedBleRunningEntry: entry into the Running state of the LedManager state machine
 *
 */
static EVENT_TYPE  LedBleRunningEntry (void)
{
    // LEDMANAGER_RUNNING_ENTRY();
    ESP_LOGI(TAG, "LED RUNNING ENTRY");
    return (EVENT_NONE);
}
 
/**
 *  @brief      LedBleRunningDo: do func for the Running state of the LedManager state machine
 *
 *  @param event EVENT_TYPE
 */
static EVENT_TYPE  LedBleRunningDo (EVENT_TYPE event)
{

    // ESP_LOGI(TAG, "LED RUNNING DO");
    LedManagerInitStatus = true;
 
    for (uint8_t index = 0; index < MAX_LED_COUNT; index ++ )
    {

        bool bit = OPERATE_LED[index].ToogleBit;
        switch(OPERATE_LED[index].Operation) // Check what operatiuon needs to be performed on LED
        {
            
            case OPERATION__LED_FLASH : 

                // ESP_LOGI(TAG, "FLASH OPEARTION DETECTED");
                
                if(Timers__MsGetStatus(OPERATE_LED[index].Timer_num) == TIMERS_COMPLETED)
                {
                    bit = !bit;
                    OPERATE_LED[index].ToogleBit = bit; // save the bit
                    LedWS2812__SetPixel(OPERATE_LED[index].Hardware_Id, bit, OPERATE_LED[index].Color_R, OPERATE_LED[index].Color_G, OPERATE_LED[index].Color_B);
                    if(bit)
                    {
                        // ESP_LOGI(TAG,"bit is %d", bit);
                        Timers__MsSet(OPERATE_LED[index].Timer_num , OPERATE_LED[index].OnTime);
                    }
                    else
                    {
                        // ESP_LOGI(TAG,"bit is %d", bit);
                        Timers__MsSet(OPERATE_LED[index].Timer_num , OPERATE_LED[index].OffTime);
                    }
                }
                break;

            case OPERATION__LED_ON : 
                LedWS2812__SetPixel(OPERATE_LED[index].Hardware_Id, true, OPERATE_LED[index].Color_R, OPERATE_LED[index].Color_G, OPERATE_LED[index].Color_B);
                break;
            case OPERATION__LED_OFF : 
                LedWS2812__SetPixel(OPERATE_LED[index].Hardware_Id, false, OPERATE_LED[index].Color_R, OPERATE_LED[index].Color_G, OPERATE_LED[index].Color_B);
                break;
            case OPERATION__LED_FADEIN : 
                break;
            case OPERATION__LED_FADEOUT : 
                break;
            default:
                break;

        }
       
    

    }

    



    return (EVENT_NONE);
}
 
/**
 *  @brief      LedIdleEntry: entry into the Stopping state of the LedManager state machine
 *
 */
static EVENT_TYPE  LedIdleEntry (void)
{
    // LEDMANAGER_STOPPING_ENTRY();
 
    return (EVENT_NONE);
}
 
/**
 *  @brief      LedIdleDo: do func for the Stopping state of the LedManager state machine
 *
 *  @param event EVENT_TYPE
 */
static EVENT_TYPE  LedIdleDo (EVENT_TYPE event)
{
    // LEDMANAGER_STOPPING_DO(event);
 
    return (EVENT_NONE);
}

bool LedManager__initStatus(void)
{
    return LedManagerInitStatus;
}
 
 

