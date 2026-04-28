#ifndef TIMESYNC_H
#define TIMESYNC_H

#include <Arduino.h>
#include <WiFiManager.h>
#include <Preferences.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

class TimeSync {
public:
    static TimeSync& getInstance();
    
    // Core Methods
    void sync(bool isResync = false);
    void launchPortal();   // Explicit UI for credentials

private:
    TimeSync() {} // Singleton
    bool attemptNTP(bool isResync = false);
    void showUI(const char* state);
    void clearDisplay();
    void drawSpinner(int frame, int x, int y);
    
    // Parameters
    long gmtOffset_sec = -28800; // Default Pacific
};

#endif