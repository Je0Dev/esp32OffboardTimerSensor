#include "Button.h"

Button::Button(uint8_t pin, unsigned long longPressMs) :
    _pin(pin),
    _longPressMs(longPressMs) {}

void Button::begin() {
    pinMode(_pin, INPUT_PULLUP);
    _lastState = digitalRead(_pin);
}

// This function (adapted from your original) handles all button state logic
void Button::update() {
    // Reset flags at the start of each update
    _shortPressFlag = false;
    _longPressFlag = false;

    bool currentState = (digitalRead(_pin) == LOW);
    unsigned long currentTime = millis();

    if (currentState) {
        // --- Button is currently being HELD down ---
        if (_pressTime == 0) { // This is the first moment it was pressed
            _pressTime = currentTime;
            _longPressTriggered = false;
        }

        // Check if it's a long press (and not already triggered)
        if (!_longPressTriggered && (currentTime - _pressTime > _longPressMs)) {
            _longPressTriggered = true;
            _longPressFlag = true; // Set the flag
        }
    } else {
        // --- Button has been RELEASED ---
        if (_pressTime > 0 && !_longPressTriggered) {
            // This was a SHORT press.
            _shortPressFlag = true; // Set the flag
        }
        // Reset the button press tracker
        _pressTime = 0;
    }
}

bool Button::isPressed() {
    return (_pressTime > 0);
}

bool Button::wasShortPressed() {
    return _shortPressFlag;
}

bool Button::wasLongPressed() {
    return _longPressFlag;
}