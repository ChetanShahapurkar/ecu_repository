set(srcs "src/BleManager.c"
         "src/BleEngine.c"
         "src/EspBle.c"
         )

idf_component_register(SRCS "${srcs}"
                    REQUIRES spi_flash mbedtls Timers nvs_flash bt system
                    INCLUDE_DIRS include
                    PRIV_REQUIRES )
