/**
 *  @file
 *
 *  @brief      state machine execution engine
 *
 *  @details    The State Engine provides a utility to process a state machine (as defined in state tables) and event
 *  tokens posted by the application.
 *
 *  @note The State Engine, by design, does not have any dynamic data of its own.  All data storage is provided by
 *  the application.
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


//-------------------------------------- Include Files ----------------------------------------------------------------
#include <stdlib.h>
#include <stdio.h>
#include "GsmGprsEngine.h"

//-------------------------------------- PUBLIC (Variables) -----------------------------------------------------------

//-------------------------------------- PRIVATE (Variables, Constants & Defines) -------------------------------------
#if SE_INTERRUPT_SAFE_EXECUTE == ENABLED

#define SE_START_THREADSAFE_EVENT_QUEUE_BLOCK Micro__SaveInterruptEnableFlag();
#define SE_END_THREADSAFE_EVENT_QUEUE_BLOCK Micro__RestoreInterruptEnableFlag();

#else

#define SE_START_THREADSAFE_EVENT_QUEUE_BLOCK /* Nothing */
#define SE_END_THREADSAFE_EVENT_QUEUE_BLOCK /* Nothing */

#endif

//-------------------------------------- PRIVATE (Function Prototypes) ------------------------------------------------

static EVENT_TYPE ChangeState( STATE_CONTROL_TYPE* control_ptr, STATE_TRANSITION_TYPE* transition_ptr );
static EVENT_TYPE GetEvent( STATE_CONTROL_TYPE* control_ptr );

//=====================================================================================================================
//-------------------------------------- Public Functions -------------------------------------------------------------
//=====================================================================================================================


//---------------------------------------------------------------------------------------------------------------------
/**
 *  @brief      Initializes a state machine based on the data in the control structure

 *  @details    Transitions to the default state and executes the entry function (if any)
 *  
 *  @note       Can be used to re-initialize a "nested" state machine on reentry to the parent state
 *
 * @note       If our architecture supported exceptions, it would be appropriate to throw an exception
 *              if this function were called with an un-configured state machine.
 *
 *  @param      control_ptr = pointer to the current state control structure
 *
 */
