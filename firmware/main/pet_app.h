#pragma once

// Starts the pet's local behaviour (T04) in its own task: gestures, study
// timer, sound reaction, short sounds, telemetry and the session marker.
// It runs without network; T03 will add the API on top of it.
void pet_app_start(void);
