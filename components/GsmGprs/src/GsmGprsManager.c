/**
 *  @file       
 *
 *  @brief      System Manager Module - the main Master application module
 *
 *  @details    GsmGprsManager is the main State Machine and controls the motor
 *
 *  @section    Applicable_Documents
 *					List here all the applicable documents if needed. <tr>	
 *
 *  $Header: GsmGprsManager.c 1.6 2014/12/05 23:59:09IST Chetana S Bhat (BHATCS) Exp  $
 *
 *  @copyright  Copyright 2014-$Date: 2014/12/05 23:59:09IST $. Whirlpool Corporation. All rights reserved - CONFIDENTIAL
 */
//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------


//-------------------------------------- Include Files ----------------------------------------------------------------
#include "GsmGprsManager.h" 
#include "GsmGprsEngine.h"
#include "Timers.h"
#include "sim7600.h"
#include "sim900A.h"
#include "libGSM.h"
#include <stdio.h>
#include <string.h>

#include "Espble.h"
#include "../../SoftwareConfig.h"

//-------------------------------------- PUBLIC (Variables) -----------------------------------------------------------
char mqtt_msg1[800];
char mqtt_msg2[800];
char topic_1[50];
char topic_2[50];
char * lattitude;
char * longitude;
char * epochTime;

char MQTT_BRK_URL[100];
char MQTT_ROUTING_KEY[50];
char MQTT_USERNAME[50];
char MQTT_PASSWORD[50];
char MQTT_VHOST[100];
char MQTT_PORT_NUM[10];


bool GsmGprsManager__InitDone = false;
bool gsm_gprs_network = false;
static uint8 network_check_retry_count;

//-------------------------------------- PRIVATE (Variables, Constants & Defines) -------------------------------------

static const char *TAG = "GSMGPRS"; 
bool GsmGprsManagerInitStatus = false;

static EVENT_TYPE  GsmGprsInitializeEntry (void);
static EVENT_TYPE  GsmGprsInitializeDo (EVENT_TYPE event);

static EVENT_TYPE  GsmGprsStandbyEntry (void);
static EVENT_TYPE  GsmGprsStandbyDo (EVENT_TYPE event);

static EVENT_TYPE  GsmGprsReadyEntry(void);
static EVENT_TYPE  GsmGprsReadyDo (EVENT_TYPE event);

static EVENT_TYPE  GsmGprsErrorEntry (void);
static EVENT_TYPE  GsmGprsErrorDo (EVENT_TYPE event);

static EVENT_TYPE  GsmGprsDiagnosticEntry (void);
static EVENT_TYPE  GsmGprsDiagnosticDo (EVENT_TYPE event);

static EVENT_TYPE  RunningEntry (void);
static EVENT_TYPE  GsmGprsRunningDo (EVENT_TYPE event);

static EVENT_TYPE  GsmGprsStopEntry (void);
static EVENT_TYPE  GsmGprsStopDo (EVENT_TYPE event);

static EVENT_TYPE  GsmGprsPowerOnEntry (void);
static EVENT_TYPE  GsmGprsPowerOnDo (void);

static EVENT_TYPE  GsmGprsModemInitEntry (void);
static EVENT_TYPE  GsmGprsModemInitDo (void);

static EVENT_TYPE  GsmGprsPowerOffEntry (void);
static EVENT_TYPE  GsmGprsPowerOffDo (void);

static EVENT_TYPE  GsmGprsCheckNetworkEntry (void);
static EVENT_TYPE  GsmGprsCheckNetworkDo (void);

static EVENT_TYPE  GsmGprsSetupNetworkTimeEntry (void);
static EVENT_TYPE  GsmGprsSetupNetworkTimeDO (void);

static EVENT_TYPE  GsmGprsMqttConnectEntry (void);
static EVENT_TYPE  GsmGprsMqttConnectDo (void);

static EVENT_TYPE  GsmGprsMqttDisconnectEntry (void);
static EVENT_TYPE  GsmGprsMqttDisconnectDo (void);
static EVENT_TYPE  GsmGprsMqttDisconnectExit (void);

static EVENT_TYPE  GsmGprsMqttReconnectEntry (void);
static EVENT_TYPE  GsmGprsMqttReconnectDo (void);

static EVENT_TYPE  GsmGprsMqttSendMessageEntry (void);
static EVENT_TYPE  GsmGprsMqttSendMessageDo (void);

static EVENT_TYPE  GsmGprsNewSmsReceivedEntry (void);
static EVENT_TYPE  GsmGprsNewSmsReceivedDo (void);

static EVENT_TYPE  GsmGprsMqttConnectNewTopicEntry (void);
static EVENT_TYPE  GsmGprsMqttConnectNewTopicDo (void);

//=====================================================================================================================
//=== GsmGprsEngineCrank Auto-Generated Code for the System Manager State Machine =======================================
//=====================================================================================================================

#if (SE_EVENT_QUEUE == ENABLED)
//! Maximum number of events that can be queued up in the System Manager state machine.
//! Implementation always executes as soon as the event is posted, so there is no reason for more than one.
#define SYSTEM_MANAGER_QUEUE_DEPTH 1
#endif      // #if (SE_QUEUE_ENABLED == ENABLED)





