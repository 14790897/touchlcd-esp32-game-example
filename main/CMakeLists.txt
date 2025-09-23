idf_component_register(
    SRCS
        main.cpp
        game_common.cpp
        game_tap_ball.cpp
        game_whack.cpp
    INCLUDE_DIRS "."
    REQUIRES
        LovyanGFX
        driver
)
