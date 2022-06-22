/**
 *  @file
 *
 *  @brief      Commonly used timers & counters
 *
 *  @details    This GDM contains various timers and counters. Contents are:
 *              MS - Count down timer for miliseconds,
 *              HMS - Combined hours minutes and seconds in each counter.
 *              TOD - Time Of day counter used for systems that have a clock.
 *              Seconds - Count down timer for seconds.
 *              Minutes - Count down timer for minutes.
 *              Counter - Count up counter.
 *
 *  
 *
 *  @copyright  2012- = Whirlpool Corporation = All rights reserved - CONFIDENTIAL
 */

//-------------------------------------- Include Files ----------------------------------------------------------------
#include "Timers.h"
#include "../../C_Extensions.h"

//-------------------------------------- PUBLIC (Variables) -----------------------------------------------------------

//-------------------------------------- PRIVATE (Variables, Constants & Defines) -------------------------------------

//!MillisecondTimer structure
typedef struct
{
    unsigned char Active    :1;
    unsigned char Complete  :1;
    unsigned char Dispose   :1;
    unsigned char unused    :5;
    unsigned short int Count;
}TYPE_MSTIMER;

//!Seconds Timer structure
typedef struct
{
    unsigned char Active    :1;
    unsigned char Complete  :1;
    unsigned char Divisor   :6;
    unsigned short int Count;
}TYPE_SECONDSTIMER;

//!Minutes Timer structure
typedef struct
{
    unsigned char Active    :1;
    unsigned char Complete  :1;
    unsigned char unused    :6;
    unsigned short int Divisor;
    unsigned short int Count;
}TYPE_MINUTESTIMER;

//! HMSTimer structure
typedef struct
{
    unsigned char Active    :1;
    unsigned char Complete  :1;
    unsigned char unused    :6;
    HMS Time;
}TYPE_HMSTIMER;

//!TOD structure
typedef struct
{
    HMS Time;
}TYPE_TOD;

//!Count up structure
typedef struct
{
    COUNTERS_STATE State;
    unsigned short int Divisor;
    COUNTER_SIZE Count;
}SECONDSCOUNTUP_TYPE;


#include "Timers_prv.h"


#ifndef TIMERS_INDIVIDUAL_HMS_COUNTERS
    #define TIMERS_INDIVIDUAL_HMS_COUNTERS  DISABLED
#endif


#if (MS_TICK < 1)
    #error The MS_TICK parameter must be 1 or greater!!!.
#endif


#if (SECONDS_TICK < 1)
    #error The SECONDS_TICK parameter must be 1 or greater!!!.
#endif


#if (MINUTES_TICK < 1)
    #error The MINUTES_TICK parameter must be 1 or greater!!!.
#endif


#if (HMS_TICK < 1)
    #error The HMS_TICK parameter must be 1 or greater!!!.
#endif


#if (COUNTERS_TICK < 1)
    #error The COUNTERS_TICK parameter must be 1 or greater!!!.
#endif

#ifndef INTERRUPTS_ON_DEF
    #define INTERRUPTS_ON_DEF()
#endif

#ifndef INTERRUPTS_OFF_DEF
    #define INTERRUPTS_OFF_DEF()
#endif

#if ((HMS_NUM > 0) || (TOD_SELECT == ENABLED))      //If either HMSTimer or TOD selected to compile

    #if (TIMERS_INDIVIDUAL_HMS_COUNTERS == DISABLED)
        unsigned short int Hms_Count;               //Counts ticks until 1 sec
    #else
        unsigned short int Hms_Count[HMS_NUM];      //Counts ticks until 1 sec
        #if (TOD_SELECT == ENABLED)                 // If TOD compiled
            unsigned short int Tod_Count;           //Counts ticks until 1 sec
        #endif
    #endif
    const unsigned short int Hms_Count_Max = 1000 / HMS_TICK; //# of ticks in 1 sec
#endif

#if ((TOD_SELECT == ENABLED) && (TOD_MODE == TOD_12))   //If TOD is selected and in 12-hr mode
    #define TOD_ROLLOVER_VALUE 1                        //Rollover to 1
#elif (TOD_SELECT > 0)                                  //If TOD is in 24-hr mode
    #define TOD_ROLLOVER_VALUE 0                        //Rollover to 0
#endif

//-------------------------------------- PRIVATE (Function Prototypes) ------------------------------------------------

//=====================================================================================================================
//-------------------------------------- Public Functions -------------------------------------------------------------
//=====================================================================================================================

/**
 *  @brief      It Initializes the module Timers and its variables
 *
 *
 */
