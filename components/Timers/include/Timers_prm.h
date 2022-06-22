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
 * Init state for counters and timers.
 *
 * This is used to set the state of the timers and counters when the Timers__Init() function is called.
 */
#define TIMERS_INIT_STATE       INCOMPLETE
#define COUNTERS_INIT_STATE     COUNTERS_IDLE;

//---------------------------------------------------------------------------------------------------------------------
/**
 * MILLISECOND TIMER
 */

/**
 * MILLISECOND TIMER LIST
 *
 * To add MS Timers, just add timer names to the list. (up to 255 timers).
 * MAKE SURE you adjust the #define MS_NUM to the appropriated number of
 * items in MS_TIMER_LABEL list.
 */
typedef enum
{
	MS__TIMER_LED_1,
	MS__TIMER_LED_2,
	MS__TIMER_LED_3,
	MS__TIMER_LED_4,
	MS__TIMER_LED_5,
	MS__TIMER_LED_6,
	MS__TIMER_LED_MAX,			
	MS__TIMER_A4988MOTOR_1,
	MS__TIMER_A4988MOTOR_2,
	MS__TIMER_A4988MOTOR_3,
	MS__TIMER_A4988MOTOR_MAX,
	MS__TIMER_GSM_POWERON,
	MS__TIMER_GSM_CHECKNETWORK,
	MS__TIMER_MQTT_WAIT_BEFORE_DISCONNECT,
	MS__TIMER_GSM_NETWORK_CHECK,
	MS__TIMER_GSM_RESET,
	MS_GSMGPRS_TIMER1,
	MS_GSMGPRS_WAIT,
	MS__GSMGPRS_STARTUP,
	MS__LOG,
	MS__LOG2,

    //Add MsTimers here
    MS__TIMER_NULL
} MS_TIMER_LABEL;

/**
 * Number of MS Timers
 * \ MAKE SURE you adjust the #define MS_NUM to the appropriated number of items in MS_TIMER_LABEL list.
 *
 */
#define MS_NUM              22

/**
 * MILLISECOND TIMER TICK RESOLUTION
 *
 * Resolution of MillisecondTimer service calls (in ms)
 * Recommended values: 1, 5, 20, 25, 50, or 100
 * This is how often the MsTimers service routine will be called.
 * @note i.e. if MS_TICK is 20, the Timers_MsService() routine should be called every 20 ms.
 * if the timers are to based off of zero-cross, the user
 * configures the timer for MS_TICK=50 (3*16.67 = 50). Thus, the
 * timer service routine would be called every 3 line cycles.
 */
#define MS_TICK             1

//---------------------------------------------------------------------------------------------------------------------
/**
 * SECONDS_TIMERS
 *
 */

/**
 * SECONDS TIMER LIST
 *
 * To add Seconds Timers, add timer names to the list. (up to 255 timers).
 *
 * @note MAKE SURE you adjust the #define SECONDS_NUM to the appropriated number of
 * items in SECONDS_LABEL list.
 *
 */
typedef enum
{
	SECONDS__TIMER_PUBLISH_TELEMETRY_MSG_INTERVAL,
	SECONDS__TIMER_PUBLISH_DIAGNOSTICS_MSG_INTERVAL,
	TIMER_ICECRUSH_MAX_RUN_TIMER,
	TIMER_LIFETEST_RUN_TIMER,
	TIMER_LIFETEST_RUN_TIMER_2,
	TIMER_RECIPE_TIMER,
	TIMER_STANDBY_MODE_TIMEOUT,
	TIMER_LED_BLINK,
    //Add Second Timers here
    SECONDS_TIMER1
} SECONDS_LABEL;

/**
 * Number of Seconds Timers
 *
 * @note MAKE SURE you adjust the #define SECONDS_NUM to the appropriated number of items in SECONDS_LABEL list.
 *
 */
#define SECONDS_NUM              9

/**
 * SECONDS TIMER TICK RESOLUTION
 *
 * Resolution of SecondsTimer service calls (in ms)
 * This is how often the SecondsTimers service routine will be called.
 *
 * @note i.e. if SECONDS_TICK is 25, the Timers__ServiceSeconds() routine should be called every 25 ms. If it's called every second then set to 1000
 */

#define SECONDS_TICK             25 //1 msec tick

//---------------------------------------------------------------------------------------------------------------------
/**
 * MINUTES_TIMERS
 *
 */

/**
 * MINUTES TIMER LIST
 *
 * To add Seconds Timers, add timer names to the list. (up to 255 timers).
 * MAKE SURE you adjust the #define MINUTES_NUM to the appropriated number of
 * items in MINUTES_LABEL list.
 */
