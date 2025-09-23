idf_component_register(
    SRCS
        main.cpp
        game_common.cpp
        game_tap_ball.cpp
        game_whack.cpp
        game_memory_grid.cpp
    INCLUDE_DIRS "."
    REQUIRES
        LovyanGFX
        driver
)

target_compile_definitions(${COMPONENT_LIB} PRIVATE GAME_MODE=2 ENABLE_GAME_SWITCH=1)
