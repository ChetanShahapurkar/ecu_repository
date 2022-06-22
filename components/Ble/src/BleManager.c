/**
 *  @file       
 *
 *  @brief      System Manager Module - the main Master application module
 *
 *  @details    BleManager is the main State Machine and controls the motor
 *
 *  @section    Applicable_Documents
 *					List here all the applicable documents if needed. <tr>	
 *
 *  $Header: BleManager.c 1.6 2014/12/05 23:59:09IST Chetan S
 *
 *  @copyright  Copyright 2014-$Date: 2014/12/05 23:59:09IST $. Electrosal. All rights reserved - CONFIDENTIAL
 */
//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------


//-------------------------------------- Include Files ----------------------------------------------------------------
#include "BleManager.h" 
#include "BleEngine.h"
#include "esp_log.h"
#include "EspBle.h"
#include "esp_log.h"

//-------------------------------------- PUBLIC (Variables) -----------------------------------------------------------
char * newmsg;
//-------------------------------------- PRIVATE (Variables, Constants & Defines) -------------------------------------

static const char *TAG = "BleManager"; 
static EVENT_TYPE  BleInitializeEntry (void);
static EVENT_TYPE  BleInitializeDo (EVENT_TYPE event);

static EVENT_TYPE  BleStandbyEntry (void);
static EVENT_TYPE  BleStandbyDo (EVENT_TYPE event);

static EVENT_TYPE  BleGetMsgEntry(void);
static EVENT_TYPE  BleGetMsgDo (EVENT_TYPE event);

static EVENT_TYPE  BlePublishMsgEntry (void);
static EVENT_TYPE  BlePublishMsgDo (EVENT_TYPE event);

static EVENT_TYPE  BleDiagnosticEntry (void);
static EVENT_TYPE  BleDiagnosticDo (EVENT_TYPE event);

static EVENT_TYPE  BleNewMsgReceivedEntry (void);
static EVENT_TYPE  BleNewMsgReceivedDo (EVENT_TYPE event);

static EVENT_TYPE  BleRunningEntry (void);
static EVENT_TYPE  BleRunningDo (EVENT_TYPE event);

static EVENT_TYPE  BleStopEntry (void);
static EVENT_TYPE  BleStopDo (EVENT_TYPE event);

//=====================================================================================================================
//=== BleEngineCrank Auto-Generated Code for the System Manager State Machine =======================================
//=====================================================================================================================

#if (SE_EVENT_QUEUE == ENABLED)
//! Maximum number of events that can be queued up in the System Manager state machine.
//! Implementation always executes as soon as the event is posted, so there is no reason for more than one.
#define BLE_MANAGER_QUEUE_DEPTH 1
#endif      // #if (SE_QUEUE_ENABLED == ENABLED)





//! Data structure used by the BleEngine module to run the System Manager state machine.
static STATE_CONTROL_TYPE BLE_MANAGER;

#if (SE_EVENT_QUEUE == ENABLED)
//! Allocate memory for the System Manager state machine's event queue.
static EVENT_TYPE BLE_MANAGER_Queue[BLE_MANAGER_QUEUE_DEPTH];
#endif      // #if (SE_EVENT_QUEUE == ENABLED)

//! List of transitions for Initialize State of the System Manager state machine.
static STATE_TRANSITION_TYPE TRANSITIONS_FOR_STATE_BLE_INITIALIZE[] =
{
//  {Event Name,         Guard Function,  Transition Action,    Next State},
    {EV_BLE_INIT_COMPLETE,   GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_BLE_RUNNING},
    {EV_BLE_INT_ERROR,      GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_BLE_PUBLISH_MSG},
    END_OF_TRANSITION_TABLE
};

//! List of transitions for Stand By State of the System Manager state machine.
static STATE_TRANSITION_TYPE TRANSITIONS_FOR_STATE_BLE_STAND_BY[] =
{
//  {Event Name,          Guard Function,  Transition Action,    Next State},
    {EV_BLE_PUBLISH_MSG,            GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_BLE_PUBLISH_MSG},
    {EV_BLE_GET_MSG,     GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_BLE_GET_MSG},
    END_OF_TRANSITION_TABLE
};

