#include "ClockTask.h"

void ClockTask::start(int core) {
  xTaskCreatePinnedToCore(
    this->taskEntry, 
    "ClockTask", 
    4096, 
    this, 
    1, 
    NULL, 
    core
  );
}

void ClockTask::taskEntry(void* pvParameters) {
  ClockTask* instance = (ClockTask*)pvParameters;
  instance->run();
}

void ClockTask::run() {
  static int mockH = 12;
  static int mockM = 34;
  static int mockS = 0;

  for (;;) {
    mockS++;
    if (mockS >= 60) { mockS = 0; mockM++; }
    if (mockM >= 60) { mockM = 0; mockH++; }
    if (mockH >= 24) { mockH = 0; }

    if (prevM != mockM || prevH != mockH || prevL != lang) {
      if (dma_display != nullptr) {
        dma_display->fillScreen(0);
        displayTime(mockH, mockM, lang);
      }

      prevH = mockH;
      prevM = mockM;
      prevL = lang;

      Serial.printf("Native Task Update: %02d:%02d\n", mockH, mockM);
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