void Timers__Initialize(void)
{
    unsigned char i;

    #if ((HMS_NUM > 0) || (TOD_SELECT == ENABLED))  //If either TOD or HMSTimer is compiled
        #if (TIMERS_INDIVIDUAL_HMS_COUNTERS == DISABLED)
            Hms_Count = 0;                              //Clear tick counter
        #else
            for (i=0; i<HMS_NUM ; i++)
            {
                Hms_Count[i] = 0;                       //Counts ticks until 1 sec
            }
            #if (TOD_SELECT == ENABLED)                 // If TOD compiled
                Tod_Count = 0;
            #endif
        #endif

    #endif

    //Loop through all compiled MillisecondTimers
    #if (MS_NUM > 0)
        for (i=0; i < MS_NUM; i++)
        {
            Ms_Timers[i].Active = INACTIVE;
            Ms_Timers[i].Complete = TIMERS_INIT_STATE;
            Ms_Timers[i].Count = 0;
            Ms_Timers[i].Dispose = FALSE;
        }
    #endif

        //Loop through all compiled Seconds timers
    #if (SECONDS_NUM > 0)
        for (i=0; i < SECONDS_NUM; i++)
        {
            Seconds_Timers[i].Active = INACTIVE;
            Seconds_Timers[i].Complete = TIMERS_INIT_STATE;
            Seconds_Timers[i].Divisor = 0;
            Seconds_Timers[i].Count = 0;
        }
    #endif

        //Loop through all compiled Minutes timers
    #if (MINUTES_NUM > 0)
        for (i=0; i < MINUTES_NUM; i++)
        {
            Minutes_Timers[i].Active = INACTIVE;
            Minutes_Timers[i].Complete = TIMERS_INIT_STATE;
            Minutes_Timers[i].Divisor = 0;
            Minutes_Timers[i].Count = 0;
        }
    #endif

    //Loop throught all compiled Hms_Timers
    #if (HMS_NUM > 0)
        for (i=0; i < HMS_NUM; i++)
        {
            Hms_Timers[i].Active = INACTIVE;
            Hms_Timers[i].Complete = TIMERS_INIT_STATE;
            Hms_Timers[i].Time.Hrs = 0;
            Hms_Timers[i].Time.Min = 0;
            Hms_Timers[i].Time.Sec = 0;
        }
    #endif

    //Initialize TOD to 12:00:00
    #if (TOD_SELECT == ENABLED)
        TOD.Time.Hrs = 12;
        TOD.Time.Min = 0;
        TOD.Time.Sec = 0;
    #endif

        //Loop through all compiled Counters timers
    #if (COUNTERS_NUM > 0)
        for (i=0; i < COUNTERS_NUM; i++)
        {
            Counters_Timers[i].State = COUNTERS_INIT_STATE;
            Counters_Timers[i].Divisor = 1000/SECONDS_TICK;
            Counters_Timers[i].Count = 0;
        }
    #endif
}

//===================================== All Handlers ==========================+=====================
/**
 *  @brief      Services all compiled MillisecondTimers
 *
 */
void IRAM_ATTR Timers__ServiceMs(void)
{
#if (MS_NUM > 0)
    unsigned char i;                                //Loop count

    for(i=0; i < MS_NUM; i++)                       //Loop through all compiled Ms_Timers
    {
        if(Ms_Timers[i].Active)                     //Timer running?
        {
            Ms_Timers[i].Count--;                   //Decrement counter
            if(!(Ms_Timers[i].Count))               //Test if expired
            {
                Ms_Timers[i].Active = INACTIVE;     //Deactivate timer
                Ms_Timers[i].Complete = COMPLETE;   //Flag a timer timeout
            }
        }
    }
#endif
}

/**
 *  @Brief   Handler for the Seconds timers
 *
 */
void IRAM_ATTR Timers__ServiceSeconds(void)
{
#if (SECONDS_NUM > 0)
    unsigned char i;                                //Loop count

    for(i=0; i < SECONDS_NUM; i++)                  //Loop through all compiled seconds timers
    {
        if(Seconds_Timers[i].Active)                //Timer running?
        {
            Seconds_Timers[i].Divisor--;            //Decrement to manage divisor

            if(!Seconds_Timers[i].Divisor)          //Has divisor timed out?
            {
                Seconds_Timers[i].Divisor = 1000/SECONDS_TICK; //One second has expired. Reset divisor
                Seconds_Timers[i].Count--;              //Decrement counter
            }
            if(!(Seconds_Timers[i].Count))          //Test if expired
            {
                Seconds_Timers[i].Active = INACTIVE;//Deactivate timer
                Seconds_Timers[i].Complete = COMPLETE; //Flag a timer timeout
            }
        }
    }
#endif
}

/**
 *  @Brief   Handler for the Minutes timers
 *
 */
void Timers__ServiceMinutes(void)
{
#if (MINUTES_NUM > 0)
    unsigned char i;                                //Loop count

    for(i=0; i < MINUTES_NUM; i++)                  //Loop through all compiled minutes timers
    {
        if(Minutes_Timers[i].Active)                //Timer running?
        {
            Minutes_Timers[i].Divisor--;            //Decrement to manage divisor

            if(!Minutes_Timers[i].Divisor)          //Has divisor timed out?
            {
                Minutes_Timers[i].Divisor = 60000/MINUTES_TICK; //One minute has expired. Reset divisor
                Minutes_Timers[i].Count--;           //Decrement counter
            }
            if(!(Minutes_Timers[i].Count))           //Test if expired
            {
                Minutes_Timers[i].Active = INACTIVE; //Deactivate timer
                Minutes_Timers[i].Complete = COMPLETE; //Flag a timer timeout
            }
        }
    }
#endif
}

/**
 *  @Brief   Handler for the Counters timers
 *
 */
void Timers__ServiceCounters(void)
{
#if (COUNTERS_NUM > 0)
    unsigned char i;                                        //Loop count

    for(i=0; i < COUNTERS_NUM; i++)                         //Loop through all compiled seconds timers
    {
        if(Counters_Timers[i].State == COUNTERS_RUNNING)    //Timer running?
        {
            Counters_Timers[i].Divisor--;                   //Decrement to manage divisor

            if(!Counters_Timers[i].Divisor)                 //Has divisor timed out?
            {
                Counters_Timers[i].Divisor = 1000/SECONDS_TICK; //One second has expired. Reset divisor
                Counters_Timers[i].Count++;                 //Increment counter

                if(!Counters_Timers[i].Count)                   //Test if expired
                {
                    Counters_Timers[i].State = COUNTERS_LIMIT_REACHED;//Deactivate timer
                }
            }
        }
    }
#endif
}