void GsmGprsEngine__InitializeStateMachine( STATE_CONTROL_TYPE* control_ptr )
{
    ENTRY_FUNC_POINTER_TYPE func_ptr;

    // check for initialization
    if ((control_ptr != NULL) && (control_ptr->Configured == TRUE))
    {
        // initialize the event queue
        GsmGprsEngine__FlushEvents( control_ptr );

        // Set the initial state
        control_ptr->Cur_State = control_ptr->Initial_State;

        // Execute optional entry function
        func_ptr = control_ptr->Tables_Ptr[ control_ptr->Cur_State ].Functions.Entry_Func;
        if (  func_ptr != NULL )
        {
            // call the function
            func_ptr( );
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief       Configure the state machine information.
 *              Set initial values for all state machine variables.
 *
 *  @param      state_machine_defintion_ptr = pointer to the state machine definition structure
 *
 *  @return     returns TRUE if state machine was valid and the state machine could be configured
 *              returns FALSE if the state machine could not be configured
 */
T_BOOL GsmGprsEngine__Configure(STATE_MACHINE_DEFINITION_TYPE* state_machine_defintion_ptr)
{
    T_BOOL success = FALSE;
    STATE_CONTROL_TYPE* control_ptr;

    // Check for a valid state machine definition
    if((state_machine_defintion_ptr != NULL) &&
            (state_machine_defintion_ptr->Control_Structure_Ptr != NULL)&&
            (state_machine_defintion_ptr->States != NULL) &&
            (state_machine_defintion_ptr->Event_Queue_Ptr != NULL) &&
            (state_machine_defintion_ptr->Initial_State != STATE_NONE) &&
            (state_machine_defintion_ptr->Event_Queue_Size > 0)
            )
    {
        // Pick up the control structure pointer
        control_ptr = state_machine_defintion_ptr->Control_Structure_Ptr;

        // Fill out the control structure
        control_ptr->Initial_State      = state_machine_defintion_ptr->Initial_State;
        control_ptr->Tables_Ptr     = state_machine_defintion_ptr->States;
        control_ptr->Event_Queue_Ptr = state_machine_defintion_ptr->Event_Queue_Ptr;
        control_ptr->Queue_Size     = state_machine_defintion_ptr->Event_Queue_Size;

        // Record successful configuration
        control_ptr->Configured = TRUE;

        // Initialize the state machine
        GsmGprsEngine__InitializeStateMachine(control_ptr);        // Start the state machine

        success = TRUE;
    }
    return (success);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 *  @brief      Executes the current state
 *
 *  @param      control_ptr = pointer to the current state control structure
 *
 *  @note       If our architecture supported exceptions, it would be appropriate to throw an exception
 *              if this function were called with an un-configured state machine.
 *
 */
EVENT_TYPE GsmGprsEngine__Execute( STATE_CONTROL_TYPE* control_ptr )
{
    uint8 transition_index;
    EVENT_TYPE event;
    EVENT_TYPE ret_val = EVENT_NONE;

    EXECUTE_FUNC_POINTER_TYPE exec_func_ptr;
    GUARD_FUNC_POINTER_TYPE guard_func_ptr;
    STATE_TABLE_TYPE*  (state_ptr);

    T_BOOL found = FALSE;


    // check for initialization
    if ((control_ptr != NULL) && (control_ptr->Configured == TRUE))
    {
        // set the pointer
        state_ptr = &(control_ptr->Tables_Ptr[ control_ptr->Cur_State ]);

        // search for the event in the transition table
        transition_index = 0;

        // process a new event
        event = GetEvent( control_ptr );
        if ( event != EVENT_NONE )
        {
            // Loop through transition table
            while( state_ptr->Transitions[ transition_index ].Event != EVENT_NONE )
            {
                // Check for event match
                if ( state_ptr->Transitions[ transition_index ].Event == event )
                {
                    // Disqualify a bad (missing) target state
                    if(state_ptr->Transitions[ transition_index ].State != STATE_NONE)
                    {
                        STATE_TYPE target_state = state_ptr->Transitions[ transition_index ].State;
                        // Disqualify a transition if the target state doesn't know its name (perhaps the token is out of range)
                        if (control_ptr->Tables_Ptr[target_state].Name == target_state)
                        {
                            // Found a transition - qualify it with the optional guard condition
                            guard_func_ptr = state_ptr->Transitions[ transition_index ].Guard_Condition_Func;
                            if (guard_func_ptr != NULL)
                            {
                                // call it
                                if(guard_func_ptr() == TRUE)
                                    found = TRUE;
                            }
                            else
                            {
                                found = TRUE;
                            }

                            if (found == TRUE)
                            {
                                // if found, done looking
                                break;  // from while loop
                            }
                        }
                    }
                }  // end event match

                // keep looking
                // increment the transition table index
                transition_index++;
            }
            // done looping through transition table
        }

        if (found == TRUE)
        {
            // change states
            ret_val = ChangeState( control_ptr, &state_ptr->Transitions[ transition_index ] );
        }
        else  // No qualified transition found
        {
            // is there a execute function?
            exec_func_ptr = state_ptr->Functions.Exec_Func;
            if (exec_func_ptr != NULL)
            {
                // call it
                ret_val = exec_func_ptr(event );
            }
        }
    }
    return ret_val;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 *  @brief      Posts an event into the queue
 *
 *  @param      control_ptr = pointer to the current state control structure
 *  @param      event = event token to throw in the queue
 *
 *  @return     returns TRUE if event could be posted to the queue
 *              returns FALSE if the queue is full
 */
T_BOOL GsmGprsEngine__PostEvent( STATE_CONTROL_TYPE* control_ptr, EVENT_TYPE event )
{
    T_BOOL queued = FALSE;

    // check for initialization
    if ((control_ptr != NULL) && (control_ptr->Configured == TRUE))
    {
        SE_START_THREADSAFE_EVENT_QUEUE_BLOCK;
        // is there room
        if ( control_ptr->Queue_Count < control_ptr->Queue_Size )
        {
            // add the item to the queue
            *( control_ptr->Write_Ptr++ ) = event;

            // adjust the count
            control_ptr->Queue_Count++;

            // check for overflow
            if ( control_ptr->Write_Ptr >= ( control_ptr->Event_Queue_Ptr + control_ptr->Queue_Size ))
            {
                // reset it to beginning
                control_ptr->Write_Ptr = control_ptr->Event_Queue_Ptr;
            }

            // update status
            queued = TRUE;
        }
        SE_END_THREADSAFE_EVENT_QUEUE_BLOCK;
    }
    // return the status
    return ( queued );
}

//---------------------------------------------------------------------------------------------------------------------
/**
 *  @brief      flushes all events from the queue
 *
 *  @param      control_ptr = pointer to the current state control structure
 *
 */
void GsmGprsEngine__FlushEvents( STATE_CONTROL_TYPE* control_ptr )
{
    // check for initialization
    if ((control_ptr != NULL) && (control_ptr->Configured == TRUE))
    {
        SE_START_THREADSAFE_EVENT_QUEUE_BLOCK;
        // reset the pointers/set the count to 0
        control_ptr->Read_Ptr = control_ptr->Event_Queue_Ptr;
        control_ptr->Write_Ptr = control_ptr->Event_Queue_Ptr;
        control_ptr->Queue_Count = 0;
        SE_END_THREADSAFE_EVENT_QUEUE_BLOCK
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 *  @brief      Reports whether the control structure has been configured for this state machine
 *
 *  @param      control_ptr = pointer to the  state control structure
 *
 *  @return     returns TRUE if state machine is valid has been configured
 *              returns FALSE if the state machine has not been configured
 */
T_BOOL GsmGprsEngine__IsConfigured(STATE_CONTROL_TYPE* control_ptr )
{
    T_BOOL is_configued = FALSE;

    // check for initialization
    if ((control_ptr != NULL) && (control_ptr->Configured == TRUE))
    {
        is_configued = TRUE;
    }
    return (is_configued);
}


//---------------------------------------------------------------------------------------------------------------------
/**
 *  @brief      returns the count of events in the queue for this state machine
 *
 *  @param      control_ptr = pointer to the  state control structure
 *
 *  @return     the number of event tokens in the event queue
 */
uint8 GsmGprsEngine__GetEventsInQueue(STATE_CONTROL_TYPE* control_ptr )
{
    uint8 events_in_queue = 0;

    // check for initialization
    if ((control_ptr != NULL) && (control_ptr->Configured == TRUE))
    {
        events_in_queue = control_ptr->Queue_Count;
    }
    return (events_in_queue);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 *  @brief      returns the current state for this state machine
 *
 *  @param      control_ptr = pointer to the state control structure
 *
 *  @return     returns the current state of the state machine
 */
STATE_TYPE GsmGprsEngine__GetCurrentState( STATE_CONTROL_TYPE* control_ptr )
{
    STATE_TYPE current_state;


    // check for initialization
    if ((control_ptr != NULL) && (control_ptr->Configured == TRUE))
    {
        current_state = control_ptr->Cur_State;
    }
    else
    {
        current_state = STATE_NONE;
    }
    return (current_state);
}

//=====================================================================================================================
//-------------------------------------- Private Functions ------------------------------------------------------------
//=====================================================================================================================

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief       Progresses the state machine from one state to another.
 *
 *  @param      control_ptr = pointer to the state control structure
 *  @param      transition_ptr = pointer to data controlling this state transition
 */
static EVENT_TYPE ChangeState( STATE_CONTROL_TYPE* control_ptr, STATE_TRANSITION_TYPE* transition_ptr )
{
    EXIT_FUNC_POINTER_TYPE exit_func_ptr;
    TRANSITION_FUNC_POINTER_TYPE transition_func_ptr;
    ENTRY_FUNC_POINTER_TYPE entry_func_ptr;

    EVENT_TYPE ret_val = EVENT_NONE;

    // is there an exit function
    exit_func_ptr = control_ptr->Tables_Ptr[ control_ptr->Cur_State ].Functions.Exit_Func;
    if ( exit_func_ptr != NULL )
    {
        // execute it
        exit_func_ptr( );
    }

    // is there an transition function
    transition_func_ptr = transition_ptr->Transition_Func;
    if ( transition_func_ptr != NULL )
    {
        // call it
        transition_func_ptr( );
    }

    // change state
    control_ptr->Cur_State = transition_ptr->State;

    // is there an entry function
    entry_func_ptr = control_ptr->Tables_Ptr[ control_ptr->Cur_State ].Functions.Entry_Func;
    if ( entry_func_ptr != NULL )
    {
        // call it and remember the return value
        ret_val = entry_func_ptr( );
    }
    return ret_val;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 *  @brief      Dequeues the oldest event from the event queue
 *
 *  @param      control_ptr = pointer to the current state control structure
 *
 *  @return     returns an event if available or EVENT_NONE if not
 */
static EVENT_TYPE GetEvent( STATE_CONTROL_TYPE* control_ptr )
{
    EVENT_TYPE event = EVENT_NONE;

    // are there any entries
    if ( control_ptr->Queue_Count != 0 )
    {
        // get the event/increment the pointer
        event = *( control_ptr->Read_Ptr++ );

        // check for overflow
        if ( control_ptr->Read_Ptr >= ( control_ptr->Event_Queue_Ptr + control_ptr->Queue_Size ))
        {
            // reset it to beginning
            control_ptr->Read_Ptr = control_ptr->Event_Queue_Ptr;
        }

        // decrement the count
        SE_START_THREADSAFE_EVENT_QUEUE_BLOCK;
        control_ptr->Queue_Count--;
        SE_END_THREADSAFE_EVENT_QUEUE_BLOCK;
    }

    // return the event
    return (event);
}
