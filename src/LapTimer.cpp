#include "LapTimer.h"

// Constructor: Initializes the components with their pins and settings
LapTimer::LapTimer() :
    _sensor(SENSOR_PIN),
    _bootButton(BOOT_BUTTON_PIN, LONG_PRESS_MS),
    _timer(MAX_LAPS_TO_STORE),
    _currentState(State::INITIALIZING) {}

void LapTimer::begin() {
    Serial.begin(115200);
    pinMode(BUILTIN_LED_PIN, OUTPUT);
    digitalWrite(BUILTIN_LED_PIN, LOW);

    // Initialize components
    _sensor.begin();
    _bootButton.begin();

    logHeader("SIMPLE LAP TIMER (SERIAL-ONLY)");
    updateState(State::INITIALIZING);
    delay(1000);

    // Start the mandatory learning process
    updateState(State::LEARNING_CLEAR_WAIT);
    logHeader("STEP 1: LEARN CLEAR BEAM");
    Serial.println("Action: Ensure the sensor beam is CLEAR.");
    Serial.println("Action: Press and HOLD the BOOT button to sample.");
}

void LapTimer::update() {
    // Update all components first
    _bootButton.update(); // Reads button state
    heartbeat();          // Prints status

    // --- Global Long Press to Stop ---
    // This works in any state (except the initial ones)
    if (_bootButton.wasLongPressed() && _currentState > State::LEARNING_BROKEN_WAIT) {
        Serial.println("Long press detected: STOPPING RACE.");
        finishRaceSession();
    }

    // --- Main State Machine ---
    // Call the specific function for the current state
    switch (_currentState) {
        case State::LEARNING_CLEAR_WAIT:
            handleLearningClear();
            break;
        case State::LEARNING_BROKEN_WAIT:
            handleLearningBroken();
            break;
        case State::WAITING_FOR_START:
            handleWaitingToStart();
            break;
        case State::PENDING_START_CONFIRM:
            handlePendingStartConfirm();
            break;
        case State::TIMING_LAP:
            handleTimingLap();
            break;

        case State::INITIALIZING:
        case State::RACE_FINISHED:
        default:
            // Do nothing, wait for user input or reset
            break;
    }
}

// --- State Handlers ---

/**
 * @brief Handles the "Learning Clear" state.
 */
void LapTimer::handleLearningClear() {
    if (_bootButton.isPressed()) {
        if (!_isCalibrating) {
            // Button was just pressed, start sampling
            _isCalibrating = true;
            _sensor.startCalibration();
            Serial.println("Sampling... KEEP HOLDING button...");
        }
        // Blink LED and add a sample
        digitalWrite(BUILTIN_LED_PIN, HIGH);
        _sensor.addCalibrationSample();
        delay(20); // Small delay between samples
        digitalWrite(BUILTIN_LED_PIN, LOW);
        delay(20);

    } else if (_isCalibrating) {
        // Button was just released, finish calibration
        _isCalibrating = false;
        digitalWrite(BUILTIN_LED_PIN, LOW);

        if (_sensor.finishCalibrationClear()) {
            logHeader("STEP 2: LEARN BROKEN BEAM");
            Serial.printf("Clear Beam reading accepted: %d\n", _sensor.getClearReading());
            Serial.println("Action: Now place the robot to BREAK the beam.");
            Serial.println("Action: Press and HOLD the BOOT button to sample.");
            updateState(State::LEARNING_BROKEN_WAIT);
        } else {
            logHeader("ERROR: INVALID CLEAR READING");
            Serial.printf("Average value was %d. Must be >= 1000 and <= 4095.\n", _sensor.getClearReading());
            Serial.println("Please fix the sensor (distance/screw) and try again.");
        }
    }
}

/**
 * @brief Handles the "Learning Broken" state.
 */
void LapTimer::handleLearningBroken() {
    if (_bootButton.isPressed()) {
        if (!_isCalibrating) {
            _isCalibrating = true;
            _sensor.startCalibration();
            Serial.println("Sampling... KEEP HOLDING button...");
        }
        digitalWrite(BUILTIN_LED_PIN, HIGH);
        _sensor.addCalibrationSample();
        delay(20);
        digitalWrite(BUILTIN_LED_PIN, LOW);
        delay(20);

    } else if (_isCalibrating) {
        _isCalibrating = false;
        digitalWrite(BUILTIN_LED_PIN, LOW);

        if (_sensor.finishCalibrationBroken()) {
            _sensor.calculateThreshold(); // Finalize threshold
            logHeader("LEARNING COMPLETE");
            Serial.printf("Broken Beam reading accepted: %d\n", _sensor.getBrokenReading());
            Serial.printf("Calculated Line Threshold: < %d\n", _sensor.getThreshold());
            Serial.println("System active. Waiting for robot...");
            updateState(State::WAITING_FOR_START);
        } else {
            logHeader("ERROR: INVALID BROKEN READING");
            Serial.printf("Average value was %d. Must be >= 0 and < 1000.\n", _sensor.getBrokenReading());
            Serial.println("Please fix the sensor (distance/screw) and try again.");
        }
    }
}

/**
 * @brief Looks for the first line cross to start the race.
 */