//! List of transitions for Error State of the System Manager state machine.
static STATE_TRANSITION_TYPE TRANSITIONS_FOR_STATE_BLE_PUBLISH_MSG[] =
{
//  {Event Name,          Guard Function,  Transition Action,    Next State},
    {EV_BLE_STOP,             GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_BLE_STOPPING},
    END_OF_TRANSITION_TABLE
};

//! List of transitions for Ready State of the System Manager state machine.
static STATE_TRANSITION_TYPE TRANSITIONS_FOR_STATE_BLE_GET_MSG[] =
{
//  {Event Name,          Guard Function,  Transition Action,    Next State},
    {EV_BLE_PUBLISH_MSG,            GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_BLE_PUBLISH_MSG},
    {EV_BLE_RUN,              GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_BLE_RUNNING},
    {EV_BLE_DIAGNOSTIC,       GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_BLE_DIAGNOSTIC},
    END_OF_TRANSITION_TABLE
};

//! List of transitions for Running State of the System Manager state machine.
static STATE_TRANSITION_TYPE TRANSITIONS_FOR_STATE_BLE_RUNNING[] =
{
//  {Event Name,          Guard Function,  Transition Action,    Next State},
    {EV_BLE_PUBLISH_MSG,            GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_BLE_PUBLISH_MSG},
    {EV_BLE_NEWMSG_RECEIVED,        GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_BLE_NEWMSG_RECEIVED},
    {EV_BLE_STOP,                   GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_BLE_STOPPING},
    END_OF_TRANSITION_TABLE
};

//! List of transitions for Diagnostic State of the System Manager state machine.
static STATE_TRANSITION_TYPE TRANSITIONS_FOR_STATE_BLE_DIAGNOSTIC[] =
{
//  {Event Name,          Guard Function,  Transition Action,    Next State},
    {EV_BLE_PUBLISH_MSG,            GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_BLE_PUBLISH_MSG},
    END_OF_TRANSITION_TABLE
};

//! List of transitions for Diagnostic State of the System Manager state machine.
static STATE_TRANSITION_TYPE TRANSITIONS_FOR_STATE_BLE_NEW_MSG_RECEIVED[] =
{
//  {Event Name,          Guard Function,  Transition Action,    Next State},
    {EV_BLE_PUBLISH_MSG,            GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_BLE_PUBLISH_MSG},
    {EV_BLE_RUN,            GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_BLE_RUNNING},
    END_OF_TRANSITION_TABLE
};

//! List of transitions for Stopping State of the System Manager state machine.
static STATE_TRANSITION_TYPE TRANSITIONS_FOR_STATE_BLE_STOPPING[] =
{
//  {Event Name,         Guard Function,  Transition Action,    Next State},
    {EV_BLE_PUBLISH_MSG,     GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_BLE_PUBLISH_MSG},
    END_OF_TRANSITION_TABLE
};

//! State table for the System Manager state machine.
static STATE_TABLE_TYPE BLE_MANAGER_STATE_TABLE[] =
{
//  {State Name,       {Enter Action,    Do Action,            Exit Action},    Transition Table},
    {STATE_BLE_INITIALIZE,      {BleInitializeEntry,        BleInitializeDo,        EXIT_FUNC_NONE}, TRANSITIONS_FOR_STATE_BLE_INITIALIZE},
    {STATE_BLE_STAND_BY,        {BleStandbyEntry,           BleStandbyDo,           EXIT_FUNC_NONE}, TRANSITIONS_FOR_STATE_BLE_STAND_BY},
    {STATE_BLE_PUBLISH_MSG,     {BlePublishMsgEntry,        BlePublishMsgDo,        EXIT_FUNC_NONE}, TRANSITIONS_FOR_STATE_BLE_PUBLISH_MSG},
    {STATE_BLE_GET_MSG,         {BleGetMsgEntry,            BleGetMsgDo,            EXIT_FUNC_NONE}, TRANSITIONS_FOR_STATE_BLE_GET_MSG},
    {STATE_BLE_RUNNING,         {BleRunningEntry,           BleRunningDo,           EXIT_FUNC_NONE}, TRANSITIONS_FOR_STATE_BLE_RUNNING},
    {STATE_BLE_DIAGNOSTIC,      {BleDiagnosticEntry,        BleDiagnosticDo,        EXIT_FUNC_NONE}, TRANSITIONS_FOR_STATE_BLE_DIAGNOSTIC},
    {STATE_BLE_STOPPING,        {BleStopEntry,              BleStopDo,              EXIT_FUNC_NONE}, TRANSITIONS_FOR_STATE_BLE_STOPPING},
    {STATE_BLE_NEWMSG_RECEIVED, {BleNewMsgReceivedEntry,    BleNewMsgReceivedDo,    EXIT_FUNC_NONE}, TRANSITIONS_FOR_STATE_BLE_NEW_MSG_RECEIVED}
};