//===================================== MillisecondTimers ==========================+=====================
/**
 *  @brief      Sets MillisecondTimer specified by 'id' to count down
 *              ms_value milliseconds. Also adjust for the time base module.
 *              \Also starts the timer.
 *
 *  @param      id :MsTimer id (enum label)
 *  @param      ms_value : millisecond value to set timer
 *
 */
void Timers__MsSet(MS_TIMER_LABEL id, unsigned short int ms_value)
{
#if (MS_NUM > 0)
    unsigned long temp;                             // Declare long for multiplication

    if (id < MS_NUM)
    {
        Ms_Timers[id].Dispose = FALSE;              // Clear the dispose bit
        if (ms_value < MS_TICK)                     // Is the requested time less than defined tic size?
        {
            Timers__MsExpire(id);                   // Set requested timer to expired
        }
        else                                        // If no error
        {
            INTERRUPTS_OFF_DEF();
            Ms_Timers[id].Active = INACTIVE;        // Signal timer is active
            Ms_Timers[id].Complete = COMPLETE;      // Flag a timer timeout
            INTERRUPTS_ON_DEF();
            #if TIME_BASE                           // If time base module used
                //Adjust ms_value to new time
                temp = (unsigned long)(TIME_BASE_VALUE * (unsigned long)ms_value) / 1000.0f;
                temp = temp / MS_TICK;
                INTERRUPTS_OFF_DEF();
                Ms_Timers[id].Count = (unsigned short int)temp;  // of ticks needed to reach new ms value
            #else
                temp = ms_value / MS_TICK;
                INTERRUPTS_OFF_DEF();
                Ms_Timers[id].Count = (unsigned short int)temp;  // of ticks needed to reach ms_value milliseconds
            #endif
            if (Ms_Timers[id].Count != 0)
            {
                Ms_Timers[id].Complete = INCOMPLETE;// Make sure time out flag is cleared
                Ms_Timers[id].Active = ACTIVE;      // Signal timer is active
            }
            INTERRUPTS_ON_DEF();
        }
  }
#endif
}

/**
 *  @brief      Resets the timer specified by 'id' to its initial default state of INCOMPLETE and INACTIVE.
 *
 *  @param      id : MsTimer id (enum label)
 *
 */
void Timers__MsReset(MS_TIMER_LABEL id)
{
#if (MS_NUM > 0)
    //If no error
    if (id < MS_NUM)
    {
        // Clear counter and set to inactive and incomplete
        INTERRUPTS_OFF_DEF();
        Ms_Timers[id].Active = INACTIVE;
        Ms_Timers[id].Count = 0;
        Ms_Timers[id].Complete = INCOMPLETE;
        INTERRUPTS_ON_DEF();
    }
#endif
}

/**
 *  @brief      Clears the MillisecondTimer counter specified by 'id' and
 *              sets it to INACTIVE and COMPLETE - thus making it appear as
 *              if it has timed out.
 *
 *  @param      id : MsTimer id (enum label)
 *
 */
void Timers__MsExpire(MS_TIMER_LABEL id)
{
#if (MS_NUM > 0)
    // If no error
    if (id < MS_NUM)
    {
        // Clear counter and set to inactive and complete
        INTERRUPTS_OFF_DEF();
        Ms_Timers[id].Active = INACTIVE;
        Ms_Timers[id].Count = 0;
        Ms_Timers[id].Complete = COMPLETE;
        INTERRUPTS_ON_DEF();
    }
#endif
}

/**
 *  @brief      Clears the MillisecondTimer counter specified by 'id' and
 *              sets it to DISPOSE 
 *
 *  @param      id : MsTimer id (enum label)
 *
 */
void Timers__MsDispose(MS_TIMER_LABEL id)
{
#if (MS_NUM > 0)
    // If no error
    if (id < MS_NUM)
    {
        // Clear counter and set to inactive and complete
        INTERRUPTS_OFF_DEF();
        Ms_Timers[id].Active = INACTIVE;
        Ms_Timers[id].Count = 0;
        Ms_Timers[id].Complete = COMPLETE;
        Ms_Timers[id].Dispose = TRUE;
        INTERRUPTS_ON_DEF();
    }
#endif
}

/**
 *  @brief      Pauses the MillisecondTimer specified by 'id' by making it inactive.
 *
 *  @param      id - MsTimer id (enum label)
 *
 */
void Timers__MsPause(MS_TIMER_LABEL id)
{
#if (MS_NUM > 0)
    // If no error
    if (id < MS_NUM)
    {
        INTERRUPTS_OFF_DEF();
        Ms_Timers[id].Active = INACTIVE;            // Deactivate timer
        INTERRUPTS_ON_DEF();
    }
#endif
}


/**
 *  @brief      Allows the MillisecondTimer specified by 'id' to resume from a pause by setting it to active.
 *
 *  @param      id - MsTimer id (enum label)
 *
 */
void Timers__MsResume(MS_TIMER_LABEL id)
{
#if (MS_NUM > 0)
    // If no error & time left in timer
    if ((id < MS_NUM) &&
        (Ms_Timers[id].Complete != COMPLETE))
    {
        Ms_Timers[id].Active = ACTIVE;
    }
#endif
}

/**
 *  @brief      Returns the status of the MsTimer specified by 'id'.
 *
 *  @param      id - MsTimer id (enum label)
 *
 *  @return     TIMER_STATE : TIMERS_IDLE, TIMERS_RUNNING, TIMERS_PAUSED, TIMERS_COMPLETED, TIMERS_INVALID
 */
