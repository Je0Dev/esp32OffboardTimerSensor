#pragma once
#include <Arduino.h>

class Sensor {
public:
    Sensor(uint8_t pin);
    void begin();
    int read();
    bool isBeamBroken();

    // --- New Calibration Functions ---
    void startCalibration();         // Resets the averaging calculation
    void addCalibrationSample();     // Adds one reading to the average
    bool finishCalibrationClear();   // Finalizes "clear" reading
    bool finishCalibrationBroken();  // Finalizes "broken" reading
    void calculateThreshold();       // Calculates the midpoint

    // --- Getters ---
    int getClearReading() const;
    int getBrokenReading() const;
    int getThreshold() const;
    int getLastReadValue() const;

private:
    uint8_t _pin;
    int _clearReading = 0;
    int _brokenReading = 0;
    int _threshold = 0;
    int _lastReadValue = 0; // Store the last read value

    // --- Calibration state ---
    unsigned long _calibrationSum = 0;
    int _calibrationCount = 0;
};