set(srcs "src/CurrentSense.c"
         "src/CurrentSenseManager.c"

        )

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS include
                    REQUIRES driver Timers esp_adc_cal)