TIMER_STATE Timers__MsGetStatus (MS_TIMER_LABEL id)
{
#if (MS_NUM > 0)
    TIMER_STATE state;                              // State to be returned
    TYPE_MSTIMER temp_ms;

    state = TIMERS_INVALID;                         // TIMERS_INVALID state

    if (id < MS_NUM)                                // If does not exist
    {
        INTERRUPTS_OFF_DEF();                       // Take timer data snapshot
        temp_ms = Ms_Timers[id];
        INTERRUPTS_ON_DEF();

        if (temp_ms.Active == ACTIVE)               // If timer is active
        {
            state = TIMERS_RUNNING;                 // must be RUNNING
        }
        else                                        // If timer is inactive
        {
            if(Ms_Timers[id].Dispose == TRUE)
            {

                state = TIMERS_IDLE;                // is TIMERS_IDLE

            }
            else
            {
                if (temp_ms.Complete == COMPLETE)       // If timer is complete
                {
                    state = TIMERS_COMPLETED;           // must be TIMERS_COMPLETED
                }
                else                                    // If inactive and incomplete
                {
                    if (temp_ms.Count > 0)              // If time left
                    {
                        state = TIMERS_PAUSED;          // must be TIMERS_PAUSED
                    }
                    else                                // Otherwise (inactive, incomplete, no time)
                    {
                        state = TIMERS_IDLE;            // is TIMERS_IDLE
                    }
                }

            }
            
        }
    }

    return (state);

#else
    return (TIMERS_INVALID);
#endif
}

/**
 *  @brief      Returns the time (in ms) remaining the MillisecondTimer identified by 'id'.
 *
 *  @param      id - MsTimer id (enum label)
 *
 *  @return     unsigned short int remaining time (ms)
 */
unsigned short int Timers__MsGetTime (MS_TIMER_LABEL id)
{
#if (MS_NUM > 0)
    unsigned short int temp_count;
    if (id >= MS_NUM)                               // If error exists
    {
        return 0;
    }
    else
    {
        INTERRUPTS_OFF_DEF();
        temp_count = Ms_Timers[id].Count;
        INTERRUPTS_ON_DEF();
        return (temp_count * MS_TICK);              // Convert # of ticks back to ms
    }
#else
    return (0);
#endif
}


//========================================== HMS TIMERS ===================================================
/**
 *  @brief      Services all compiled HMSimers and the TOD clock
 *
 */
void Timers__ServiceHMS(void)
{
#if ((HMS_NUM > 0) || (TOD_SELECT == ENABLED))      //HMS and TOD service routine block
    unsigned char i;                                // Loop counter

    #if (TIMERS_INDIVIDUAL_HMS_COUNTERS == DISABLED)
        Hms_Count++;                                    // Increment millisecond counter
        if (Hms_Count >= Hms_Count_Max)                 // Reached 1 sec?
        {
            Hms_Count = 0;                              // Resetting tick counter since has reached 1 sec

            #if (HMS_NUM > 0)                           // If HMSTimer compiled
                for (i=0; i < HMS_NUM; i++)             // Loop through all compiled HMStimers
                {
                    if (Hms_Timers[i].Active)           // Timer running?
                    {
                        if(Hms_Timers[i].Time.Sec)      // Any secs left?
                        {
                            Hms_Timers[i].Time.Sec--;
                            if (!((Hms_Timers[i].Time.Sec) |
                                  (Hms_Timers[i].Time.Min) |
                                  (Hms_Timers[i].Time.Hrs)))  //Timer finished?
                            {
                                Hms_Timers[i].Complete = COMPLETE;
                                Hms_Timers[i].Active = INACTIVE;
                            }
                        }
                        else if(Hms_Timers[i].Time.Min) // Any min left?
                        {
                            Hms_Timers[i].Time.Min--;
                            Hms_Timers[i].Time.Sec = 59;
                        }
                        else if(Hms_Timers[i].Time.Hrs) // Any hrs left?
                        {
                            Hms_Timers[i].Time.Hrs--;
                            Hms_Timers[i].Time.Min = 59;
                            Hms_Timers[i].Time.Sec = 59;
                        }
                    }
                }
            #endif                                      // End HMSTimer

            #if (TOD_SELECT == ENABLED)                 // If TOD compiled
                // TOD adjustment
                TOD.Time.Sec++;                         // Increment seconds
                if(TOD.Time.Sec >= 60)                  // Completed a min?
                {
                    TOD.Time.Sec = 0;
                    TOD.Time.Min++;                     // Increment minutes
                    if(TOD.Time.Min >= 60)              // Completed a min?
                    {
                        TOD.Time.Min = 0;
                        TOD.Time.Hrs++;                 // Increment hours
                        if(TOD.Time.Hrs > TOD_MODE)     // Rollover?
                        {
                            TOD.Time.Hrs = TOD_ROLLOVER_VALUE;
                        }
                    }
                }
            #endif                                      // End TOD
        }
    #else

        #if (TOD_SELECT == ENABLED)                 // If TOD compiled
            Tod_Count++;
            if (Tod_Count >= Hms_Count_Max)                 // Reached 1 sec?
            {
                Tod_Count = 0;                              // Resetting tick counter since has reached 1 sec
                // TOD adjustment
                TOD.Time.Sec++;                         // Increment seconds
                if(TOD.Time.Sec >= 60)                  // Completed a min?
                {
                    TOD.Time.Sec = 0;
                    TOD.Time.Min++;                     // Increment minutes
                    if(TOD.Time.Min >= 60)              // Completed a min?
                    {
                        TOD.Time.Min = 0;
                        TOD.Time.Hrs++;                 // Increment hours
                        if(TOD.Time.Hrs > TOD_MODE)     // Rollover?
                        {
                            TOD.Time.Hrs = TOD_ROLLOVER_VALUE;
                        }
                    }
                }
            }
        #endif                                      // End TOD

        #if (HMS_NUM > 0)                           // If HMSTimer compiled
            for (i=0; i < HMS_NUM; i++)             // Loop through all compiled HMStimers
            {
                if (Hms_Timers[i].Active)           // Timer running?
                {
                    Hms_Count[i]++;
                    if (Hms_Count[i] >= Hms_Count_Max)
                    {
                        Hms_Count[i] = 0;

                        if(Hms_Timers[i].Time.Sec)      // Any secs left?
                        {
                            Hms_Timers[i].Time.Sec--;
                            if (!((Hms_Timers[i].Time.Sec) |
                                  (Hms_Timers[i].Time.Min) |
                                  (Hms_Timers[i].Time.Hrs)))  //Timer finished?
                            {
                                Hms_Timers[i].Complete = COMPLETE;
                                Hms_Timers[i].Active = INACTIVE;
                            }
                        }
                        else if(Hms_Timers[i].Time.Min) // Any min left?
                        {
                            Hms_Timers[i].Time.Min--;
                            Hms_Timers[i].Time.Sec = 59;
                        }
                        else if(Hms_Timers[i].Time.Hrs) // Any hrs left?
                        {
                            Hms_Timers[i].Time.Hrs--;
                            Hms_Timers[i].Time.Min = 59;
                            Hms_Timers[i].Time.Sec = 59;
                        }
                    }
                }
            }
        #endif                                      // End HMSTimer


    #endif

#endif                                              // End HMS and TOD service routine block
}

