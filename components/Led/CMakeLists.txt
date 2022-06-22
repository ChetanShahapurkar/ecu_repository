set(srcs "src/LedManager.c"
         "src/LedEngine.c"
         "src/LedWS2812.c"
		 
         )

idf_component_register(SRCS "${srcs}"
                    REQUIRES spi_flash mbedtls driver Timers
                    INCLUDE_DIRS include
                    PRIV_REQUIRES led_strip)
