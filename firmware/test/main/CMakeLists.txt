# The logic under test lives in the product firmware (firmware/main) and is
# compiled here from the same files, not from copies.
idf_component_register(SRCS "test_pet_state.c"
                            "../../main/study_session.c"
                            "../../main/gestures.c"
                            "../../main/pet_state.c"
                       INCLUDE_DIRS "../../main"
                       PRIV_REQUIRES unity esp_timer newlib)