//! Data structure used by the GsmGprsEngine module to run the System Manager state machine.
static STATE_CONTROL_TYPE System_Manager;

#if (SE_EVENT_QUEUE == ENABLED)
//! Allocate memory for the System Manager state machine's event queue.
static EVENT_TYPE System_Manager_Queue[SYSTEM_MANAGER_QUEUE_DEPTH];
#endif      // #if (SE_EVENT_QUEUE == ENABLED)

//! List of transitions for Initialize State of the System Manager state machine.
static STATE_TRANSITION_TYPE TRANSITIONS_FOR_STATE_GSMGPRS_INITIALIZE[] =
{
//  {Event Name,         Guard Function,  Transition Action,    Next State},
    {EV_GSMGPRS_STANDBY,   GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_GSMGPRS_STANDBY},
    END_OF_TRANSITION_TABLE
};

//! List of transitions for Initialize State of the System Manager state machine.
static STATE_TRANSITION_TYPE TRANSITIONS_FOR_STATE_GSMGPRS_STANDBY[] =
{
//  {Event Name,         Guard Function,  Transition Action,    Next State},
    {EV_GSMGPRS_CHECKNETWORK,   GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_GSMGPRS_CHECKNETWORK},
    END_OF_TRANSITION_TABLE
};




//! List of transitions for Running State of the System Manager state machine.
static STATE_TRANSITION_TYPE TRANSITIONS_FOR_STATE_GSMGPRS_RUNNING[] =
{
//  {Event Name,          Guard Function,  Transition Action,    Next State},
    {EV_GSMGPRS_MQTTSEND_MESSAGE,       GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_GSMGPRS_MQTT_SEND_MESSAGE},
    {EV_GSMGPRS_NEWSMS_RECEIVED,        GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_GSMGPRS_NEWSMS_RECEIVED},
    {EV_GSMGPRS_CONNECT_NEW_TOPIC,      GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_GSMGPRS_CONNECT_NEW_TOPIC},
    {EV_GSMGPRS_MQTTCONNECT,            GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_GSMGPRS_MQTT_CONNECT},
    {EV_GSMGPRS_MQTTDISCONNECT,         GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_GSMGPRS_MQTT_DISCONNECT},
    END_OF_TRANSITION_TABLE
};


//! List of transitions for Stopping State of the System Manager state machine.
static STATE_TRANSITION_TYPE TRANSITIONS_FOR_STATE_GSMGPRS_MODEMINIT[] =
{
//  {Event Name,         Guard Function,  Transition Action,    Next State},
    {EV_GSMGPRS_MQTTCONNECT,            GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_GSMGPRS_MQTT_CONNECT},
    {EV_GSMGPRS_RUN,            GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_GSMGPRS_RUNNING},
    END_OF_TRANSITION_TABLE
};


//! List of transitions for Stopping State of the System Manager state machine.
static STATE_TRANSITION_TYPE TRANSITIONS_FOR_STATE_GSMGPRS_CHECKNETWORK[] =
{
//  {Event Name,         Guard Function,  Transition Action,    Next State},
    {EV_GSMGPRS_SETUP_NETWORK_TIME,             GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_GSMGPRS_SETUP_NETWORK_TIME},
    {EV_GSMGPRS_RUN,                            GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_GSMGPRS_RUNNING},
    END_OF_TRANSITION_TABLE
};


//! List of transitions for Stopping State of the System Manager state machine.
static STATE_TRANSITION_TYPE TRANSITIONS_FOR_STATE_GSMGPRS_SETUP_NETWORK_TIME[] =
{
//  {Event Name,         Guard Function,  Transition Action,    Next State},
    {EV_GSMGPRS_RUN,            GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_GSMGPRS_RUNNING},
    END_OF_TRANSITION_TABLE
};

//! List of transitions for Stopping State of the System Manager state machine.
static STATE_TRANSITION_TYPE TRANSITIONS_FOR_STATE_GSMGPRS_MQTTCONNECT[] =
{
//  {Event Name,         Guard Function,  Transition Action,    Next State},
    {EV_GSMGPRS_MQTTSEND_MESSAGE,       GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_GSMGPRS_MQTT_SEND_MESSAGE},
    {EV_GSMGPRS_NEWSMS_RECEIVED,        GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_GSMGPRS_NEWSMS_RECEIVED},
    {EV_GSMGPRS_CONNECT_NEW_TOPIC,      GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_GSMGPRS_CONNECT_NEW_TOPIC},
    {EV_GSMGPRS_MQTTCONNECT,            GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_GSMGPRS_MQTT_CONNECT},
    {EV_GSMGPRS_MQTTDISCONNECT,         GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_GSMGPRS_MQTT_DISCONNECT},
    {EV_GSMGPRS_RUN,            GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_GSMGPRS_RUNNING},
    END_OF_TRANSITION_TABLE
};

