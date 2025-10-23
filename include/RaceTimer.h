#pragma once

#include <Arduino.h>
#include <vector>

// Data structure for storing one lap
struct LapData {
    int lapNumber;
    unsigned long lapTimeMs;
    unsigned long obstacleTimeMs; // <-- ADDED
};

class RaceTimer {
public:
    RaceTimer(int maxLaps);
    
    void start();
    // *** MODIFIED: Now accepts obstacle time ***
    unsigned long recordLap(unsigned long obstacleTimeMs); 
    void stop();
    void reset();
    
    void printSummary();

    // --- Getters ---
    bool isRunning() const;
    int getLapCount() const;
    unsigned long getCurrentLapTime() const;
    bool isHistoryFull() const;

private:
    std::vector<LapData> _lapHistory;
    int _maxLaps;
    bool _isRunning = false;
    unsigned long _lapStartTime = 0;
    int _lapCount = 0;
};