#include "sdkconfig.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "system.h"
#include "driver/rmt.h"
#include "led_strip.h"


#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <sys/param.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_spiffs.h"
#include "esp_netif.h"
#include "esp_err.h"
#include "esp_vfs_fat.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"
#include "sdkconfig.h"
#include "driver/sdspi_host.h"
#include "driver/spi_common.h"
#include "sdmmc_cmd.h"
#include "soc/soc_caps.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "driver/uart.h"
#include "string.h"
#include "Timers.h"
#include "BleManager.h"
#include "EspBle.h"

#include "main.h"
static const char *TAG = "MAIN";


//A constant pointer to a function with no return value or parameters.
typedef void (* const TASK_TYPE)(void);

//A constant pointer to a list of TASK_TYPESs.
typedef void (* const * const TASK_LIST_TYPE)(void);

//A NULL pointer to use as a terminator for function lists.
#define NULL_TASK ((TASK_TYPE)0)

#include "main_prv.h"


static uint8_t Main_Slot;
//!Number of the current interrupt slot in the timer interrupt service routine
static uint8_t Interrupt_Slot;
//!Timer to measure a single slot duration
static uint8_t Main_Slot_Timer;
//!Value of Main_Slot_Timer the last time a Slot was executed
static uint8_t Main_Slot_Timer_Diff;
//!Number of slots in the main loop
#define MAIN_SLOT_COUNT (sizeof(Main_Tasks)/sizeof(TASK_LIST_TYPE))
//!Number of slots in the main interrupt
#define INTERRUPT_SLOT_COUNT (sizeof(Interrupt_Tasks)/sizeof(TASK_LIST_TYPE))

#define SLOT_TIME (INTERRUPTS_PER_MILLISECOND * MILLISECONDS_PER_MAIN_SLOT)

//=========================================== PRIVATE (Function Prototypes) ===========================================
static void ExecuteTasks(TASK_TYPE * task_list);
//=====================================================================================================================


void app_main(void)
{
 
    //  --- Initialize program

	// Initialize main() routine variables.
	Interrupt_Slot = INTERRUPT_SLOT_COUNT - 1;      // Execute interrupt slot 0 first
	Main_Slot = MAIN_SLOT_COUNT - 1;                // Execute main slot 0 first
	Main_Slot_Timer = SLOT_TIME;                    // Timer for main slots
	Main_Slot_Timer_Diff = 0;                       // Provides mutual exclusion with the timer interrupt


	//Initialize the Micro-controller and application modules
	ExecuteTasks(Initialization_Tasks);

	/*Allow some time for WiFi / BLE operations to finish*/
	vTaskDelay(1000/portTICK_PERIOD_MS);
    
    tg0_timer0_init();
    ESP_LOGI(TAG, "SYSTEM Initialized");

    

    for(;;)
    {


        #if (EXECUTE_EVERY_MAIN_LOOP_TASKS == ENABLED)
            // Execute tasks that run every main loop
            ExecuteTasks(Every_Loop_Tasks);
        #endif

        // Time Gate for Slotted Architecture
        // Implementation catches up if it falls behind
        if ((uint8_t)(Main_Slot_Timer - Main_Slot_Timer_Diff) >= SLOT_TIME)
        {
            // '+=' operator used here so that overruns are eventually caught up
            Main_Slot_Timer_Diff += SLOT_TIME;

            // Execute tasks that run every slot
            ExecuteTasks(Every_Slot_Tasks);


            // Move to the next main loop Slot.  Roll back to zero if necessary.
            Main_Slot++;
            if (Main_Slot >= MAIN_SLOT_COUNT)
            {
                Main_Slot = 0;
            }

            // Execute tasks that only run during a specific slot
            ExecuteTasks(Main_Tasks[Main_Slot]);

            // Tests if the Slot Time was respected
            if ((uint8_t)(Main_Slot_Timer - Main_Slot_Timer_Diff) >= SLOT_TIME)
            {
                // Exception: Main Slot Time Violation
                EXCEPTION_MAIN_SLOT_VIOLATION((Main_Slot_Timer - Main_Slot_Timer_Diff),SLOT_TIME);

            }

        }
        else
        {

            //The code associated to NOISE_IMMUNITY runs between slots.
            //Ensure that the code is not too long that could cause problems to the slots.
            //NOISE_IMMUNITY();
        }


        // vTaskDelay(1);


    }


    // xTaskCreate(&system__Handler, "system", 1024*5 ,NULL,5,NULL );
    // vTaskDelay(1000 / portTICK_RATE_MS);

}



//---------------------------------------------------------------------------------------------------------------------
/**
 *  @brief      Interrupt handler for the timer interrupt.
 *
 *  @details    This routine implements an interrupt service routine with a gated slotted
 *              architecture.  Each interrupt calls the routines for the next interrupt slot.
 *              It can be configured through the parameter files.
 */
void Main__TimerISR( void )
{


#if (EXECUTE_EVERY_INTERRUPT_TASKS == ENABLED)
	ExecuteTasks(Every_Interrupt_Tasks);
#endif                                          // EVERY_INTERRUPT_TASKS_ENABLED

	// Move to the next interrupt Slot.  Roll back to zero if necessary.
	Interrupt_Slot++;
	if (Interrupt_Slot >= INTERRUPT_SLOT_COUNT)
	{
		Interrupt_Slot = 0;
	}

	ExecuteTasks(Interrupt_Tasks[Interrupt_Slot]);

	Main_Slot_Timer++;

}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief  This method reinitializes all the counters for the slots.
 * @details This method is required to resynchronize the system if it's needed.
 */
void Main__TimeSlotInitialize(void)
{
	Interrupt_Slot = INTERRUPT_SLOT_COUNT - 1;      // Execute interrupt slot 0 first
	Main_Slot = MAIN_SLOT_COUNT - 1;                // Execute main slot 0 first
	Main_Slot_Timer = SLOT_TIME;                    // Timer for main slots
	Main_Slot_Timer_Diff = 0;                       // Provides mutual exclusion with the timer interrupt
}

//=====================================================================================================================
//-------------------------------------- Private Functions ------------------------------------------------------------
//=====================================================================================================================
//---------------------------------------------------------------------------------------------------------------------
/**
 *  @brief      Execute the list of tasks pointed to by task_list.
 *  @details    This code is approximately a 33% improvement in execution time and a 44%
 *              improvement in code size over a version that uses an index into the task_list
 *              array.  Former code shown here.
 *              <pre>
 *                  void ExecuteTasks(TASK_TYPE task_list[])
 *                  {
 *                      uint8 task;
 *
 *                      for (task=0; task_list[task] != NULL_TASK; task++)
 *                      {
 *                          task_list[task]();
 *                      }
 *                  }
 *              </pre>
 *
 *  @param      task_list = Null-terminated list of tasks to execute.
 *                          Each task must have the prototype: void Task(void)
 */
static void ExecuteTasks(TASK_TYPE * task_list)
{
	while (*task_list != NULL_TASK)
	{
		(*task_list)();
		task_list++;
	}
}


