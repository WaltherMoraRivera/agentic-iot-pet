#pragma once

// Each file registers its tests with RUN_TEST inside these functions;
// app_main in test_pet_state.c calls them between UNITY_BEGIN and UNITY_END.
void run_sound_reaction_tests(void);
void run_telemetry_tests(void);
void run_settings_tests(void);
