# ESP32 Offboard  IR Lap Timer (Serial Version)

---
This is a *simple* lap timer *tested only* for an ESP32 using a single IR (infrared) sensor. It is designed to be highly robust, with no display required. The entire "UI" is handled through the built-in BOOT button and the Arduino Serial Monitor.

---

##  Setup

You only need an ESP32 and a 3-pin IR sensor module.

* **IR Sensor (Analog Out)**: `Sensor A0` -> `ESP32 GPIO 32`
    * *(This must be a safe ADC1 pin, like 32, 33, 34, 35, 36, or 39, for it to work right)*
    * *(Check pin layout to make sure that you connect everything right)*
* **BOOT Button**: `GPIO 0` (Built-in to most ESP32 dev boards,such as mine)
* **LED**: `GPIO 2` (The built-in blue LED that mine had)

---

## Guid

This entire project is controlled using the Serial Monitor and the `BOOT` button.

### 1. Open the Serial Monitor

* Connect your ESP32, build, and upload the code.
* Open the Serial Monitor at **115200 baud**. You will be greeted with the first step.

### 2.  Learn "Clear Beam" Step

The timer must first learn what a clear, unbroken beam looks like.

* **Action:** Ensure the sensor beam is **CLEAR** (no robot in the way).
* **Action:** **Press and HOLD** the `BOOT` button.
* The blue LED will blink rapidly as it takes multiple samples.
* **Action:** **Release** the button after 1-2 seconds.
* If the reading is valid (e.g., `>= 1000`), it will be accepted, and you'll move to the next step.

### 3. Learn "Broken Beam" Step

Now, the timer needs to learn what an obstacle looks like.

* **Action:** Place your robot or hand to **BREAK** the sensor beam.
* **Action:** **Press and HOLD** the `BOOT` button.
* The blue LED will blink as it samples the "broken" value.
* **Action:** **Release** the button.
* If the reading is valid (e.g., `< 1000`), the system will calculate the midpoint threshold and arm the timer.

### 4. Start the Race!!!

The system is now active and "Waiting for robot..."

* **Action:** Let the robot cross the line.
* The timer will detect the *first* line cross and enter a "Pending Start" state.
* The Serial Monitor will display: `ACTION: Start line detected. Press BOOT to start race.`
* **Action:** Do a single, **short press** of the `BOOT` button to confirm.
* The race immediately starts, and the timer is running for Lap 1.

### 5. Timing Laps

The race is now active. This is where this logic comes in:

1.  When the robot **first breaks the beam**:
    * The built-in blue LED will turn **ON**.
    * An "obstacle timer" starts, visible in the `heartbeat` status.
2.  The system waits while the beam remains broken. The lap is **not** finished yet.
3.  When the robot **clears the beam**:
    * The built-in blue LED turns **OFF**.
    * The lap is now officially **finished**.
    * The system records the **Total Lap Time** (from start-to-clear) and the **Obstacle Time** (from broken-to-clear).
    * The lap summary is printed, and the next lap begins immediately.

### 6. Stop the Race & Get Simple Compact Summary

You can stop the race at any time.

* **Action:** **Press and HOLD** the `BOOT` button for 2 seconds.
* The race session will immediately end.
* A "RACE SUMMARY" table will be printed, showing all completed laps, their total times, and their obstacle times.
* The system will enter a "Finished" state. To run another race, you must restart the ESP32.

---
## Building Steps
1.  Make sure you have [Visual Studio Code](https://code.visualstudio.com/) with the [PlatformIO IDE extension](https://platformio.org/platformio-ide) installed.
2.  Clone repo.
3.  Click the "Build" and "Upload" buttons in the PlatformIO toolbar
4.  Open the Serial Monitor to begin.

---
## Licence
- https://en.wikipedia.org/wiki/MIT_License 
