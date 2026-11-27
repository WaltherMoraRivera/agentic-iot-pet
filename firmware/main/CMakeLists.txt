idf_component_register(SRCS "main.c" "study_session.c" "gestures.c" "pet_state.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES esp_system esp_hw_support esp_timer spi_flash)
