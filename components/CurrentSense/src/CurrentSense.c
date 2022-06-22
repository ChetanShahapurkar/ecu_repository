/* ADC1 Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   64          //Multisampling

#define SENSOR_VP ADC_CHANNEL_0     //IO36
#define SENSOR_VN ADC_CHANNEL_3     //IO39
#define SENSOR_EXTRA ADC_CHANNEL_6     //IO34

static esp_adc_cal_characteristics_t *adc_chars;

// static const adc_channel_t channel = ADC_CHANNEL_0;    
static const adc_bits_width_t width = ADC_WIDTH_BIT_12;

static const adc_atten_t atten = ADC_ATTEN_DB_0;
static const adc_unit_t unit = ADC_UNIT_1;


static void check_efuse(void)
{
#if CONFIG_IDF_TARGET_ESP32
    //Check if TP is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
        printf("eFuse Two Point: Supported\n");
    } else {
        printf("eFuse Two Point: NOT supported\n");
    }
    //Check Vref is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK) {
        printf("eFuse Vref: Supported\n");
    } else {
        printf("eFuse Vref: NOT supported\n");
    }
#elif CONFIG_IDF_TARGET_ESP32S2
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
        printf("eFuse Two Point: Supported\n");
    } else {
        printf("Cannot retrieve eFuse Two Point calibration values. Default calibration values will be used.\n");
    }
#else
#error "This example is configured for ESP32/ESP32S2."
#endif
}


static void print_char_val_type(esp_adc_cal_value_t val_type)
{
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        printf("Characterized using Two Point Value\n");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        printf("Characterized using eFuse Vref\n");
    } else {
        printf("Characterized using Default Vref\n");
    }
}


void CurrentSense__init(void)
{
    //Check if Two Point or Vref are burned into eFuse
    check_efuse();

    //Configure ADC
    if (unit == ADC_UNIT_1) 
    {
        adc1_config_width(width);
        adc1_config_channel_atten(SENSOR_VP, atten);
        adc1_config_channel_atten(SENSOR_VN, atten);
        adc1_config_channel_atten(SENSOR_EXTRA, atten);
    } 
    // else 
    // {
    //     adc2_config_channel_atten((adc2_channel_t)channel, atten);
    // }

    //Characterize ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, width, DEFAULT_VREF, adc_chars);
    print_char_val_type(val_type);

    //Continuously sample ADC1

}

void CurrentSense_Handler(void)
{
    
    // while (1) 
    {
        uint32_t adc_reading = 0;
        uint32_t adc_reading2 = 0;
        uint32_t adc_reading3 = 0;
        //Multisampling
        for (int i = 0; i < NO_OF_SAMPLES; i++) 
        {
            if (unit == ADC_UNIT_1) {
                adc_reading += adc1_get_raw((adc1_channel_t)SENSOR_VP);
                adc_reading2 += adc1_get_raw((adc1_channel_t)SENSOR_VN);
                adc_reading3 += adc1_get_raw((adc1_channel_t)SENSOR_EXTRA);
            } 
            // else 
            // {
            //     int raw;
            //     adc2_get_raw((adc2_channel_t)channel, width, &raw);
            //     adc_reading += raw;
            // }
        }
        adc_reading /= NO_OF_SAMPLES;
        adc_reading2 /= NO_OF_SAMPLES;
        adc_reading3 /= NO_OF_SAMPLES;
        //Convert adc_reading to voltage in mV
        uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
        // printf("Raw: %d\tVoltage: %dmV\n", adc_reading, voltage);

        uint32_t voltage2 = esp_adc_cal_raw_to_voltage(adc_reading2, adc_chars);
        // printf("Raw: %d\tVoltage: %dmV\n", adc_reading, voltage);

        uint32_t voltage3 = esp_adc_cal_raw_to_voltage(adc_reading3, adc_chars);
        // printf("Raw: %d\tVoltage: %dmV\n", adc_reading, voltage);

        printf("Raw: %d\tVoltage: %dmV\t  Raw1: %d\tVoltage1: %dmV  Raw2: %d\tVoltage2: %dmV\n", adc_reading, voltage, adc_reading2, voltage2, adc_reading3, voltage3);
        // vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
