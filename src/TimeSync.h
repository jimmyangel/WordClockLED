#ifndef TIMESYNC_H
#define TIMESYNC_H

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <Arduino.h>
#include <WiFi.h>
#include <time.h>

extern MatrixPanel_I2S_DMA *dma_display;

// Removed M5 dependency
void timeSync(String ssid, String password);

#endif
