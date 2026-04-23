#ifndef TIMESYNC_H
#define TIMESYNC_H

#include <Arduino.h>
#include <WiFi.h>
#include <time.h>

// Removed M5 dependency
void timeSync(String ssid, String password);

#endif
