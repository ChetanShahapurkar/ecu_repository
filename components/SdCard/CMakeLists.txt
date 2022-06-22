set(srcs "src/SdCard.c"
         "src/ftp.c"
        )

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS include
                    REQUIRES driver Timers fatfs mdns nvs_flash system)