typedef enum
{
    //Add Minutes Timers here
    MINUTES_TIMER1
} MINUTES_LABEL;

/**
 * Number of Minutes Timers
 * @note MAKE SURE you adjust the #define MINUTES_NUM to the appropriated number of items in MINUTES_LABEL list.
 *
 */
#define MINUTES_NUM              1

/**
 * MINUTES TIMER TICK RESOLUTION
 *
 * Resolution of Minutes Timer service calls (in ms)
 * This is how often the SecondsTimers service routine will be called.
 *
 * i.e. if MINTUES_TICK is 25, the Timers__ServiceMinutes() routine should be called every 25 ms. If it's called every second then set to 1000
 */
#define MINUTES_TICK 25

//---------------------------------------------------------------------------------------------------------------------
/**
 * HMS_TIMERS
 *
 */

/**
 * HMS TIMER LIST
 *
 * To add hms_timers, just add timer names to the list. (up to 255 timers).
 * To eliminate the HMSTimer from compiling, remove all items
 * from the list.
 *
 * @note MAKE SURE you adjust the #define HMS_NUM to the appropriated number of items in HMS_TIMER_LABEL list.
 */
typedef enum
{
    //Add HMSTimers here
    HMS_TIMER1
} HMS_TIMER_LABEL;

/**
 * Number of HMS timers
 * MAKE SURE you adjust the #define HMS_NUM to the appropriated number of items in HMS_TIMER_LABEL list.
 */
#define HMS_NUM             1

/**
 * HMS TIMER TICK RESOLUTION
 *
 * Resolution of HMSTimer service calls (in ms)
 * Recommended values: 1, 5, 20, 25, 50, 100, or 1000
 * This is how often the HMSTimers service routine will be called.
 * i.e. if HMS_TICK is 20, the Timers_HMSService() routine should
 * be called every 20 ms.
 * i.e if the timers are to based off of zero-cross, the user configure the timer for MS_TICK=50(3*16.67 = 50). Thus.
 * the timer service routine would be every 3 cycles.
 */
 #define HMS_TICK           5


//---------------------------------------------------------------------------------------------------------------------
/**
 * TOD_CLOCK
 */

/**
 * TOD SELECT
 *
 * Whether or not the user would like the TOD clock to be compiled (ENABLED or DISABLED)
 */
#define TOD_SELECT          ENABLED


/**
 * TOD MODE
 *
 * TOD mode: either 12-hr mode (TOD_12) or 24-hr mode (TOD_24).
 */
#define TOD_MODE            TOD_12

//---------------------------------------------------------------------------------------------------------------------
/**
 * Count up COUNTERS
 */

/**
 * COUNTERS TIMER LIST
 *
 * To add count up counters, add counter names to the list. (up to 255).
 * To eliminate the Counters functions from compiling, remove all items from the list
 *
 * @note MAKE SURE you adjust the #define COUNTERS_NUM to the appropriated number of items in COUNTERS_LABEL list.
 */

typedef enum
{
    //Add Counters Timers here
    COUNTUP_TEST1
} COUNTERS_LABEL;

/**
 * Number of Counters Timers
 * @note MAKE SURE you adjust the #define COUNTERS_NUM to the appropriated number of items in COUNTERS_LABEL list.
 */
#define COUNTERS_NUM              1

/**
 * COUNTERS TIMER TICK RESOLUTION
 *
 * Resolution of CountersTimer service calls (in ms)
 * This is how often the CountersTimers service routine will be called.
 *
 * @note i.e. if SECONDS_TICK is 25, the Timers_CountersService() routine should be called every 25 ms. If it's called every second then set to 1000
 */
#define COUNTERS_TICK             25

/**
 * COUNTERS SIZE
 *
 * Determines number of bytes (and therefore max time) the counter uses.
 *
 * @param BYTE_SIZE_2 - Uses 2 bytes to store count up value
 * @param BYTE_SIZE_4 - Uses 4 bytes to store count up value
 *
 * @note Example: If tic is set for a 1 second interval ( COUNTERS_TIC = 25) limits are:
 * @note  BYTE_SIZE_2 - you will get 65535 seconds, 1092 minutes, 18 hours or .8 days
 * @note  BYTE_SIZE_4 - you will get many seconds and minutes, 49710.3 days or 7001.4 weeks or 136.5 years.
 */
#define BYTE_SIZE_2

//=====================================================================================================================
//-------------------------------------- PUBLIC (Function Prototypes) -------------------------------------------------
//=====================================================================================================================



