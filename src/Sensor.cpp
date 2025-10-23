#include "Sensor.h"

Sensor::Sensor(uint8_t pin) : _pin(pin) {}

void Sensor::begin() {
    pinMode(_pin, INPUT);
}

int Sensor::read() {
    _lastReadValue = analogRead(_pin);
    return _lastReadValue;
}

bool Sensor::isBeamBroken() {
    // We read the value *once* and compare it
    return (read() < _threshold);
}

void Sensor::startCalibration() {
    _calibrationSum = 0;
    _calibrationCount = 0;
}

void Sensor::addCalibrationSample() {
    _calibrationSum += read();
    _calibrationCount++;
}

bool Sensor::finishCalibrationClear() {
    if (_calibrationCount == 0) return false;
    _clearReading = _calibrationSum / _calibrationCount;
    
    // *** FIX: Allow 4095 and use inclusive >= 1000 ***
    return (_clearReading >= 1000 && _clearReading <= 4095);
}

bool Sensor::finishCalibrationBroken() {
    if (_calibrationCount == 0) return false;
    _brokenReading = _calibrationSum / _calibrationCount;

    // *** FIX: Allow 0 and use exclusive < 1000 ***
    return (_brokenReading >= 0 && _brokenReading < 1000);
}

void Sensor::calculateThreshold() {
    if (_clearReading > _brokenReading) { // Added safety check
        _threshold = (_clearReading + _brokenReading) / 2;
    } else {
        // Failsafe in case something went wrong
        _threshold = _clearReading - 100; 
    }
}

// --- Getters ---
int Sensor::getClearReading() const { return _clearReading; }
int Sensor::getBrokenReading() const { return _brokenReading; }
int Sensor::getThreshold() const { return _threshold; }
int Sensor::getLastReadValue() const { return _lastReadValue; }