//! State machine definition structure for the System Manager state machine.
static STATE_MACHINE_DEFINITION_TYPE BLE_MANAGER_MACHINE =
{
    STATE_BLE_INITIALIZE,                               // Initial state
    BLE_MANAGER_STATE_TABLE,                     // Table defining state behaviors
    &BLE_MANAGER,                                // Control structure
#if (SE_EVENT_QUEUE == ENABLED)
    BLE_MANAGER_Queue,                           // Pointer to the event queue buffer
    BLE_MANAGER_QUEUE_DEPTH,                     // Number of events that the event queue can hold
#endif      // #if (SE_EVENT_QUEUE == ENABLED)
};

// End of BleEngineCrank Auto-Generated Code.

//=====================================================================================================================
//-------------------------------------- Public Functions -------------------------------------------------------------
//=====================================================================================================================

//---------------------------------------------------------------------------------------------------------------------
/**
 *  @brief      It Initializes the module BleManager and its variables
 *
 */
void BleManager__Initialize(void)
{
    // Initialize the BleManager state-machine.
    (void)BleEngine__Configure(&BLE_MANAGER_MACHINE);
}

/**
 *  @brief      Executes an iteration of the SysMan state machine
 *
 */
void BleManager__Handler(void *pvParameter)
{
    BleManager__Initialize();

    for(;;)
    {
        (void)BleEngine__Execute(&BLE_MANAGER);
        vTaskDelay(500 / portTICK_RATE_MS);
    }
    
}

/**
 *  @brief      reports the current state of the SysMan state machine
 *
 *  @return SYSMAN_STATE_TYPE
 */
BLE_MANAGER_STATE_TYPE BleManager__GetState(void)
{
    return ((BLE_MANAGER_STATE_TYPE)(BLE_MANAGER.Cur_State));
}

/**
 *  @brief      posts events to the SysMan state machine
 *
 *  @param event BLE_MANAGER_EVENT_TYPE
 */
void BleManager__PostEvent(BLE_MANAGER_EVENT_TYPE event)
{
    BleEngine__PostEvent(&BLE_MANAGER, (EVENT_TYPE) event);
}


void BleManager__ExternalProcess()
{

}


BLE_MANAGER_STATE_TYPE BleManager__EventToSystem( char **buf)
{

    BLE_MANAGER_STATE_TYPE state;
    state = BleManager__GetState();

    if(state == STATE_BLE_NEWMSG_RECEIVED)
    {
        *buf = newmsg;
        
    }
    
    

    return state;
    
}

//=====================================================================================================================
//-------------------------------------- Private Functions ------------------------------------------------------------
//=====================================================================================================================
/**
 *  @brief     BleInitializeEntry: entry into the Initialize state of the BleManager state machine
 *
 */
static EVENT_TYPE  BleInitializeEntry (void)
{
    // EspBle__Init();
    ESP_LOGI(TAG, "Bled Init Done");
    return (EVENT_NONE);
}

/**
 *  @brief      BleInitializeDo: do func for the Initialize state of the BleManager state machine
 *
 *  @param event EVENT_TYPE
 */
