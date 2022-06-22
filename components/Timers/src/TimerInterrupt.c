#include "Timers.h"
#include "../../main/main.h"

void IRAM_ATTR timer_group0_isr(void *para)
{// timer group 0, ISR
	int timer_idx = (int) para;
	static int cnt;
	uint32_t intr_status = TIMERG0.int_st_timers.val;
	if((intr_status & BIT(timer_idx)) && timer_idx == TIMER_0) {
		TIMERG0.hw_timer[timer_idx].update = 1;
		TIMERG0.int_clr_timers.t0 = 1;
		TIMERG0.hw_timer[timer_idx].config.alarm_en = 1;

		Main__TimerISR();

		
	}
}

void tg0_timer0_init(void)
{
	int timer_group = TIMER_GROUP_0;
	int timer_idx = TIMER_0;
	timer_config_t config;
	config.alarm_en = 1;
	config.auto_reload = 1;
	config.counter_dir = TIMER_COUNT_UP;
	config.divider = TIMER_DIVIDER;
	config.intr_type = TIMER_INTR_SEL;
	config.counter_en = TIMER_PAUSE;
	/*Configure timer*/
	timer_init(timer_group, timer_idx, &config);
	/*Stop timer counter*/
	timer_pause(timer_group, timer_idx);
	/*Load counter value */
	timer_set_counter_value(timer_group, timer_idx, 0x00000000ULL);
	/*Set alarm value*/
	timer_set_alarm_value(timer_group, timer_idx, (TIMER_INTERVAL0_SEC * TIMER_SCALE) - TIMER_FINE_ADJ);
	/*Enable timer interrupt*/
	timer_enable_intr(timer_group, timer_idx);
	/*Set ISR handler*/
	timer_isr_register(timer_group, timer_idx, timer_group0_isr, (void*) timer_idx, ESP_INTR_FLAG_IRAM, NULL);
	/*Start timer counter*/
	timer_start(timer_group, timer_idx);
}

void tg0_timer_disable_int(void)
{
	int timer_group = TIMER_GROUP_0;
	int timer_idx = TIMER_0;
	timer_group_intr_disable(timer_group, timer_idx);
}


