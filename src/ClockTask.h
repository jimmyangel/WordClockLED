#ifndef CLOCKTASK_H
#define CLOCKTASK_H

#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "constants.h"

extern MatrixPanel_I2S_DMA *dma_display;
void displayTime(int h, int m, int lang); // Declaration for the external function

class ClockTask {
  public:
    void start(int core = 1);
    static void taskEntry(void* pvParameters); // Renamed to match .cpp
    int lang = SPANISH;

  private:
    void run(); // Added this because it was missing in your .h
    int prevH = -1;
    int prevM = -1;
    int prevL = -1; // Added to track language changes
};

#endif


