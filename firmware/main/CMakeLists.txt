idf_component_register(SRCS "main.c" "pet_app.c" "study_session.c" "gestures.c" "pet_state.c"
                            "sound_reaction.c" "telemetry.c" "settings.c"
                            "sensors.c" "pin_check.c" "audio.c" "sounds_pcm.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES esp_system esp_hw_support esp_timer spi_flash nvs_flash
                                     esp_driver_gpio esp_driver_i2s)