//! List of transitions for Stopping State of the System Manager state machine.
static STATE_TRANSITION_TYPE TRANSITIONS_FOR_STATE_GSMGPRS_MQTTDISCONNECT[] =
{
//  {Event Name,         Guard Function,  Transition Action,    Next State},
    {EV_GSMGPRS_MQTTSEND_MESSAGE,       GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_GSMGPRS_MQTT_SEND_MESSAGE},
    {EV_GSMGPRS_NEWSMS_RECEIVED,        GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_GSMGPRS_NEWSMS_RECEIVED},
    {EV_GSMGPRS_CONNECT_NEW_TOPIC,      GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_GSMGPRS_CONNECT_NEW_TOPIC},
    {EV_GSMGPRS_MQTTCONNECT,            GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_GSMGPRS_MQTT_CONNECT},
    {EV_GSMGPRS_MQTTDISCONNECT,         GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_GSMGPRS_MQTT_DISCONNECT},
    {EV_GSMGPRS_RUN,            GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_GSMGPRS_RUNNING},
    END_OF_TRANSITION_TABLE
};
//! List of transitions for Stopping State of the System Manager state machine.
static STATE_TRANSITION_TYPE TRANSITIONS_FOR_STATE_GSMGPRS_MQTTRECONNECT[] =
{
//  {Event Name,         Guard Function,  Transition Action,    Next State},
    {EV_GSMGPRS_RUN,            GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_GSMGPRS_RUNNING},
    {EV_GSMGPRS_CONNECT_NEW_TOPIC,      GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_GSMGPRS_CONNECT_NEW_TOPIC},
    END_OF_TRANSITION_TABLE
};


//! List of transitions for Stopping State of the System Manager state machine.
static STATE_TRANSITION_TYPE TRANSITIONS_FOR_STATE_GSMGPRS_MQTTREBOOTUPMESSAGE[] =
{
//  {Event Name,         Guard Function,  Transition Action,    Next State},
    {EV_GSMGPRS_MQTTSEND_MESSAGE,       GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_GSMGPRS_MQTT_SEND_MESSAGE},
    {EV_GSMGPRS_NEWSMS_RECEIVED,        GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_GSMGPRS_NEWSMS_RECEIVED},
    {EV_GSMGPRS_CONNECT_NEW_TOPIC,      GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_GSMGPRS_CONNECT_NEW_TOPIC},
    {EV_GSMGPRS_MQTTCONNECT,            GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_GSMGPRS_MQTT_CONNECT},
    {EV_GSMGPRS_MQTTDISCONNECT,         GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_GSMGPRS_MQTT_DISCONNECT},
    {EV_GSMGPRS_RUN,            GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_GSMGPRS_RUNNING},
    END_OF_TRANSITION_TABLE
};

//! List of transitions for Stopping State of the System Manager state machine.
static STATE_TRANSITION_TYPE TRANSITIONS_FOR_STATE_GSMGPRS_NEWSMSREC[] =
{
//  {Event Name,         Guard Function,  Transition Action,    Next State},
    {EV_GSMGPRS_RUN,            GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_GSMGPRS_RUNNING},
    {EV_GSMGPRS_MQTTSEND_MESSAGE,       GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_GSMGPRS_MQTT_SEND_MESSAGE},
    {EV_GSMGPRS_MQTTCONNECT,            GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_GSMGPRS_MQTT_CONNECT},
    {EV_GSMGPRS_MQTTDISCONNECT,         GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_GSMGPRS_MQTT_DISCONNECT},
    {EV_GSMGPRS_CONNECT_NEW_TOPIC,      GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_GSMGPRS_CONNECT_NEW_TOPIC},
    END_OF_TRANSITION_TABLE
};

//! List of transitions for Stopping State of the System Manager state machine.
static STATE_TRANSITION_TYPE TRANSITIONS_FOR_STATE_GSMGPRS_COnnECTTONEWTOPIC[] =
{
//  {Event Name,         Guard Function,  Transition Action,    Next State},
    {EV_GSMGPRS_MQTTSEND_MESSAGE,       GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_GSMGPRS_MQTT_SEND_MESSAGE},
    {EV_GSMGPRS_NEWSMS_RECEIVED,        GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_GSMGPRS_NEWSMS_RECEIVED},
    {EV_GSMGPRS_MQTTCONNECT,            GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_GSMGPRS_MQTT_CONNECT},
    {EV_GSMGPRS_MQTTDISCONNECT,         GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_GSMGPRS_MQTT_DISCONNECT},
    {EV_GSMGPRS_RUN,            GUARD_FUNC_NONE, TRANSITION_FUNC_NONE, STATE_GSMGPRS_RUNNING},
    END_OF_TRANSITION_TABLE
};