void LapTimer::handleWaitingToStart() {
    unsigned long currentTime = millis();
    // This state logic remains the same: a quick break triggers the start confirmation
    if (_sensor.isBeamBroken() && (currentTime - _lastLineEventTime > DEBOUNCE_MS)) {
        _lastLineEventTime = currentTime;
        digitalWrite(BUILTIN_LED_PIN, HIGH); delay(50); digitalWrite(BUILTIN_LED_PIN, LOW);
        
        updateState(State::PENDING_START_CONFIRM);
        Serial.println("\nACTION: Start line detected. Press BOOT to start race.");
    }
}

/**
 * @brief Waits for the user to press BOOT to confirm the race start.
 */
void LapTimer::handlePendingStartConfirm() {
    if (_bootButton.wasShortPressed()) {
        logHeader("RACE START CONFIRMED");
        _timer.start(); // Start the race timer
        _lastLineEventTime = millis(); // Initialize cooldown timer
        updateState(State::TIMING_LAP);
        Serial.printf("Race Started! Lap %d underway.\n", _timer.getLapCount());
    }
}

/**
 * @brief The main race state, looks for line crosses to record laps.
 */
void LapTimer::handleTimingLap() {
    unsigned long currentTime = millis();
    bool isBroken = _sensor.isBeamBroken();

    if (isBroken) {
        // --- Beam is currently broken ---
        if (!_isObstacleDetected) {
            // ** EVENT: BEAM WAS JUST BROKEN **
            // Check if this is a "new" event (not part of the last cooldown)
            if (currentTime - _lastLineEventTime > DEBOUNCE_MS) {
                _isObstacleDetected = true;
                _obstacleStartTime = currentTime;
                digitalWrite(BUILTIN_LED_PIN, HIGH); // Turn on LED
                Serial.println("Obstacle detected, timing...");
            }
            // If it's *within* the cooldown, we do nothing.
        }
        // If _isObstacleDetected is already true, we just wait.
        
    } else {
        // --- Beam is currently clear ---
        if (_isObstacleDetected) {
            // ** EVENT: BEAM WAS JUST CLEARED **
            // This is the *end* of the lap.
            _isObstacleDetected = false;
            _lastLineEventTime = currentTime; // Start cooldown *now*

            digitalWrite(BUILTIN_LED_PIN, LOW); // Turn off LED

            // 1. Calculate obstacle time
            unsigned long obstacleTime = currentTime - _obstacleStartTime;

            // 2. Record the lap
            unsigned long lapTime = _timer.recordLap(obstacleTime);

            Serial.printf("LAP %d FINISHED! Lap Time: %.3fs | Obstacle Time: %.3fs (Sensor: %d)\n",
                          _timer.getLapCount() - 1,
                          lapTime / 1000.0,
                          obstacleTime / 1000.0,
                          _sensor.getLastReadValue());

            if (_timer.isHistoryFull()) {
                Serial.println("WARN: Max lap storage reached.");
            }
            Serial.printf("Starting Lap %d...\n", _timer.getLapCount());
        }
        // If beam is clear and _isObstacleDetected is false, we do nothing.
    }
}

// --- Helper Functions ---

void LapTimer::finishRaceSession() {
    if (_currentState == State::RACE_FINISHED) return; // Already finished

    logHeader("RACE SESSION FINISHED");
    _timer.stop();
    updateState(State::RACE_FINISHED);
    _timer.printSummary(); // Ask the timer to print its summary
}

void LapTimer::heartbeat() {
    unsigned long currentTime = millis();
    if (currentTime - _lastHeartbeatTime > HEARTBEAT_MS) {
        _lastHeartbeatTime = currentTime;
        int currentReading = _sensor.read(); // Read sensor value

        Serial.print("Timer: ");
        if (_currentState == State::TIMING_LAP) {
             Serial.printf("%.1fs", _timer.getCurrentLapTime() / 1000.0);
             if (_isObstacleDetected) {
                Serial.printf(" (Obstacle: %.1fs)", (currentTime - _obstacleStartTime) / 1000.0);
             }
        } else if (_currentState == State::PENDING_START_CONFIRM) {
             Serial.print("Confirm Start");
        } else if (_currentState == State::RACE_FINISHED) {
             Serial.print("Finished");
        // *** FIX: Was 'currentState', now '_currentState' ***
        } else if (_currentState == State::LEARNING_CLEAR_WAIT || _currentState == State::LEARNING_BROKEN_WAIT) {
             Serial.print("Learning...");
        } else { // Waiting or Initializing
             Serial.print("0.0s");
        }
         Serial.printf(" | Sensor: %d\n", currentReading);
    }
}

void LapTimer::updateState(State newState) {
    _currentState = newState;
    const char* statusString = "Unknown";
    // *** FIX: Was 'currentState', now '_currentState' ***
    switch (_currentState) {
        case State::INITIALIZING:       statusString = "Initializing"; break;
        case State::LEARNING_CLEAR_WAIT: statusString = "Learning Clear"; break;
        case State::LEARNING_BROKEN_WAIT: statusString = "Learning Broken"; break;
        case State::WAITING_FOR_START:  statusString = "Ready"; break;
        case State::PENDING_START_CONFIRM: statusString = "Confirm Start"; break;
        case State::TIMING_LAP:         statusString = "Timing Lap"; break;
        case State::RACE_FINISHED:      statusString = "Finished"; break;
    }
    Serial.printf("Status: %s\n", statusString);
}

void LapTimer::logHeader(const char* title) {
    Serial.println();
    Serial.println("========================================");
    Serial.printf("  %s\n", title);
    Serial.println("========================================");
}