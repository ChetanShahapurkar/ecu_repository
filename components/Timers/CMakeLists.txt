set(srcs "src/Timers.c"
         "src/TimerInterrupt.c"
		 
         )

idf_component_register(SRCS "${srcs}"
                    REQUIRES driver
                    INCLUDE_DIRS include)