//! State table for the System Manager state machine.
static STATE_TABLE_TYPE SYSTEM_MANAGER_STATE_TABLE[] =
{
//  {State Name,       {Enter Action,    Do Action,            Exit Action},    Transition Table},
    {STATE_GSMGPRS_INITIALIZE,          {GsmGprsInitializeEntry,        GsmGprsInitializeDo,        EXIT_FUNC_NONE}, TRANSITIONS_FOR_STATE_GSMGPRS_INITIALIZE},
    {STATE_GSMGPRS_STANDBY,             {GsmGprsStandbyEntry,           GsmGprsStandbyDo,           EXIT_FUNC_NONE}, TRANSITIONS_FOR_STATE_GSMGPRS_STANDBY},
    {STATE_GSMGPRS_CHECKNETWORK,        {GsmGprsCheckNetworkEntry,      GsmGprsCheckNetworkDo,      EXIT_FUNC_NONE}, TRANSITIONS_FOR_STATE_GSMGPRS_CHECKNETWORK},
    {STATE_GSMGPRS_SETUP_NETWORK_TIME,  {GsmGprsSetupNetworkTimeEntry,  GsmGprsSetupNetworkTimeDO,  EXIT_FUNC_NONE}, TRANSITIONS_FOR_STATE_GSMGPRS_SETUP_NETWORK_TIME},
    
    
    {STATE_GSMGPRS_RUNNING,             {RunningEntry,                  GsmGprsRunningDo,           EXIT_FUNC_NONE}, TRANSITIONS_FOR_STATE_GSMGPRS_RUNNING},
    {STATE_GSMGPRS_MODEMINIT,           {GsmGprsModemInitEntry,         GsmGprsModemInitDo,         EXIT_FUNC_NONE}, TRANSITIONS_FOR_STATE_GSMGPRS_MODEMINIT},
    {STATE_GSMGPRS_MQTT_CONNECT,        {GsmGprsMqttConnectEntry,       GsmGprsMqttConnectDo,        EXIT_FUNC_NONE}, TRANSITIONS_FOR_STATE_GSMGPRS_MQTTCONNECT},
    {STATE_GSMGPRS_MQTT_DISCONNECT,     {GsmGprsMqttDisconnectEntry,    GsmGprsMqttDisconnectDo,     GsmGprsMqttDisconnectExit }, TRANSITIONS_FOR_STATE_GSMGPRS_MQTTDISCONNECT},
    {STATE_GSMGPRS_MQTT_RECONNECT,      {GsmGprsMqttReconnectEntry,     GsmGprsMqttReconnectDo,      EXIT_FUNC_NONE}, TRANSITIONS_FOR_STATE_GSMGPRS_MQTTRECONNECT},
    {STATE_GSMGPRS_MQTT_SEND_MESSAGE,   {GsmGprsMqttSendMessageEntry,   GsmGprsMqttSendMessageDo,      EXIT_FUNC_NONE}, TRANSITIONS_FOR_STATE_GSMGPRS_MQTTREBOOTUPMESSAGE},
    {STATE_GSMGPRS_NEWSMS_RECEIVED,     {GsmGprsNewSmsReceivedEntry,    GsmGprsNewSmsReceivedDo,      EXIT_FUNC_NONE}, TRANSITIONS_FOR_STATE_GSMGPRS_NEWSMSREC},
    {STATE_GSMGPRS_CONNECT_NEW_TOPIC,   {GsmGprsMqttConnectNewTopicEntry,  GsmGprsMqttConnectNewTopicDo,      EXIT_FUNC_NONE}, TRANSITIONS_FOR_STATE_GSMGPRS_COnnECTTONEWTOPIC},
};

//! State machine definition structure for the System Manager state machine.
static STATE_MACHINE_DEFINITION_TYPE SYSTEM_MANAGER_MACHINE =
{
    STATE_GSMGPRS_INITIALIZE,                               // Initial state
    SYSTEM_MANAGER_STATE_TABLE,                     // Table defining state behaviors
    &System_Manager,                                // Control structure
#if (SE_EVENT_QUEUE == ENABLED)
    System_Manager_Queue,                           // Pointer to the event queue buffer
    SYSTEM_MANAGER_QUEUE_DEPTH,                     // Number of events that the event queue can hold
#endif      // #if (SE_EVENT_QUEUE == ENABLED)
};

// End of GsmGprsEngineCrank Auto-Generated Code.

//=====================================================================================================================
//-------------------------------------- Public Functions -------------------------------------------------------------
//=====================================================================================================================

//---------------------------------------------------------------------------------------------------------------------
/**
 *  @brief      It Initializes the module GsmGprsManager and its variables
 *
 */
void GsmGprsManager__Initialize(void)
{
    // Initialize the GsmGprsManager state-machine.
    (void)GsmGprsEngine__Configure(&SYSTEM_MANAGER_MACHINE);
}

/**
 *  @brief      Executes an iteration of the SysMan state machine
 *
 */
void GsmGprsManager__Handler(void)
{
  
   (void)GsmGprsEngine__Execute(&System_Manager);

}

/**
 *  @brief      reports the current state of the SysMan state machine
 *
 *  @return SYSMAN_STATE_TYPE
 */
SYSTEM_MANAGER_STATE_TYPE GsmGprsManager__GetState(void)
{
    return ((SYSTEM_MANAGER_STATE_TYPE)(System_Manager.Cur_State));
}



/**
 *  @brief      posts events to the SysMan state machine
 *
 *  @param event SYSTEM_MANAGER_EVENT_TYPE
 */
void GsmGprsManager__PostEvent(SYSTEM_MANAGER_EVENT_TYPE event)
{
    GsmGprsEngine__PostEvent(&System_Manager, (EVENT_TYPE) event);
}
//=====================================================================================================================
//-------------------------------------- Private Functions ------------------------------------------------------------
//=====================================================================================================================
/**
 *  @brief     GsmGprsInitializeEntry: entry into the Initialize state of the GsmGprsManager state machine
 *
 */
static EVENT_TYPE  GsmGprsInitializeEntry (void)
{
    //Initialize the GPIO, and RING interrupt here
    ESP_LOGI(TAG,"GSM INIT Entry");

    //  sim7600__EnableRingInterrupt();
     sim7600__HardwareInit();

     Timers__MsSet(MS__GSMGPRS_STARTUP,GSM_STARTUP_TIME);

    
    return (EVENT_NONE);
}

