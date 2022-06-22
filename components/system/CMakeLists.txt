set(srcs "src/system.c"
		 
         )

idf_component_register(SRCS "${srcs}"
                    REQUIRES spi_flash mbedtls GsmGprs stepperMotor Led Ble Mac SdCard As2315 Battery CurrentSense
                    INCLUDE_DIRS include)
