#include "sdkconfig.h"


#if CONFIG_EXAMPLE_USE_WS2812

#include "led_strip.h"
#include "LedWS2812.h"
#include "esp_log.h"
#include "esp_err.h"
#include "driver/rmt.h"


#define GPIO_SW     16
#define GPIO_INPUT_PIN_SEL_SW  ((1ULL<<GPIO_SW))

//=========================================== PRIVATE VARIABLE DECLARATION ===========================================
static const char *TAG = "LED_WS2812";
static led_strip_t *strip;
//=====================================================================================================================
//==================================================== DEFINITIONS ====================================================
#define RMT_TX_CHANNEL RMT_CHANNEL_0
#define EXAMPLE_CHASE_SPEED_MS (2)
#define EXAMPLE_CHASE_SPEED_MS_STEP_2 (20)
//=====================================================================================================================


void LedWS2812__Init(void)
{

    rmt_config_t config = RMT_DEFAULT_CONFIG_TX(CONFIG_EXAMPLE_WS2812_DATA_PIN, RMT_TX_CHANNEL);
    // set counter clock to 40MHz
    config.clk_div = 4;

    ESP_ERROR_CHECK(rmt_config(&config));
    ESP_ERROR_CHECK(rmt_driver_install(config.channel, 0, 0));

    // install ws2812 driver
    led_strip_config_t strip_config = LED_STRIP_DEFAULT_CONFIG(CONFIG_EXAMPLE_NUMBER_OF_WS2812_LED, (led_strip_dev_t)config.channel);
    strip = led_strip_new_rmt_ws2812(&strip_config);
    if (!strip) {
        ESP_LOGE(TAG, "install WS2812 driver failed");
    }
    // Clear LED strip (turn off all LEDs)
    ESP_ERROR_CHECK(strip->clear(strip, 100));
    // Show simple rainbow chasing pattern
    ESP_LOGI(TAG, "LED Init Done");

    // LedWS2812__SetPixel(0,1,255,255,255);

    gpio_config_t io_conf = {};
    
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL_SW;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

}

void LedWS2812__SetPixel(uint8_t led_num,bool OnOff_state, uint8_t R, uint8_t G,uint8_t B )
{
    if(OnOff_state)
    {
        ESP_ERROR_CHECK(strip->set_pixel(strip, led_num, R, G, B));
        ESP_ERROR_CHECK(strip->refresh(strip, 5));
    }
    else
    {
        ESP_ERROR_CHECK(strip->set_pixel(strip, led_num, 0x00, 0x00, 0x00));
        ESP_ERROR_CHECK(strip->refresh(strip, 5));
    }
    
    // vTaskDelay(1 / portTICK_RATE_MS); // allow some time before proceeding

}



void led_strip_hsv2rgb(uint32_t h, uint32_t s, uint32_t v, uint32_t *r, uint32_t *g, uint32_t *b)
{
    h %= 360; // h -> [0,360]
    uint32_t rgb_max = v * 2.55f;
    uint32_t rgb_min = rgb_max * (100 - s) / 100.0f;

    uint32_t i = h / 60;
    uint32_t diff = h % 60;

    // RGB adjustment amount by hue
    uint32_t rgb_adj = (rgb_max - rgb_min) * diff / 60;

    switch (i) {
    case 0:
        *r = rgb_max;
        *g = rgb_min + rgb_adj;
        *b = rgb_min;
        break;
    case 1:
        *r = rgb_max - rgb_adj;
        *g = rgb_max;
        *b = rgb_min;
        break;
    case 2:
        *r = rgb_min;
        *g = rgb_max;
        *b = rgb_min + rgb_adj;
        break;
    case 3:
        *r = rgb_min;
        *g = rgb_max - rgb_adj;
        *b = rgb_max;
        break;
    case 4:
        *r = rgb_min + rgb_adj;
        *g = rgb_min;
        *b = rgb_max;
        break;
    default:
        *r = rgb_max;
        *g = rgb_min;
        *b = rgb_max - rgb_adj;
        break;
    }
}

    static uint8_t ct_led = 0;
    static uint32_t counter_val = 0;
    static uint32_t step_counter = 0;
    static bool first_cycle = true;
    static bool second_cycle = false;
    static bool step1 = true;
    static bool step2 = false;
    static bool step3 = false; 

    uint32_t red = 0;
    uint32_t green = 0;
    uint32_t blue = 0;
    uint16_t hue = 0;
    uint16_t start_rgb = 0;     