/**
 *  @brief      GsmGprsInitializeDo: do func for the Initialize state of the GsmGprsManager state machine
 *
 *  @param event EVENT_TYPE
 */
static EVENT_TYPE  GsmGprsInitializeDo (EVENT_TYPE event)
{
  
    if(Timers__MsGetStatus(MS__LOG) == TIMERS_COMPLETED){ESP_LOGI(TAG,"GSM INIT Do");}
    if(Timers__MsGetStatus(MS__LOG) != TIMERS_RUNNING){Timers__MsSet(MS__LOG,1000);}

    if(sim7600__GetSystemBootStatus() == true)
    {
         Timers__MsSet(MS_GSMGPRS_WAIT,GSM_INTITAL_WAIT_TIME);
         GsmGprsManager__PostEvent(EV_GSMGPRS_STANDBY);       
    }
    if(Timers__MsGetStatus(MS__GSMGPRS_STARTUP) == TIMERS_COMPLETED)
    {
        Timers__MsSet(MS_GSMGPRS_WAIT,GSM_INTITAL_WAIT_TIME);
        GsmGprsManager__PostEvent(EV_GSMGPRS_STANDBY);
    }
     
    return (EVENT_NONE);
}


/**
 *  @brief     GsmGprsInitializeEntry: entry into the Initialize state of the GsmGprsManager state machine
 *
 */
static EVENT_TYPE  GsmGprsStandbyEntry (void)
{
    ESP_LOGI(TAG,"GSM Standby Entry");
    return (EVENT_NONE);
}

/**
 *  @brief      GsmGprsInitializeDo: do func for the Initialize state of the GsmGprsManager state machine
 *
 *  @param event EVENT_TYPE
 */
static EVENT_TYPE  GsmGprsStandbyDo (EVENT_TYPE event)
{
    if(Timers__MsGetStatus(MS__LOG) == TIMERS_COMPLETED){ESP_LOGI(TAG,"GSM Standby Do");}
    if(Timers__MsGetStatus(MS__LOG) != TIMERS_RUNNING){Timers__MsSet(MS__LOG,1000);}
    if(Timers__MsGetStatus(MS_GSMGPRS_WAIT) == TIMERS_COMPLETED)
    {
        sim7600__ModemBootsup();
        // libgsm__GetOperatorName();
        GsmGprsManager__PostEvent(STATE_GSMGPRS_CHECKNETWORK);  
    }
  
    return (EVENT_NONE);
}

/**
 * @brief 
 * 
 */
static EVENT_TYPE  GsmGprsCheckNetworkEntry (void)
{
     ESP_LOGI(TAG,"GSM Check Network Entry");
    Timers__MsSet(MS__TIMER_GSM_NETWORK_CHECK, GSM_NETWORK_RECHECK_TIME);
    return (EVENT_NONE);
}

/**
 * @brief 
 * 
 */
static EVENT_TYPE  GsmGprsCheckNetworkDo (void)
{
    if(Timers__MsGetStatus(MS__LOG) == TIMERS_COMPLETED){ESP_LOGI(TAG,"Check Network Do");}
    if(Timers__MsGetStatus(MS__LOG) != TIMERS_RUNNING){Timers__MsSet(MS__LOG,1000);}
    char *buff;
    char *network;
    
    

    if((Timers__MsGetStatus(MS__TIMER_GSM_NETWORK_CHECK) == TIMERS_COMPLETED) && (network_check_retry_count <= GSM_NEWTWORK_CHECK_RETRY_COUNT))
    {
        network_check_retry_count ++;
        ESP_LOGI(TAG,"Network Check Retry Count : %d",network_check_retry_count);
        Timers__MsSet(MS__TIMER_GSM_NETWORK_CHECK, GSM_NETWORK_RECHECK_TIME);
        GsmGprsManager__GetOperator_1(&network);
        if(!strcmp(network, ""))
        {
            // No network
        }
        else
        {
            // Network found
            // sim7600__NetTimeSetup();
            GsmGprsManager__PostEvent(EV_GSMGPRS_SETUP_NETWORK_TIME);// Network found , now go to setup network time settings
        }

        
    }
    
    if((network_check_retry_count > GSM_NEWTWORK_CHECK_RETRY_COUNT))
    {
        ESP_LOGI(TAG,"Network Check Timed Out");
        if(!strcmp(network, ""))
        {
            // No network
            GsmGprsManager__SetNetworkStatus(false);
            GsmGprsManager__PostEvent(EV_GSMGPRS_RUN);// if not network event after all retries, then simply go to RUN by setting the flag of no network
        }
        else
        {
            // Network found
            // sim7600__NetTimeSetup();
            GsmGprsManager__PostEvent(EV_GSMGPRS_SETUP_NETWORK_TIME);// Network found , now go to setup network time settings
        }

    }
    return (EVENT_NONE);
}



/**
 * @brief 
 * 
 */
static EVENT_TYPE  GsmGprsSetupNetworkTimeEntry (void)
{
     ESP_LOGI(TAG,"GSM Network time setup Entry");
    Timers__MsSet(MS__TIMER_GSM_RESET, GSM_RESET_WAIT_TIME);
    sim7600__EnableRingInterrupt();
    LibGSM__CNMIEnable();
    smsDeleteAll();
    // gsm_NetworkTimeSetup();
    
    return (EVENT_NONE);
}

