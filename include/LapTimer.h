#pragma once

#include <Arduino.h>
#include "Sensor.h"
#include "Button.h"
#include "RaceTimer.h"

// --- Configuration ---
// (All configuration constants are now part of this class)
constexpr int SENSOR_PIN = 32;       // Use a safe ADC1 pin (like 32, 33, 34, 35)
constexpr int BOOT_BUTTON_PIN = 0;   // The built-in BOOT button
constexpr int BUILTIN_LED_PIN = 2;   // The built-in LED
constexpr int DEBOUNCE_MS = 1000;    // 1-second cooldown after a line event
constexpr int LONG_PRESS_MS = 2000;  // 2 seconds for a long press to stop
constexpr int MAX_LAPS_TO_STORE = 50;
constexpr int HEARTBEAT_MS = 1000; // How often to print status

class LapTimer {
public:
    LapTimer();
    void begin();
    void update();

private:
    // --- State Machine ---
    enum class State {
        INITIALIZING,
        LEARNING_CLEAR_WAIT,
        LEARNING_BROKEN_WAIT,
        WAITING_FOR_START,
        PENDING_START_CONFIRM,
        TIMING_LAP,
        RACE_FINISHED
    };
    State _currentState;

    // --- Core Components ---
    // These objects manage their own logic
    Sensor _sensor;
    Button _bootButton;
    RaceTimer _timer;

    // --- State-specific Handlers ---
    // The main loop calls the correct handler based on the state
    void handleLearningClear();
    void handleLearningBroken();
    void handleWaitingToStart();
    void handlePendingStartConfirm();
    void handleTimingLap();

    // --- Internal Helpers ---
    void updateState(State newState);
    void logHeader(const char* title);
    void heartbeat();
    void finishRaceSession();

    // --- Internal State Variables ---
    bool _isCalibrating = false; // True while user is holding button to sample
    unsigned long _lastLineEventTime = 0;
    unsigned long _lastHeartbeatTime = 0;
    
    // *** ADDED: New state variables for obstacle timing ***
    bool _isObstacleDetected = false; 
    unsigned long _obstacleStartTime = 0;
};