/**
 *  @file
 *
 *  @brief      Parameter file to configure all timers
 *
 *  
 *
 *  @copyright  Copyright 2012-. Whirlpool Corporation. All rights reserved - CONFIDENTIAL
 */

//=====================================================================================================================
//-------------------------------------- PUBLIC (Extern Variables, Constants & Defines) -------------------------------
//=====================================================================================================================
/**
 * INTERRUPT PROTECTION
 *
 * The timers module will block interrupts during critical reads and writes of ms timer data if interrupt protection
 * is defined to be true. Otherwise, without protection, any functions that affect the Ms_Timers structure should not be
 * called from an interrupt.
 *
 * @param ENABLED - Enables interrupt protection
 * @param DISABLED - Disables interrupt protection
 */
#define INTERRUPT_PROTECTION        ENABLED


/**
 * TIME BASE SELECT
 *
 * Whether or not the calling routine will use the processor's time base module for timing (TRUE or FALSE).
 * Time base timing introduces a non-standard number of calls per second, which is a power of 2 (not power of 10).
 *
 * @param ENABLED - Enables time base selection
 * @param DISABLED - Disables time base selection
 */
#define TIME_BASE                   DISABLED

/**
 * TIMERS_INDIVIDUAL_HMS_COUNTERS
 *
 * This option enabled/disables the possibility of having individual counter for each HMS timer
 *
 * @param ENABLED - Enables counter for each HMS timer
 * @param DISABLED - Disables counter for each HMS timer
 */
#define TIMERS_INDIVIDUAL_HMS_COUNTERS  ENABLED

/**
 * TIME BASE VALUE
 * \-------------------------------------------------------------------------------------
 * \ The number of pulses emitted from the time base module in 1 second (usually 1024).
 * \ When TIME_BASE is FALSE, this value is irrelevant.
 */
#define TIME_BASE_VALUE             1024

/**
 *  MEMORY SECTIONS
 */
#if (MS_NUM > 0)
    //! if required the application specific can surround this declaration with #pragmas
    TYPE_MSTIMER Ms_Timers[MS_NUM];                     //!< MillisecondsTimer array
#endif

#if (SECONDS_NUM > 0)
    //! if required the application specific can surround this declaration with #pragmas
    TYPE_SECONDSTIMER Seconds_Timers[SECONDS_NUM];      //!< Seconds array
#endif

#if (MINUTES_NUM > 0)
    //! if required the application specific can surround this declaration with #pragmas
    TYPE_MINUTESTIMER Minutes_Timers[MINUTES_NUM];      //!< Minutes array
#endif

#if (HMS_NUM > 0)
    //! if required the application specific can surround this declaration with #pragmas
    TYPE_HMSTIMER Hms_Timers[HMS_NUM];                  //!< HMSTimer array
#endif

#if (TOD_SELECT == ENABLED)
    //! if required the application specific can surround this declaration with #pragmas
    TYPE_TOD TOD;                                       //!< TOD structure
#endif

#if (COUNTERS_NUM > 0)
    //! if required the application specific can surround this declaration with #pragmas
    SECONDSCOUNTUP_TYPE Counters_Timers[COUNTERS_NUM]; //!< Up counter array
#endif

//=====================================================================================================================
//-------------------------------------- PUBLIC (Function Prototypes) -------------------------------------------------
//=====================================================================================================================



