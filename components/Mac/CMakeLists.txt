set(srcs "src/Mac.c"
        )

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS include
                    REQUIRES driver Timers)