/**
 *  @brief      Set Hms_Timers specified by 'id' to time hrs:min:sec
 *
 *  @param      id - HMSTimer id (enum label)
 *  @param      hrs - hours component of HMS timer
 *  @param      min - minute component of HMS timer
 *  @param      sec - second component of HMS timer
 *
 */
void Timers__HMSSet(HMS_TIMER_LABEL id, unsigned char hrs, unsigned char min, unsigned char sec)
{
#if (HMS_NUM > 0)                                   // Start HMSTimer block
    if (id < HMS_NUM)                               // If no error
    {
        Hms_Timers[id].Active = INACTIVE;           // Activate timer
        Hms_Timers[id].Time.Hrs = hrs;              //Set time
        Hms_Timers[id].Time.Min = min;
        Hms_Timers[id].Time.Sec = sec;

        if (!(hrs | min | sec))  //Timer set to 0?
        {
            Hms_Timers[id].Complete = COMPLETE;
            Hms_Timers[id].Active = INACTIVE;
        }
        else
        {
            Hms_Timers[id].Complete = INCOMPLETE;
            Hms_Timers[id].Active = ACTIVE;             // Activate timer
        }
        #if (TIMERS_INDIVIDUAL_HMS_COUNTERS == ENABLED )
            Hms_Count[id] = 0;
        #endif


    }
#endif
}

/**
 *  @brief      Clears the HMSTimer counter specified by 'id' and
 *              sets it to INACTIVE and COMPLETE - thus making it appear as
 *              \ if it has timed out.
 *
 *  @param      id - HMSTimer id (enum label)
 *
 */
void Timers__HMSExpire(HMS_TIMER_LABEL id)
{
#if (HMS_NUM > 0)                                   // Start HMSTimer block
    if (id < HMS_NUM)                               // If no error
    {
        // Deactivate timer and clear time
        Hms_Timers[id].Active = INACTIVE;
        Hms_Timers[id].Time.Hrs = 0;
        Hms_Timers[id].Time.Min = 0;
        Hms_Timers[id].Time.Sec = 0;
        Hms_Timers[id].Complete = COMPLETE;
    }
#endif
}

/**
 *  @brief      Resets the timer specified by 'id' to its initial default state of INCOMPLETE and INACTIVE.
 *
 *  @param      id - HMSTimer id (enum label)
 *
 */
void Timers__HMSReset(HMS_TIMER_LABEL id)
{
#if (HMS_NUM > 0)                                   // Start HMSTimer block
    if (id < HMS_NUM)                               // If no error
    {
        //! Deactivate timer and clear time
        Hms_Timers[id].Active = INACTIVE;
        Hms_Timers[id].Time.Hrs = 0;
        Hms_Timers[id].Time.Min = 0;
        Hms_Timers[id].Time.Sec = 0;
        Hms_Timers[id].Complete = INCOMPLETE;
    }
#endif
}

/**
 *  @brief      Pauses the HMSTimer specified by 'id' by setting to inactive.
 *
 *  @param      id - HMSTimer id (enum label)
 *
 */
void Timers__HMSPause(HMS_TIMER_LABEL id)
{
#if (HMS_NUM > 0)                                   // Start HMSTimer block
    if (id < HMS_NUM)                               // If no error
    {
        Hms_Timers[id].Active = INACTIVE;           // Deactivate timer
    }
#endif
}

/**
 *  @brief      Resumes the HMSTimer specified by 'id' by setting to active.
 *
 *  @param      id - HMSTimer id (enum label)
 *
 */
void Timers__HMSResume(HMS_TIMER_LABEL id)
{
#if (HMS_NUM > 0)                                   // Start HMSTimer block
    if ((id < HMS_NUM) &&
        (Hms_Timers[id].Complete != COMPLETE))      //If no error & not complete
    {
        Hms_Timers[id].Active = ACTIVE;
    }
#endif
}

/**
 *  @brief      Returns the status of the HMSTimer specified by 'id'.
 *
 *  @param      id - HMSTimer id (enum label)
 *
 *  @return     TIMER_STATE: TIMERS_IDLE, TIMERS_RUNNING, TIMERS_PAUSED, TIMERS_COMPLETED, TIMERS_INVALID
 */
