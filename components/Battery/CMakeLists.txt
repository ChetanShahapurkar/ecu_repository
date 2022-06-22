set(srcs "src/Battery.c"
         "src/BatteryManager.c"

        )

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS include
                    REQUIRES driver Timers)
