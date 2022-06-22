/**
 *  @file
 *
 *  @brief      State machine execution engines
 *
 *  @section    Applicable_Documents
 *                  State_Engine_Requirements.docx
 *
 *  
 *
 *  @copyright  *****  Copyright 2011.  Whirlpool Corporation.  All rights reserved - CONFIDENTIAL  *****
 */
//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------
#ifndef GsmGprsEngine_H_
    #define GsmGprsEngine_H_
#include "../../C_Extensions.h"
#include "GsmGprsEngine_prm.h"

//=====================================================================================================================
//-------------------------------------- PUBLIC (Extern Variables, Constants & Defines) -------------------------------
//=====================================================================================================================

//! event type

#if SE_LARGE_EVENT_POOL == ENABLED
typedef unsigned short EVENT_TYPE;
#else
typedef uint8 EVENT_TYPE;
#endif

//! state type
typedef uint8 STATE_TYPE;

typedef EVENT_TYPE* EVENT_POINTER_TYPE;

//! action function types (for entry, exit, and transition action functions)
typedef EVENT_TYPE ENTRY_FUNC_TYPE(void);                           //The entry function takes no arguments, return an EVENT_TYPE
typedef EVENT_TYPE ( *ENTRY_FUNC_POINTER_TYPE )( void );

typedef void TRANSITION_FUNC_TYPE(void);                            //The old action type has been subdivided into transition/exit types
typedef void ( *TRANSITION_FUNC_POINTER_TYPE )( void );

typedef void EXIT_FUNC_TYPE(void);
typedef void ( *EXIT_FUNC_POINTER_TYPE )( void );


//! state execute action function type
typedef EVENT_TYPE EXECUTE_FUNC_TYPE( EVENT_TYPE );                 //The execute function types an EVENT_TYPE argument, and returns an EVENT_TYPE
typedef EVENT_TYPE ( *EXECUTE_FUNC_POINTER_TYPE )( EVENT_TYPE );

//! transition guard condition function type
typedef T_BOOL GUARD_FUNC_TYPE( void );
typedef T_BOOL ( *GUARD_FUNC_POINTER_TYPE )( void );

// define the default event type/state type
#if SE_LARGE_EVENT_POOL == ENABLED
#define EVENT_NONE 0xFFFF
#else
#define EVENT_NONE 0xFF
#endif

#define STATE_NONE 0xFF

// define default action, execute action, and guard condition function pointers
#define ENTRY_FUNC_NONE         (ENTRY_FUNC_POINTER_TYPE) 0
#define TRANSITION_FUNC_NONE    (TRANSITION_FUNC_POINTER_TYPE) 0
#define EXIT_FUNC_NONE          (EXIT_FUNC_POINTER_TYPE) 0
#define EXECUTE_FUNC_NONE (EXECUTE_FUNC_POINTER_TYPE) 0
#define GUARD_FUNC_NONE (GUARD_FUNC_POINTER_TYPE) 0

//! state action functions type
typedef const struct
{
    ENTRY_FUNC_POINTER_TYPE    Entry_Func;            // entry function
    EXECUTE_FUNC_POINTER_TYPE   Exec_Func;             // execute function
    EXIT_FUNC_POINTER_TYPE    Exit_Func;             // exit function
} STATE_FUNCTION_TYPE;

//! state transition structure
typedef const struct
{
    EVENT_TYPE          Event;                  // transition event
    GUARD_FUNC_POINTER_TYPE    Guard_Condition_Func;   // guard condition function
    TRANSITION_FUNC_POINTER_TYPE   Transition_Func;        // transition function
    STATE_TYPE          State;                  // next state
} STATE_TRANSITION_TYPE;

//! state table structure
typedef const struct
{
    STATE_TYPE                Name;             // state name ( for clarity only )
    STATE_FUNCTION_TYPE       Functions;        // action functions
    STATE_TRANSITION_TYPE*    Transitions;      // pointer to an array of events for this state
} STATE_TABLE_TYPE;

//! state control structure
typedef struct
{
    STATE_TYPE            Cur_State;                // current state
    STATE_TABLE_TYPE*     Tables_Ptr;               // pointer to an array of states
    EVENT_POINTER_TYPE    Event_Queue_Ptr;          // event queue
    uint8         Queue_Size;               // queue size
    uint8         Queue_Count;              // number of items in the queue
    EVENT_POINTER_TYPE    Write_Ptr;                // queue write pointer
    EVENT_POINTER_TYPE    Read_Ptr;                 // queue read pointer
    STATE_TYPE            Initial_State;            // initial state
    T_BOOL                Configured;               // structure configured flag
} STATE_CONTROL_TYPE;

//! state machine definition
//! @image html "State Machine Definition Composition.jpg"
typedef const struct
{
    STATE_TYPE          Initial_State;          // initial state token
    STATE_TABLE_TYPE*   States;                 // pointer to an array of states
    STATE_CONTROL_TYPE* Control_Structure_Ptr;  // pointer to the control structure
    EVENT_TYPE*         Event_Queue_Ptr;        // pointer to the event queue
    uint8       Event_Queue_Size;    // event queue size
} STATE_MACHINE_DEFINITION_TYPE;

//=====================================================================================================================
//-------------------------------------- PUBLIC (Function Prototypes) -------------------------------------------------
//=====================================================================================================================
void GsmGprsEngine__InitializeStateMachine( STATE_CONTROL_TYPE* control_ptr );
EVENT_TYPE GsmGprsEngine__Execute( STATE_CONTROL_TYPE* control_ptr );
T_BOOL GsmGprsEngine__PostEvent( STATE_CONTROL_TYPE* control_ptr, EVENT_TYPE event );
void GsmGprsEngine__FlushEvents( STATE_CONTROL_TYPE* control_ptr );
T_BOOL GsmGprsEngine__Configure(STATE_MACHINE_DEFINITION_TYPE* const state_machine_defintion_ptr);
T_BOOL GsmGprsEngine__IsConfigured(STATE_CONTROL_TYPE* control_ptr );
STATE_TYPE GsmGprsEngine__GetCurrentState( STATE_CONTROL_TYPE* control_ptr );
uint8 GsmGprsEngine__GetEventsInQueue(STATE_CONTROL_TYPE* control_ptr );

#endif // GsmGprsEngine_H_