void led_fashion(void)
{
    
        step_counter ++;
        if(step_counter > 7000)
        {
            // step_counter = 0;
            // step1 = false;
            // step2 = true;
            // step3 = false;

        }
        if(step1 == true)
        {
            counter_val ++;
            for (int i = 0; i < 3; i++) {
            for (int j = i; j < CONFIG_EXAMPLE_STRIP_LED_NUMBER; j += 3) {
                // Build RGB values
                hue = j * 360 / CONFIG_EXAMPLE_STRIP_LED_NUMBER + start_rgb;
                led_strip_hsv2rgb(hue, 100, 100, &red, &green, &blue);
                // Write RGB values to strip driver
                ESP_ERROR_CHECK(strip->set_pixel(strip, j, red, green, blue));

                if(counter_val > 2000)
                {
                    
                    counter_val = 0;
                    ct_led ++;
                    
                    if(ct_led > CONFIG_EXAMPLE_STRIP_LED_NUMBER-1)
                    {
                        ct_led = 0;
                    }
                    // ESP_LOGI(TAG,"Led changed : %d", ct_led);
                    ESP_ERROR_CHECK(strip->set_pixel(strip, ct_led, 0xFF, 0xFF, 0xFF));
                }

                ESP_ERROR_CHECK(strip->set_pixel(strip, ct_led, 0xFF, 0xFF, 0xFF));

                
            }
            // Flush RGB values to LEDs
            ESP_ERROR_CHECK(strip->refresh(strip, 100));
            vTaskDelay(pdMS_TO_TICKS(EXAMPLE_CHASE_SPEED_MS));
            strip->clear(strip, 50);
            vTaskDelay(pdMS_TO_TICKS(EXAMPLE_CHASE_SPEED_MS));
        }
        start_rgb += 60;

        if(first_cycle == true)
        {
            if(counter_val > 150)
            {
                
                counter_val = 0;
                ct_led ++;
                
                if(ct_led > CONFIG_EXAMPLE_STRIP_LED_NUMBER-1)
                {
                    first_cycle = false;
                    second_cycle = true;
                    ct_led = CONFIG_EXAMPLE_STRIP_LED_NUMBER-1;
                }
                // ESP_LOGI(TAG,"Led changed : %d", ct_led);
                // ESP_ERROR_CHECK(strip->set_pixel(strip, ct_led, 0xFF, 0xFF, 0xFF));
            }

        }
        else if(second_cycle == true)
        {
            if(counter_val > 150)
            {
                
                counter_val = 0;
                ct_led --;
                
                if(ct_led < 1)
                {
                    first_cycle = true;
                    second_cycle = false;
                    ct_led = 0;
                }
                // ESP_LOGI(TAG,"Led changed : %d", ct_led);
                // ESP_ERROR_CHECK(strip->set_pixel(strip, ct_led, 0xFF, 0xFF, 0xFF));
            }

        }

        }
        else if(step2 == true)
        {

            ESP_ERROR_CHECK(strip->set_pixel(strip, 0, 0x00, 0x00, 0xFF));
            ESP_ERROR_CHECK(strip->refresh(strip, 100));
            vTaskDelay(500 / portTICK_RATE_MS);
            ESP_ERROR_CHECK(strip->set_pixel(strip, 1, 0xFF, 0x00, 0x00));
            ESP_ERROR_CHECK(strip->refresh(strip, 100));
            vTaskDelay(500 / portTICK_RATE_MS);
            ESP_ERROR_CHECK(strip->set_pixel(strip, 2, 0xFF, 0xFF, 0x00));
            ESP_ERROR_CHECK(strip->refresh(strip, 100));
            vTaskDelay(500 / portTICK_RATE_MS);
            ESP_ERROR_CHECK(strip->set_pixel(strip, 3, 0x00, 0xFF, 0x00));
            ESP_ERROR_CHECK(strip->refresh(strip, 100));
            vTaskDelay(2000 / portTICK_RATE_MS);

            ESP_ERROR_CHECK(strip->set_pixel(strip, 0, 0x00, 0x00, 0x00));
            ESP_ERROR_CHECK(strip->refresh(strip, 100));
            ESP_ERROR_CHECK(strip->set_pixel(strip, 1, 0x00, 0x00, 0x00));
            ESP_ERROR_CHECK(strip->refresh(strip, 100));
            ESP_ERROR_CHECK(strip->set_pixel(strip, 2, 0x00, 0x00, 0x00));
            ESP_ERROR_CHECK(strip->refresh(strip, 100));
            ESP_ERROR_CHECK(strip->set_pixel(strip, 3, 0x00, 0x00, 0x00));
            ESP_ERROR_CHECK(strip->refresh(strip, 100));
            vTaskDelay(500 / portTICK_RATE_MS);


            

        }

}




#endif