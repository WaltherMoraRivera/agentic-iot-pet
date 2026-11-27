idf_component_register(SRCS "main.c" "study_session.c" "gestures.c" "pet_state.c"
                            "sound_reaction.c" "telemetry.c" "settings.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES esp_system esp_hw_support esp_timer spi_flash nvs_flash)
