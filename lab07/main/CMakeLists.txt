# idf_component_register(SRCS main.c
#                        INCLUDE_DIRS .
#                        PRIV_REQUIRES config)
# # PRIV_REQUIRES ...
# # target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")


idf_component_register(SRCS main.c gameControl.c missile.c tank.c enemy.c
                       INCLUDE_DIRS .
                       PRIV_REQUIRES config esp_timer lcd cursor sound pin)

# If needed, you can still add compile options
# target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")

