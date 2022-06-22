

#include "A4988.h"
#include "esp_log.h"
#include "../../C_Extensions.h"



//=========================================== PRIVATE VARIABLE DECLARATION ===========================================
static const char *TAG = "A4988";

//=====================================================================================================================
//==================================================== DEFINITIONS ====================================================
#define MOTOR_DIR    18
#define MOTOR_STEP   19
#define MOTOR_ENABLE   17
#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<MOTOR_DIR) | (1ULL<<MOTOR_STEP))
//=====================================================================================================================


void A4988__Init(void)
{

    //zero-initialize the config structure.
    gpio_config_t io_conf = {};
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 1;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
    printf("\nStpper motor inited Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());


}

void A4988__StepPulse(bool pulse)
{
    gpio_set_level(MOTOR_STEP, pulse);
}

void A4988__StepPulseHIGH(void)
{
    gpio_set_level(MOTOR_STEP, 1);
}

void A4988__StepPulseLOW(void)
{
    gpio_set_level(MOTOR_STEP, 0);
}

void A4988__SetDir(bool dir)
{
    gpio_set_level(MOTOR_DIR, dir);
}