/**
 * @brief 
 * 
 */
static EVENT_TYPE  GsmGprsSetupNetworkTimeDO (void)
{

 
    if(Timers__MsGetStatus(MS__LOG) == TIMERS_COMPLETED){ESP_LOGI(TAG,"GSM Network time setup Do");}
    if(Timers__MsGetStatus(MS__LOG) != TIMERS_RUNNING){Timers__MsSet(MS__LOG,1000);}

    if((Timers__MsGetStatus(MS__TIMER_GSM_RESET) == TIMERS_COMPLETED))
    {
         GsmGprsManager__PostEvent(EV_GSMGPRS_RUN);// Go to run after finishing network time setup and finishing gsm reset wait time
    }

    
    return (EVENT_NONE);
}


/**
 *  @brief      RunningEntry: entry into the Running state of the GsmGprsManager state machine
 *
 */

static EVENT_TYPE  RunningEntry (void)
{
 
    ESP_LOGI(TAG,"GSM Running Entry");
    return (EVENT_NONE);
}

/**
 *  @brief      GsmGprsRunningDo: do func for the Running state of the GsmGprsManager state machine
 *
 *  @param event EVENT_TYPE
 */
static EVENT_TYPE  GsmGprsRunningDo (EVENT_TYPE event)
{
    GsmGprsManagerInitStatus = true;
    if(Timers__MsGetStatus(MS__LOG) == TIMERS_COMPLETED){ESP_LOGI(TAG,"=====GPRS RUNNING=====");}
    if(Timers__MsGetStatus(MS__LOG) != TIMERS_RUNNING){Timers__MsSet(MS__LOG,1000);}


    // if new sms is received then generate event
    if(sim7600__GetNewSmsReceivedStatus() == true)
    {
        GsmGprsManager__PostEvent(EV_GSMGPRS_NEWSMS_RECEIVED);
    }


    return (EVENT_NONE);
}




/**
 * @brief 
 * 
 */
static EVENT_TYPE  GsmGprsModemInitEntry (void)
{
   
    ESP_LOGI(TAG,"GSM Modem Init Entry");

        // static uint8_t ct;
        
        // gsm_NetworkTimeSetup();

        // while(ct < 20)
        // {
        //     ct++;
        //      ESP_LOGI(TAG, "System Booting : %d", (30-ct));
        //      vTaskDelay(1000 / portTICK_RATE_MS);
        // }
        // smsDeleteAll();
        // sim7600__EnableRingInterrupt();
        // sim7600__ModemBootsup();

    return (EVENT_NONE);
}

/**
 * @brief 
 * 
 */
static EVENT_TYPE  GsmGprsModemInitDo (void)
{
    ESP_LOGI(TAG,"GSM Modem Init Do");
    GsmGprsManager__PostEvent(EV_GSMGPRS_RUN);

    return (EVENT_NONE);
}




/**
 * @brief 
 * 
 */
static EVENT_TYPE  GsmGprsMqttConnectEntry (void)
{

    ESP_LOGI(TAG, "Gsm MQTT connect entry");
    

    // char *buff;
    // char *network;
    // GsmGprsManager__GetOperator_1(&network);
    // if(!strcmp(network, ""))
    // {
    //     // No network
    //     GsmGprsManager__SetNetworkStatus(false);
    // }
    // else
    // {
    //     //Network present
    //     GsmGprsManager__SetNetworkStatus(true);
    //     sim7600__ConnectMQTT();
    //     Timers__MsSet(MS__TIMER_MQTT_WAIT_BEFORE_DISCONNECT,MQTT_DISCONNECT_WAIT_TIME);
    // }


    GsmGprsManager__SetNetworkStatus(true);
    sim7600__ConnectMQTT();
    Timers__MsSet(MS__TIMER_MQTT_WAIT_BEFORE_DISCONNECT,MQTT_DISCONNECT_WAIT_TIME);

    

    
    
    return (EVENT_NONE);
}

/**
 * @brief 
 * 
 */
static EVENT_TYPE  GsmGprsMqttConnectDo (void)
{
    

    if(Timers__MsGetStatus(MS__LOG) == TIMERS_COMPLETED){ESP_LOGI(TAG, "MQTT connect Do");}
    if(Timers__MsGetStatus(MS__LOG) != TIMERS_RUNNING){Timers__MsSet(MS__LOG,1000);}

    if(Timers__MsGetStatus(MS__TIMER_MQTT_WAIT_BEFORE_DISCONNECT) == TIMERS_COMPLETED)
    {
        GsmGprsManager__PostEvent(EV_GSMGPRS_MQTTDISCONNECT);
        
    }
    else
    {
      
    }
    return (EVENT_NONE);
}



/**
 * @brief 
 * 
 */
static EVENT_TYPE  GsmGprsMqttDisconnectEntry (void)
{  
    ESP_LOGI(TAG, "mqtt diconnect entry"); 
    sim7600__StopPPP();
    return (EVENT_NONE);
}

/**
 * @brief 
 * 
 */

