set(COMPONENT_ADD_INCLUDEDIRS . "include")

set(COMPONENT_SRCS
        "screen.cpp"
        "screen_driver.cpp"
        "screen_driver_ssd1306.cpp"
        )


set(COMPONENT_REQUIRES brain-common)

register_component()
