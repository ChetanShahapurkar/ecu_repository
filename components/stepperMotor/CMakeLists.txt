set(srcs "src/A4988.c"
         "src/StepperMotorEngine.c"
         "src/StepperMotorManager.c"
        )

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS include
                    REQUIRES driver Timers)
