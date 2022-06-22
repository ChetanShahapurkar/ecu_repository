set(srcs "src/GsmGprsEngine.c"
         "src/GsmGprsManager.c"
         "src/esp_modem.c"
         "src/esp_modem_dce_service"
         "src/esp_modem_netif.c"
         "src/esp_modem_compat.c"
         "src/sim800.c"
         "src/sim7600.c"
         "src/sim900A.c"
         "src/bg96.c"
         "src/LibGSM.c"
		 
         )

idf_component_register(SRCS "${srcs}"
                    REQUIRES mqtt json SdCard Ble system
                    INCLUDE_DIRS include)