TIMER_STATE Timers__HMSGetStatus (HMS_TIMER_LABEL id)
{
#if (HMS_NUM > 0)                                   // Start HMSTimer block
    TIMER_STATE state;                              // State to be returned

    state = TIMERS_INVALID;                         // TIMERS_INVALID state

    if (id < HMS_NUM)                               // If does not exist
    {
        if (Hms_Timers[id].Active == ACTIVE)        // If timer is active
        {
            state = TIMERS_RUNNING;                 // must be RUNNING
        }
        else                                        // If timer is inactive
        {
            if (Hms_Timers[id].Complete == COMPLETE)// If timer is complete
            {
                state = TIMERS_COMPLETED;           // must be TIMERS_COMPLETED
            }
            else                                    // If inactive and incomplete
            {
                if ( (Hms_Timers[id].Time.Hrs > 0) ||   // If time left
                     (Hms_Timers[id].Time.Min > 0) ||
                     (Hms_Timers[id].Time.Sec > 0) )
                {
                    state = TIMERS_PAUSED;          // must be TIMERS_PAUSED
                }
                else                                // Otherwise (inactive, incomplete, no time)
                {
                    state = TIMERS_IDLE;            // is TIMERS_IDLE
                }
            }
        }
    }


    return (state);
#else
    return (TIMERS_INVALID);
#endif
}

/**
 *  @brief      Returns the time remaining the HMSTimer defined by 'id'.
 *
 *  @param      id - HMSTimer id (enum label)
 *  @param      time - pointer to time structure to be filled
 *
 */
void Timers__HMSGetTime (HMS_TIMER_LABEL id, HMS * time)
{
#if (HMS_NUM > 0)                                   // Start HMSTimer block
    if (id < HMS_NUM)                               // If no error
    {
        // Copy time data over
        time->Hrs = Hms_Timers[id].Time.Hrs;
        time->Min = Hms_Timers[id].Time.Min;
        time->Sec = Hms_Timers[id].Time.Sec;
    }
#endif
}

//============================================== TOD ================================================================
/**
 *  @brief      Set the TOD clock to time hrs:min:sec
 *
 *  @param      hrs - hours component of TOD
 *  @param      min - minute component of TOD
 *  @param      sec - second component of TOD
 *
 */
void Timers__TODSet(unsigned char hrs, unsigned char min, unsigned char sec)
{
#if (TOD_SELECT == ENABLED)                         //Start TOD block
    if ((hrs < 24) &&
        (min < 60) &&
        (sec < 60))                                 // If no error
    {
        TOD.Time.Hrs = hrs;                         // Set time
        TOD.Time.Min = min;
        TOD.Time.Sec = sec;
    }
#endif
}


/**
 *  @brief      Returns TOD time
 *
 *  @param      time - pointer to time structure to be filled
 *
 */
void Timers__TODGetTime (HMS * time)
{
#if (TOD_SELECT == ENABLED)                         //Start TOD block
    // Copy time data over
    time->Hrs = TOD.Time.Hrs;
    time->Min = TOD.Time.Min;
    time->Sec = TOD.Time.Sec;
#endif
}

//========================================= Counters Timer ==========================================================
/**
 *  @brief      Resets specified counters variables and sets state to idle
 *
 *  @param      id - Counter to reset
 *
 */
void Timers__CounterReset(COUNTERS_LABEL id)
{
#if (COUNTERS_NUM > 0)
    if (id < COUNTERS_NUM)
    {
        // Reset counter and flag as active
        INTERRUPTS_OFF_DEF();
        Counters_Timers[id].Count = 0;
        Counters_Timers[id].Divisor = 1000/SECONDS_TICK;
        Counters_Timers[id].State = COUNTERS_IDLE;
        INTERRUPTS_ON_DEF();
    }
#endif
}

/**
 *  @brief      Resets and starts the specified counter.
 *
 *  @param      id - Counter to start
 *
 */
void Timers__CounterStart(COUNTERS_LABEL id)
{
#if (COUNTERS_NUM > 0)
    if (id < COUNTERS_NUM)
    {
        // Reset counter and flag as active
        INTERRUPTS_OFF_DEF();
        Counters_Timers[id].Count = 0;
        Counters_Timers[id].Divisor = 1000/SECONDS_TICK;
        Counters_Timers[id].State = COUNTERS_RUNNING;
        INTERRUPTS_ON_DEF();
    }
#endif
}

/**
 *  @brief      Pauses the specified counter and sets state to paused.
 *
 *  @param      id - Counter to pause
 *
 */void Timers__CountersPause(COUNTERS_LABEL id)
{
#if (COUNTERS_NUM > 0)
    if (id < COUNTERS_NUM)
    {
        INTERRUPTS_OFF_DEF();
        Counters_Timers[id].State = COUNTERS_PAUSED;            // Deactivate timer
        INTERRUPTS_ON_DEF();
    }
#endif
}

 /**
  *  @brief      Resumes the specified counter.
  *
  *  @param      id - Counter to resume
  *
  */void Timers__CountsResume(COUNTERS_LABEL id)
{
#if (COUNTERS_NUM > 0)
    if (id < COUNTERS_NUM)
    {
        Counters_Timers[id].State = COUNTERS_RUNNING;
    }
#endif
}

  /**
   *  @brief      Returns the status of the specified counter.
   *
   *  @param      id - Counter to return status of
   *
   *  @return     State of specified counter matching COUNTERS_STATE enum
   */
COUNTERS_STATE Timers__CountersGetStatus(COUNTERS_LABEL id)
{
#if (COUNTERS_NUM > 0)
    return (Counters_Timers[id].State);
#else
    return (COUNTERS_INVALID);
#endif
}