static EVENT_TYPE  GsmGprsMqttDisconnectDo (void)
{
    if(Timers__MsGetStatus(MS__LOG) == TIMERS_COMPLETED){ESP_LOGI(TAG, "MQTT Disconnect Do");}
    if(Timers__MsGetStatus(MS__LOG) != TIMERS_RUNNING){Timers__MsSet(MS__LOG,1000);}

    if(sim7600__GetPPPStopStatus() == true)
    {
        GsmGprsManager__PostEvent(EV_GSMGPRS_RUN);
        //ppp mode is stopped so reconfigure the interrupt
        // sim7600__EnableRingInterrupt();
        ESP_LOGI(TAG, "Interrupt enabled..");   
    }
    
    return (EVENT_NONE);
}

static EVENT_TYPE  GsmGprsMqttDisconnectExit (void)
{
    sim7600__EnableRingInterrupt();
    ESP_LOGI(TAG, "Interrupt enabled..");   
    Application__BleEnable();
    ESP_LOGI(TAG, "MQTT Disconnect Exit");   
    return (EVENT_NONE);
}



/**
 * @brief 
 * 
 */
static EVENT_TYPE  GsmGprsMqttReconnectEntry (void)
{
    return (EVENT_NONE);
}

/**
 * @brief 
 * 
 */
static EVENT_TYPE  GsmGprsMqttReconnectDo (void)
{
    return (EVENT_NONE);
}



/**
 * @brief 
 * 
 */
static EVENT_TYPE  GsmGprsMqttSendMessageEntry (void)
{

    GsmGprsManager__PostEvent(EV_GSMGPRS_MQTTCONNECT);
    
    return (EVENT_NONE);
}

/**
 * @brief 
 * 
 */
static EVENT_TYPE  GsmGprsMqttSendMessageDo (void)
{
    ESP_LOGI(TAG,"=====sendmsg do=====");
    return (EVENT_NONE);
}

/**
 * @brief 
 * 
 */
static EVENT_TYPE  GsmGprsNewSmsReceivedEntry (void)
{
    ESP_LOGI(TAG, "new msg entry");
    // vTaskDelay(1000 / portTICK_RATE_MS);
    libGSM__SetSmSDecodestatus(false);
    ReadNewSms();

    return (EVENT_NONE);
}

/**
 * @brief 
 * 
 */
static EVENT_TYPE  GsmGprsNewSmsReceivedDo (void)
{
    

    if(Timers__MsGetStatus(MS__LOG) == TIMERS_COMPLETED)
    {
        ESP_LOGI(TAG, "new msg do");
    }

    if(Timers__MsGetStatus(MS__LOG) != TIMERS_RUNNING)
    {
        Timers__MsSet(MS__LOG,1000);
    }


    // if(libGSM__GetSmSDecodestatus() == false)
    // {
    //     // Message is decoded and read by application, so now go to running state
    //     sim7600__SetNewSmsReceivedStatus(false);
    //     // GsmGprsManager__PostEvent(EV_GSMGPRS_RUN);

    // }


    return (EVENT_NONE);
}



/**
 * @brief 
 * 
 */
static EVENT_TYPE  GsmGprsMqttConnectNewTopicEntry (void)
{

    ESP_LOGI(TAG, "MQTT connect to newt topic entry");
    // Always first check the network before connecting to MQTT

    // sim7600__NetworkCheck();
    // char *network;
    // GsmGprsManager__GetOperator_1(&network);
    // if(!strcmp(network, ""))
    // {
    //     // No network
    //     GsmGprsManager__SetNetworkStatus(false);
    // }
    // else
    // {
    //     //Network present
    //     // GsmGprsManager__PostEvent(EV_GSMGPRS_RUN);
    //     GsmGprsManager__SetNetworkStatus(true);
    //     sim7600__ConnectMQTT();
    //     Timers__MsSet(MS__TIMER_MQTT_WAIT_BEFORE_DISCONNECT,MQTT_TEMP_TOPIC_DISCONNECT_WAIT_TIME);
    // }



    GsmGprsManager__SetNetworkStatus(true);
    sim7600__ConnectMQTT();
    Timers__MsSet(MS__TIMER_MQTT_WAIT_BEFORE_DISCONNECT,MQTT_TEMP_TOPIC_DISCONNECT_WAIT_TIME);


    return (EVENT_NONE);
}

char current_cmd[20];

void GsmGprs__SetCurrentCMD(char * cmd)
{
    strcpy(current_cmd, cmd);
}
void GsmGprs__GetCurrentCMD(char **buff)
{
    *buff = current_cmd;
}

/**
 * @brief 
 * 
 */
static EVENT_TYPE  GsmGprsMqttConnectNewTopicDo (void)
{

    ESP_LOGI(TAG,"NEW topic do");

    if((Timers__MsGetStatus(MS__TIMER_MQTT_WAIT_BEFORE_DISCONNECT) == TIMERS_COMPLETED) &&  (GsmGprsManager__GetNetworkStatus() == true))
    {
        //Disconnect mqtt and ppp connection here
        // ESP_LOGI(TAG, "mqtt diconnected"); 
        GsmGprsManager__PostEvent(EV_GSMGPRS_MQTTDISCONNECT);
   
        
    }

   
    return (EVENT_NONE);
}


