/**
 * Author: mastroloko
 * A simple, clean, Serial-only lap timer for an offboard IR sensor.
 *
 * This main.cpp file simply creates and runs the main LapTimer application.
 * All logic is handled in the LapTimer class and its components.
 */

#include <Arduino.h>
#include "LapTimer.h"

// Create the one and only LapTimer object
LapTimer timer;

void setup() {
    // Initialize the timer application
    timer.begin();
}

void loop() {
    // Run the timer's main update loop
    timer.update();
}