static EVENT_TYPE  BleInitializeDo (EVENT_TYPE event)
{
    ESP_LOGI(TAG, "Ble Init Do");
    BleManager__PostEvent(EV_BLE_INIT_COMPLETE);
    return (EVENT_NONE);
}

/**
 *  @brief      BleStandbyEntry: entry into the Standby state of the BleManager state machine
 *
 */
static EVENT_TYPE  BleStandbyEntry (void)
{
  
    return (EVENT_NONE);
}

/**
 *  @brief      BleStandbyDo: do func for the Standby state of the BleManager state machine
 *
 *  @param event EVENT_TYPE
 */
static EVENT_TYPE  BleStandbyDo (EVENT_TYPE event)
{
 
    return (EVENT_NONE);
}

/**
 *  @brief      BleGetMsgEntry: entry into the Ready state of the BleManager state machine
 *
 */
static EVENT_TYPE  BleGetMsgEntry (void)
{
   
    return (EVENT_NONE);
}

/**
 *  @brief      BleGetMsgDo: do func for the Ready state of the BleManager state machine
 *
 *  @param event EVENT_TYPE
 */
static EVENT_TYPE  BleGetMsgDo (EVENT_TYPE event)
{
   

    return (EVENT_NONE);
}

/**
 *  @brief      Action function: entry into the Error state of the BleManager state machine
 *
 */
static EVENT_TYPE  BlePublishMsgEntry (void)
{
  
    return (EVENT_NONE);
}

/**
 *  @brief      Action function: do func for the Error state of the BleManager state machine
 *
 *  @param event EVENT_TYPE
 */
static EVENT_TYPE  BlePublishMsgDo (EVENT_TYPE event)
{
   
    return (EVENT_NONE);
}

/**
 *  @brief      BleDiagnosticEntry: entry into the Diagnostic state of the BleManager state machine
 *
 */
static EVENT_TYPE  BleDiagnosticEntry (void)
{
   
    return (EVENT_NONE);
}

/**
 *  @brief      BleDiagnosticDo: do func for the Diagnostic state of the BleManager state machine
 *
 *  @param event EVENT_TYPE
 */
static EVENT_TYPE  BleDiagnosticDo (EVENT_TYPE event)
{
   
    return (EVENT_NONE);
}

/**
 *  @brief      BleDiagnosticEntry: entry into the Diagnostic state of the BleManager state machine
 *
 */
static EVENT_TYPE  BleNewMsgReceivedEntry (void)
{

    ESP_LOGI(TAG, "Ble New msg Entry");

    newmsg = EspBle__GetNewBleMsg();

    // ESP_LOGI(TAG, "NEW MESSAG = %s", newmsg);
   
    return (EVENT_NONE);
}

/**
 *  @brief      BleDiagnosticDo: do func for the Diagnostic state of the BleManager state machine
 *
 *  @param event EVENT_TYPE
 */
static EVENT_TYPE  BleNewMsgReceivedDo (EVENT_TYPE event)
{
   
    return (EVENT_NONE);
}

/**
 *  @brief      BleRunningEntry: entry into the Running state of the BleManager state machine
 *
 */
static EVENT_TYPE  BleRunningEntry (void)
{
  ESP_LOGI(TAG, "Ble Running Entry");
    return (EVENT_NONE);
}

/**
 *  @brief      BleRunningDo: do func for the Running state of the BleManager state machine
 *
 *  @param event EVENT_TYPE
 */
static EVENT_TYPE  BleRunningDo (EVENT_TYPE event)
{


    ESP_LOGI(TAG, "I am Running in BLE DO");
    vTaskDelay(1000 / portTICK_RATE_MS);
    return (EVENT_NONE);
}

/**
 *  @brief      BleStopEntry: entry into the Stopping state of the BleManager state machine
 *
 */
static EVENT_TYPE  BleStopEntry (void)
{
    

    return (EVENT_NONE);
}

/**
 *  @brief      BleStopDo: do func for the Stopping state of the BleManager state machine
 *
 *  @param event EVENT_TYPE
 */
static EVENT_TYPE  BleStopDo (EVENT_TYPE event)
{
   

    return (EVENT_NONE);
}

