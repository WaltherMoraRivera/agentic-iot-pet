idf_component_register(SRCS "main.c" "bench_pins.c" "test_touch_led.c" "test_i2c_oled.c"
                            "test_mic.c" "test_amp.c"
                       PRIV_REQUIRES esp_driver_gpio esp_driver_i2c esp_driver_i2s
                                     esp_system esp_hw_support esp_timer spi_flash)