/**
 *  @brief      Returns the count of the specified counter.
 *
 *  @param      id - Counter to return value of
 *
 *  @return     Value of specifled counter
 *
 */
COUNTER_SIZE Timers__CountersGetCount(COUNTERS_LABEL id)
{
#if (COUNTERS_NUM > 0)
    if (id >= COUNTERS_NUM)                               // If error exists
    {
        return 0;
    }
    else
    {
        return(Counters_Timers[id].Count);
    }
#else
    return (0);
#endif
}

//========================================= Seconds Timer ==========================================================
/**
 *  @brief      Sets Seconds timer specified by 'id' to count down
 *              seconds_value seconds.
 *              Also starts the timer.
 *
 *  @param      id :SecondsTimer id (enum label)
 *  @param      seconds_value : seconds value to set timer
 *
 */
void Timers__SecondsSet(SECONDS_LABEL id, unsigned short int seconds_value)
{
#if (SECONDS_NUM > 0)
    if (id < SECONDS_NUM)
    {
        INTERRUPTS_OFF_DEF();
        Seconds_Timers[id].Active = INACTIVE;       // Signal timer is active
        Seconds_Timers[id].Complete = COMPLETE;     // Flag a timer timeout
        INTERRUPTS_ON_DEF();

        Seconds_Timers[id].Divisor = 1000/SECONDS_TICK;
        INTERRUPTS_OFF_DEF();
        Seconds_Timers[id].Count = seconds_value;   //Set seconds value
        if (Seconds_Timers[id].Count != 0)
        {
            Seconds_Timers[id].Complete = INCOMPLETE;// Make sure time out flag is cleared
            Seconds_Timers[id].Active = ACTIVE;     // Signal timer is active
        }
        INTERRUPTS_ON_DEF();
    }
#endif
}

/**
 *  @brief      Resets and sets state of specified counter to Complete.
 *
 *  @param      id - Counter to expire
 *
 */
void Timers__SecondsExpire(SECONDS_LABEL id)
{
#if (SECONDS_NUM > 0)
    if (id < SECONDS_NUM)
    {
        // Clear counter and set to inactive and complete
        INTERRUPTS_OFF_DEF();
        Seconds_Timers[id].Active = INACTIVE;
        Seconds_Timers[id].Count = 0;
        Seconds_Timers[id].Complete = COMPLETE;
        INTERRUPTS_ON_DEF();
    }
#endif
}

/**
 *  @brief      Resets specified counter and sets state to Incomplete.
 *
 *  @param      id - Counter to reset
 *
 */
void Timers__SecondsReset(SECONDS_LABEL id)
{
#if (SECONDS_NUM > 0)
    if (id < SECONDS_NUM)
    {
        // Clear counter and set to inactive and incomplete
        INTERRUPTS_OFF_DEF();
        Seconds_Timers[id].Active = INACTIVE;
        Seconds_Timers[id].Count = 0;
        Seconds_Timers[id].Complete = INCOMPLETE;
        INTERRUPTS_ON_DEF();
    }
#endif
}

/**
 *  @brief      Pauses specified counter.
 *
 *  @param      id - Counter to expire
 *
 */
void Timers__SecondsPause(SECONDS_LABEL id)
{
#if (SECONDS_NUM > 0)
    if (id < SECONDS_NUM)
    {
        INTERRUPTS_OFF_DEF();
        Seconds_Timers[id].Active = INACTIVE;       // Deactivate timer
        INTERRUPTS_ON_DEF();
    }
#endif
}

/**
 *  @brief      Resumes specified counter.
 *
 *  @param      id - Counter to resume
 *
 */
void Timers__SecondsResume(SECONDS_LABEL id)
{
#if (SECONDS_NUM > 0)
    if ((id < SECONDS_NUM) &&
        (Seconds_Timers[id].Complete != COMPLETE))
    {
        Seconds_Timers[id].Active = ACTIVE;
    }
#endif
}

/**
 *  @brief      Returns status of specified counter.
 *
 *  @param      id - Counter to return status of
 *
 *  @return     State of counter using TIMER_STATE enum.
 *
 */
TIMER_STATE Timers__SecondsGetStatus(SECONDS_LABEL id)
{
#if (SECONDS_NUM > 0)
    TIMER_STATE state;                              // State to be returned
    TYPE_SECONDSTIMER temp;

    state = TIMERS_INVALID;                         // TIMERS_INVALID state

    if (id < SECONDS_NUM)                           // If does not exist
    {
        INTERRUPTS_OFF_DEF();                       // Take timer data snapshot
        temp = Seconds_Timers[id];
        INTERRUPTS_ON_DEF();

        if (temp.Active == ACTIVE)                  // If timer is active
        {
            state = TIMERS_RUNNING;                 // must be RUNNING
        }
        else                                        // If timer is inactive
        {
            if (temp.Complete == COMPLETE)          // If timer is complete
            {
                state = TIMERS_COMPLETED;           // must be TIMERS_COMPLETED
            }
            else                                    // If inactive and incomplete
            {
                if (temp.Count > 0)                 // If time left
                {
                    state = TIMERS_PAUSED;          // must be TIMERS_PAUSED
                }
                else                                // Otherwise (inactive, incomplete, no time)
                {
                    state = TIMERS_IDLE;            // is TIMERS_IDLE
                }
            }
        }
    }

    return (state);

#else
    return (TIMERS_INVALID);
#endif
}

/**
 *  @brief      Returns seconds for specified counter.
 *
 *  @param      id - Counter to return value of
 *
 *  @return     Seconds for specified timer
 */