void GsmGprsManager__ProcessExternalEvent(SYSTEM_MANAGER_EVENT_TYPE ev,char *msg1, char *msg2, char *tp1, char *tp2, char *mqtthost, char *mqttVhost, char *mqttport, char *mqttusername, char *mqttpassword)
{

    memset(mqtt_msg1, '\0', sizeof(mqtt_msg1));
    memset(mqtt_msg2, '\0', sizeof(mqtt_msg2));
    memset(topic_1, '\0', sizeof(topic_1));
    memset(topic_2, '\0', sizeof(topic_2));

    memset(MQTT_BRK_URL, '\0', sizeof(MQTT_BRK_URL));
    memset(MQTT_ROUTING_KEY, '\0', sizeof(MQTT_ROUTING_KEY));
    memset(MQTT_USERNAME, '\0', sizeof(MQTT_USERNAME));
    memset(MQTT_PASSWORD, '\0', sizeof(MQTT_PASSWORD));
    memset(MQTT_VHOST, '\0', sizeof(MQTT_VHOST));



    sim7600__SetMqttMsgSendStatus(false);
        
    strcpy(mqtt_msg1, msg1);
    strcpy(mqtt_msg2, msg2);
    strcpy(topic_1, tp1);
    strcpy(topic_2, tp2);

    strcpy(MQTT_BRK_URL, mqtthost);
    strcpy(MQTT_VHOST, mqttVhost);
    strcpy(MQTT_USERNAME, mqttusername);
    strcpy(MQTT_PASSWORD, mqttpassword);
    strcpy(MQTT_PORT_NUM, mqttport);

    GsmGprsManager__PostEvent(ev);



    // if(ev == EV_GSMGPRS_MQTTSEND_MESSAGE)
    // {
    //     sim7600__SetMqttMsgSendStatus(false);
        
    //     strcpy(mqtt_msg1, msg1);
    //     strcpy(mqtt_msg2, msg2);
    //     strcpy(topic1, tp1);
    //     strcpy(topic2, tp2);

    //     GsmGprsManager__PostEvent(ev);
    // }
    // else if(ev == EV_GSMGPRS_CONNECT_NEW_TOPIC)
    // {
        
    //     mqtt_msg1 = msg1;
    //     mqtt_msg2 = msg2;
    //     new_queue = msg2;
    //     GsmGprsManager__PostEvent(ev);
    //     ESP_LOGI(TAG, "event posted - %d",ev);
    // }
    // else if(ev == EV_GSMGPRS_MQTTCONNECT)
    // {
    //     new_queue = "NULL";
    //     GsmGprsManager__PostEvent(ev);
    // }
    

}


void GsmGprsManager__GetMsgToSend(char **msg1, char ** msg2) 
{
    *msg1 = mqtt_msg1;
    *msg2 = mqtt_msg2;
}

void GsmGprsManager__GetTopicsSend(char **top1, char ** top2) 
{
    *top1 = topic_1;
    *top2 = topic_2;
}

void GsmGprsManager__GetMQTTConfiguration(char **mqtthost, char ** mqttVhost, char ** mqttport, char ** mqttusername, char ** mqttpass) 
{
    *mqtthost =  MQTT_BRK_URL;
    *mqttVhost = MQTT_VHOST;
    *mqttport = MQTT_PORT_NUM;
    *mqttusername = MQTT_USERNAME;
    *mqttpass = MQTT_PASSWORD;

}


bool GsmGprsManager__GetMsgSendStatus()
{
    return sim7600__GetMqttMsgSendStatus();
}

const char * GsmGprsManager__GetLattitude(void)
{
  
    return "18.5817";
}

const char * GsmGprsManager__GetLongitude(void)
{
 
    return "73.8202";
}

void GsmGprsManager__ConnectToQueue(void)
{
    
}
char * GsmGprsManager__GetEpochTime(void)
{
    return "1647421132";
}

void GsmGprsManager__GetImei_1(char **buff)
{
    char *temp;
    sim7600__GetImei(&temp);
    *buff = temp;  
}

void GsmGprsManager__GetImsi_1(char **buff)
{
    char *temp;
    sim7600__GetImsi(&temp);
    *buff = temp; 

}

char * GsmGprsManager__GetImei_2(void)
{
    return "869668021173566";
}

uint32_t GsmGprsManager__GetRssi_1(void)
{
 
    return sim7600__GetRssi();
 
}

void GsmGprsManager__GetOperator_1(char **buff)
{
    sim7600__NetworkCheck();
    char *temp;
    sim7600__GetOperator(&temp);
    *buff = temp; 

}

char * GsmGprsManager__GetRssi_2(void)
{
    return "43.2";
}


bool GsmGprs__GetInitStatus()
{
    return GsmGprsManager__InitDone;
}

void GsmGprsManager__GetNetworkTime(char **network_time)
{
    char *tm;
    gsm_getTime(&tm);
    *network_time = tm;
}


void GsmGprsManager__SetNetworkStatus(bool status)
{
    gsm_gprs_network = status;
}

bool GsmGprsManager__GetNetworkStatus(void)
{
    return gsm_gprs_network;
}


void GsmGprsManager__remove_PPP_Mode()
{
    sim7600__StopPPP();
}


bool GsmGprsManager__initStatus(void)
{
    return GsmGprsManagerInitStatus;
}





