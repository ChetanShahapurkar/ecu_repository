
#ifndef _TIMERS_H
#define _TIMERS_H

#include "esp_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "soc/timer_group_struct.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"
#include "driver/gpio.h"

#include "Timers_prm.h"

#define TIMER_INTR_SEL          TIMER_INTR_LEVEL  /*!< Timer level interrupt */
#define TIMER_GROUP             TIMER_GROUP_0     /*!< Test on timer group 0 */
#define TIMER_DIVIDER           80               /*!< Hardware timer clock divider, 80 to get 1MHz clock to timer */
#define TIMER_SCALE             (TIMER_BASE_CLK / TIMER_DIVIDER)  /*!< used to calculate counter value */
#define TIMER_FINE_ADJ          (0*(TIMER_BASE_CLK / TIMER_DIVIDER)/1000000) /*!< used to compensate alarm value */
// #define TIMER_INTERVAL0_SEC     (0.001000)   /*!< test interval for timer 0 */
#define TIMER_INTERVAL0_SEC   (0.00025)   /*!< test interval for timer 0 */


void IRAM_ATTR timer_group0_isr(void *para);
void tg0_timer0_init(void);
void tg0_timer_disable_int(void);



/**
 *  @file
 *
 *  @brief      Header file for timers module containing prototypes and enumerations for timers
 *
 *  
 *
 *  @copyright  Copyright 2012-. Whirlpool Corporation. All rights reserved - CONFIDENTIAL
 */


//=====================================================================================================================
//-------------------------------------- PUBLIC (Extern Variables, Constants & Defines) -------------------------------
//=====================================================================================================================



#ifdef BYTE_SIZE_2
    #define COUNTER_SIZE unsigned short int
#else
    #define COUNTER_SIZE unsigned long int
#endif

typedef struct                                          //HMS data structure
{
    unsigned char Hrs;
    unsigned char Min;
    unsigned char Sec;
}HMS;

typedef enum                                            //Timers states
{
    TIMERS_IDLE = 0,
    TIMERS_RUNNING = 1,
    TIMERS_PAUSED = 2,
    TIMERS_COMPLETED = 3,
    TIMERS_INVALID = 4
}TIMER_STATE;

typedef enum                                            //Counters states
{
    COUNTERS_IDLE = 0,
    COUNTERS_RUNNING = 1,
    COUNTERS_PAUSED = 2,
    COUNTERS_LIMIT_REACHED = 3,
    COUNTERS_INVALID = 4
}COUNTERS_STATE;

#define TOD_12 12                                       //TOD mode flag

#define TOD_24 23
//=====================================================================================================================
//-------------------------------------- PUBLIC (Function Prototypes) -------------------------------------------------
//=====================================================================================================================

void Timers__Initialize(void);
void IRAM_ATTR Timers__ServiceMs(void);
void Timers__MsSet(MS_TIMER_LABEL, unsigned short int);
void Timers__MsExpire(MS_TIMER_LABEL);
void Timers__MsDispose(MS_TIMER_LABEL);
void Timers__MsReset(MS_TIMER_LABEL);
void Timers__MsPause(MS_TIMER_LABEL);
void Timers__MsResume(MS_TIMER_LABEL);
TIMER_STATE Timers__MsGetStatus(MS_TIMER_LABEL);
unsigned short int Timers__MsGetTime(MS_TIMER_LABEL);

void Timers__ServiceHMS(void);
void Timers__HMSSet(HMS_TIMER_LABEL, unsigned char, unsigned char, unsigned char);
void Timers__HMSExpire(HMS_TIMER_LABEL);
void Timers__HMSReset(HMS_TIMER_LABEL);
void Timers__HMSPause(HMS_TIMER_LABEL);
void Timers__HMSResume(HMS_TIMER_LABEL);
TIMER_STATE Timers__HMSGetStatus(HMS_TIMER_LABEL);
void Timers__HMSGetTime(HMS_TIMER_LABEL, HMS *);

void Timers__TODSet(unsigned char, unsigned char, unsigned char);
void Timers__TODGetTime (HMS *);

void IRAM_ATTR Timers__ServiceSeconds(void);
void Timers__SecondsSet(SECONDS_LABEL id, unsigned short int seconds_value);
void Timers__SecondsExpire(SECONDS_LABEL);
void Timers__SecondsPause(SECONDS_LABEL);
void Timers__SecondsResume(SECONDS_LABEL);
void Timers__SecondsReset(SECONDS_LABEL);
TIMER_STATE Timers__SecondsGetStatus(SECONDS_LABEL);
unsigned short int Timers__SecondsGet(SECONDS_LABEL);

void Timers__ServiceMinutes(void);
void Timers__MinutesSet(MINUTES_LABEL, unsigned short int time);
void Timers__MinutesExpire(MINUTES_LABEL);
void Timers__MinutesPause(MINUTES_LABEL);
void Timers__MinutesResume(MINUTES_LABEL);
void Timers__MinutesReset(MINUTES_LABEL);
TIMER_STATE Timers__MinutesGetStatus(MINUTES_LABEL);
unsigned short int Timers__MinutesGet(MINUTES_LABEL);

void Timers__ServiceCounters(void);
void Timers__CounterStart(COUNTERS_LABEL id);
void Timers__CountersPause(COUNTERS_LABEL id);
void Timers__CountsResume(COUNTERS_LABEL id);
void Timers__CounterReset(COUNTERS_LABEL id);
COUNTERS_STATE Timers__CountersGetStatus(COUNTERS_LABEL id);
COUNTER_SIZE Timers__CountersGetCount(COUNTERS_LABEL id);

#endif

