#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "src/ClockTask.h"

// Global display pointer
MatrixPanel_I2S_DMA *dma_display = nullptr;

ClockTask clockTask;

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); } // Wait for serial port to connect
  delay(1000);                   // Extra 1s buffer
  Serial.println("--- BOOT START ---");

  // 1. Initialize Matrix
  HUB75_I2S_CFG mxconfig(64, 64, 1);
  mxconfig.gpio.e = 18; 
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(30); 
  dma_display->clearScreen();

  Serial.println("Matrix Initialized...");

  // 2. Start your Clock Task
  // This will now run on Core 1 (or whichever you assigned in the class)
  clockTask.start(1);
  
  Serial.println("Clock Task Started!");
}

void loop() {
  // Keep the loop empty for now
  delay(1000); 
}



