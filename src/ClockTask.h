#ifndef CLOCKTASK_H
#define CLOCKTASK_H

#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "constants.h"

extern MatrixPanel_I2S_DMA *dma_display;
void displayTime(int h, int m, int lang); 

class ClockTask {
  public:
    void start(int core = 1);
    static void taskEntry(void* pvParameters); 
    int lang = SPANISH;
    bool isPaused = false;

  private:
    void run(); 
    int prevH = -1;
    int prevM = -1;
    int prevL = -1; 
};

#endif


