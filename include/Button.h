#pragma once
#include <Arduino.h>

class Button {
public:
    // Constructor: specify the pin and long-press duration
    Button(uint8_t pin, unsigned long longPressMs = 2000);
    
    void begin();
    void update(); // Must be called every loop

    bool isPressed();       // Is the button currently held down?
    bool wasShortPressed(); // Was the button just released after a short press?
    bool wasLongPressed();  // Was a long press just detected?

private:
    uint8_t _pin;
    unsigned long _longPressMs;

    // Internal state variables
    byte _lastState = HIGH;
    unsigned long _pressTime = 0;
    bool _longPressTriggered = false;
    
    // "Flags" that get set for one loop cycle
    bool _shortPressFlag = false;
    bool _longPressFlag = false;
};