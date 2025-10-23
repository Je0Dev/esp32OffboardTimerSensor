#include "RaceTimer.h"

RaceTimer::RaceTimer(int maxLaps) : _maxLaps(maxLaps) {
    // Reserve space in the vector for efficiency
    _lapHistory.reserve(maxLaps);
}

void RaceTimer::start() {
    _lapHistory.clear();
    _lapStartTime = millis();
    _lapCount = 1;
    _isRunning = true;
}

// *** MODIFIED: Now saves the obstacle time ***
unsigned long RaceTimer::recordLap(unsigned long obstacleTimeMs) {
    if (!_isRunning) return 0;

    unsigned long currentTime = millis();
    unsigned long grossLapTime = currentTime - _lapStartTime;
    
    // Store the completed lap
    if (_lapHistory.size() < _maxLaps) {
        _lapHistory.push_back({_lapCount, grossLapTime, obstacleTimeMs});
    }

    // Immediately start the next lap
    _lapCount++;
    _lapStartTime = currentTime;

    return grossLapTime;
}

void RaceTimer::stop() {
    _isRunning = false;
}

void RaceTimer::reset() {
    _isRunning = false;
    _lapCount = 0;
    _lapStartTime = 0;
    _lapHistory.clear();
}

bool RaceTimer::isRunning() const {
    return _isRunning;
}

int RaceTimer::getLapCount() const {
    return _lapCount;
}

unsigned long RaceTimer::getCurrentLapTime() const {
    if (!_isRunning) return 0;
    return millis() - _lapStartTime;
}

bool RaceTimer::isHistoryFull() const {
    return _lapHistory.size() >= _maxLaps;
}

// *** MODIFIED: Updated summary table ***
void RaceTimer::printSummary() {
     Serial.println();
     Serial.println("========================================");
     Serial.println("            RACE SUMMARY");
     Serial.println("========================================");

     if (_lapHistory.empty()) {
          Serial.println("No complete laps were recorded.");
          return;
     }
     Serial.println("------------------------------------");
     Serial.println("| Lap | Time (s)   | Obstacle (s) |");
     Serial.println("|-----|------------|--------------|");
     unsigned long totalTime = 0;
     for (const auto& lap : _lapHistory) {
          Serial.printf("| %3d | %10.3f | %12.3f |\n",
                       lap.lapNumber,
                       lap.lapTimeMs / 1000.0,
                       lap.obstacleTimeMs / 1000.0);
          totalTime += lap.lapTimeMs;
     }
     Serial.println("------------------------------------");
     Serial.printf("Total Laps: %d | Total Time: %.3fs\n", _lapHistory.size(), totalTime / 1000.0);
}