# ESP32 IR Lap Timer (Serial-Only)

This is a simple, clean, and maintainable lap timer for an ESP32 using a single IR (infrared) sensor. It is designed to be highly robust, with no display required. The entire "UI" is handled through the built-in BOOT button and the Arduino Serial Monitor.

This project is built using C++ with an object-oriented, multi-file structure for maximum maintainability and readability.

## 🏁 Key Features

* **State Machine Logic:** The timer operates on a clean, easy-to-understand state machine.
* **Robust Sensor Calibration:** A guided, multi-sample averaging process ensures the sensor threshold is set reliably, even with noisy sensors. It supports the full `0` (broken) to `4095` (clear) ADC range.
* **Intelligent Lap Timing:** A lap is not just a simple "tripwire." It is defined as the time from the lap start until the moment an obstacle **clears** the beam, providing a more accurate "finish line" behavior.
* **Obstacle Time:** The timer *also* records how long the beam was broken, giving you a new data point for analysis.
* **Hardware Feedback:** The built-in LED turns ON when the beam is broken and OFF when it is cleared.
* **Clean Code:** Logic is encapsulated into separate classes (`LapTimer`, `Sensor`, `Button`, `RaceTimer`) to make the code easy to read and extend.
* **Race Summary:** Automatically prints a formatted table of all completed laps, total time, and obstacle time.

---

## Hardware Setup

You only need an ESP32 and a 3-pin IR sensor module.

* **IR Sensor (Analog Out)**: `Sensor A0` -> `ESP32 GPIO 32`
    * *(This must be a safe ADC1 pin, like 32, 33, 34, 35, 36, or 39)*
* **BOOT Button**: `GPIO 0` (Built-in to most ESP32 dev boards)
* **LED**: `GPIO 2` (The built-in blue LED)

---

## 🚀 How it Works: The User's Guide

This entire project is controlled using the Serial Monitor and the `BOOT` button.

### 1. Open the Serial Monitor

* Connect your ESP32, build, and upload the code.
* Open the Serial Monitor at **115200 baud**. You will be greeted with the first step.

### 2. Step 1: Learn "Clear Beam"

The timer must first learn what a clear, unbroken beam looks like.

* **Action:** Ensure the sensor beam is **CLEAR** (no robot in the way).
* **Action:** **Press and HOLD** the `BOOT` button.
* The blue LED will blink rapidly as it takes multiple samples.
* **Action:** **Release** the button after 1-2 seconds.
* If the reading is valid (e.g., `>= 1000`), it will be accepted, and you'll move to the next step.

### 3. Step 2: Learn "Broken Beam"

Now, the timer needs to learn what an obstacle looks like.

* **Action:** Place your robot or hand to **BREAK** the sensor beam.
* **Action:** **Press and HOLD** the `BOOT` button.
* The blue LED will blink as it samples the "broken" value.
* **Action:** **Release** the button.
* If the reading is valid (e.g., `< 1000`), the system will calculate the midpoint threshold and arm the timer.

### 4. Step 3: Start the Race

The system is now active and "Waiting for robot..."

* **Action:** Let the robot cross the line.
* The timer will detect the *first* line cross and enter a "Pending Start" state.
* The Serial Monitor will display: `ACTION: Start line detected. Press BOOT to start race.`
* **Action:** Do a single, **short press** of the `BOOT` button to confirm.
* The race immediately starts, and the timer is running for Lap 1.

### 5. Step 4: Timing Laps (New Logic!)

The race is now active. This is where the new logic comes in:

1.  When the robot **first breaks the beam**:
    * The built-in blue LED will turn **ON**.
    * An "obstacle timer" starts, visible in the `heartbeat` status.
2.  The system waits while the beam remains broken. The lap is **not** finished yet.
3.  When the robot **clears the beam**:
    * The built-in blue LED turns **OFF**.
    * The lap is now officially **finished**.
    * The system records the **Total Lap Time** (from start-to-clear) and the **Obstacle Time** (from broken-to-clear).
    * The lap summary is printed, and the next lap begins immediately.

### 6. Step 5: Stop the Race & Get Summary

You can stop the race at any time.

* **Action:** **Press and HOLD** the `BOOT` button for 2 seconds.
* The race session will immediately end.
* A "RACE SUMMARY" table will be printed, showing all completed laps, their total times, and their obstacle times.
* The system will enter a "Finished" state. To run another race, you must restart the ESP32.

---

## 🛠️ Project Structure (For Developers)

This project is broken into classes to separate concerns.

* `src/main.cpp`
    * The main Arduino entry point.
    * It only contains a single global `LapTimer` object.
    * It calls `timer.begin()` in `setup()` and `timer.update()` in `loop()`.

* `src/LapTimer.h` / `src/LapTimer.cpp`
    * This is the main "application" class.
    * It *owns* the Sensor, Button, and RaceTimer objects.
    * It contains the main `State` machine enum.
    * It directs all the logic (e.g., "in this state, check for a button press; in that state, check for a sensor cross").
    * It manages the "is obstacle detected" sub-state during a race.

* `src/Sensor.h` / `src/Sensor.cpp`
    * Handles all sensor-specific logic.
    * Reads the ADC pin.
    * Contains the calibration logic (averaging samples).
    * Provides a simple `isBeamBroken()` method.

* `src/Button.h` / `src/Button.cpp`
    * A reusable button-handling class.
    * Manages all debouncing logic.
    * Detects `isPressed()`, `wasShortPressed()`, and `wasLongPressed()`.

* `src/RaceTimer.h` / `src/RaceTimer.cpp`
    * Handles all time-keeping and data storage.
    * Stores lap times in a `std::vector<LapData>`, which now includes `obstacleTimeMs`.
    * Provides methods like `start()`, `stop()`, and `recordLap()`.
    * Contains the `printSummary()` logic, now with the new "Obstacle (s)" column.

## 🔨 How to Build

This project is set up for **PlatformIO**.

1.  Make sure you have [Visual Studio Code](https://code.visualstudio.com/) with the [PlatformIO IDE extension](https://platformio.org/platformio-ide) installed.
2.  Clone or download this repository.
3.  Open the project folder in VS Code (it should automatically detect it's a PlatformIO project).
4.  Click the "Build" and "Upload" buttons in the PlatformIO toolbar (usually at the bottom of the VS Code window).
5.  Open the Serial Monitor to begin.