unsigned short int Timers__SecondsGet(SECONDS_LABEL id)
{
#if (SECONDS_NUM > 0)
    if (id >= SECONDS_NUM)                               // If error exists
    {
        return 0;
    }
    else
    {
        return(Seconds_Timers[id].Count);
    }
#else
    return (0);
#endif
}

//========================================= Minutes Timer ==========================================================
/**
 *  @brief      Sets Minutes timer specified by 'id' to count down
 *              minutes_value minutes.
 *              Also starts the timer.
 *
 *  @param      id :SecondsTimer id (enum label)
 *  @param      seconds_value : seconds value to set timer
 *
 */

void Timers__MinutesSet(MINUTES_LABEL id, unsigned short int time)
{
#if (MINUTES_NUM > 0)
    if (id < MINUTES_NUM)
    {
        INTERRUPTS_OFF_DEF();
        Minutes_Timers[id].Active = INACTIVE;       // Signal timer is active
        Minutes_Timers[id].Complete = COMPLETE;     // Flag a timer timeout
        INTERRUPTS_ON_DEF();

        Minutes_Timers[id].Divisor = 60000/MINUTES_TICK;
        INTERRUPTS_OFF_DEF();
        Minutes_Timers[id].Count = time;            //Set minutes value
        if (Minutes_Timers[id].Count != 0)
        {
            Minutes_Timers[id].Complete = INCOMPLETE;// Make sure time out flag is cleared
            Minutes_Timers[id].Active = ACTIVE;     // Signal timer is active
        }
        INTERRUPTS_ON_DEF();
    }
#endif
}

/**
 *  @brief      Resets and sets state of specified counter to Complete.
 *
 *  @param      id - Counter to expire
 *
 */
void Timers__MinutesExpire(MINUTES_LABEL id)
{
#if (MINUTES_NUM > 0)
    if (id < MINUTES_NUM)
    {
        // Clear counter and set to inactive and complete
        INTERRUPTS_OFF_DEF();
        Minutes_Timers[id].Active = INACTIVE;
        Minutes_Timers[id].Count = 0;
        Minutes_Timers[id].Complete = COMPLETE;
        INTERRUPTS_ON_DEF();
    }
#endif
}

/**
 *  @brief      Resets specified counter and sets state to Incomplete.
 *
 *  @param      id - Counter to reset
 *
 */
void Timers__MinutesReset(MINUTES_LABEL id)
{
#if (MINUTES_NUM > 0)
    if (id < MINUTES_NUM)
    {
        // Clear counter and set to inactive and incomplete
        INTERRUPTS_OFF_DEF();
        Minutes_Timers[id].Active = INACTIVE;
        Minutes_Timers[id].Count = 0;
        Minutes_Timers[id].Complete = INCOMPLETE;
        INTERRUPTS_ON_DEF();
    }
#endif
}

/**
 *  @brief      Pauses specified counter.
 *
 *  @param      id - Counter to expire
 *
 */
void Timers__MinutesPause(MINUTES_LABEL id)
{
#if (MINUTES_NUM > 0)
    if (id < MINUTES_NUM)
    {
        INTERRUPTS_OFF_DEF();
        Minutes_Timers[id].Active = INACTIVE;       // Deactivate timer
        INTERRUPTS_ON_DEF();
    }
#endif
}

/**
 *  @brief      Resumes specified counter.
 *
 *  @param      id - Counter to resume
 *
 */
void Timers__MinutesResume(MINUTES_LABEL id)
{
#if (MINUTES_NUM > 0)
    if ((id < MINUTES_NUM) &&
        (Minutes_Timers[id].Complete != COMPLETE))
    {
        Minutes_Timers[id].Active = ACTIVE;
    }
#endif
}

/**
 *  @brief      Returns status of specified counter.
 *
 *  @param      id - Counter to return status of
 *
 *  @return     State of counter using TIMER_STATE enum.
 *
 */
TIMER_STATE Timers__MinutesGetStatus(MINUTES_LABEL id)
{
#if (MINUTES_NUM > 0)
    TIMER_STATE state;                              // State to be returned
    TYPE_MINUTESTIMER temp;

    state = TIMERS_INVALID;                         // TIMERS_INVALID state

    if (id < MINUTES_NUM)                           // If does not exist
    {
        INTERRUPTS_OFF_DEF();                       // Take timer data snapshot
        temp = Minutes_Timers[id];
        INTERRUPTS_ON_DEF();

        if (temp.Active == ACTIVE)                  // If timer is active
        {
            state = TIMERS_RUNNING;                 // must be RUNNING
        }
        else                                        // If timer is inactive
        {
            if (temp.Complete == COMPLETE)          // If timer is complete
            {
                state = TIMERS_COMPLETED;           // must be TIMERS_COMPLETED
            }
            else                                    // If inactive and incomplete
            {
                if (temp.Count > 0)                 // If time left
                {
                    state = TIMERS_PAUSED;          // must be TIMERS_PAUSED
                }
                else                                // Otherwise (inactive, incomplete, no time)
                {
                    state = TIMERS_IDLE;            // is TIMERS_IDLE
                }
            }
        }
    }

    return (state);

#else
    return (TIMERS_INVALID);
#endif
}

/**
 *  @brief      Returns minutes for specified counter.
 *
 *  @param      id - Counter to return value of
 *
 *  @return     Minutes for specified timer
 */
unsigned short int Timers__MinutesGet(MINUTES_LABEL id)
{
#if (MINUTES_NUM > 0)
    if (id >= MINUTES_NUM)                          // If error exists
    {
        return 0;
    }
    else
    {
        return(Minutes_Timers[id].Count);
    }
#else
    return (0);
#endif
}

//=====================================================================================================================
//-------------------------------------- Private Functions ------------------------------------------------------------
//=====================================================================================================================


