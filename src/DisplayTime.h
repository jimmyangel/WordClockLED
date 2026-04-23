#ifndef DISPLAYTIME_H
#define DISPLAYTIME_H

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "TimeWords.h"
#include "constants.h"

extern MatrixPanel_I2S_DMA *dma_display;

// Added color parameter
void displayTime(int hours, int minutes, int lang, uint16_t color);

#endif

