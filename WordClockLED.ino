#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <Preferences.h>
#include "src/ClockTask.h"
#include "src/TouchTask.h"
#include "src/TimeSync.h" 
#include "src/constants.h"

MatrixPanel_I2S_DMA *dma_display = nullptr;
ClockTask clockTask;
Preferences preferences;

volatile bool triggerPortal = false; // Flag set by TouchTask

void setup() {
  Serial.begin(115200);
  vTaskDelay(pdMS_TO_TICKS(1000));
  Serial.println("--- TIC TALK BOOT ---");

  // 1. Initialize Matrix
  HUB75_I2S_CFG mxconfig(64, 64, 1);
  mxconfig.gpio.e = 18; 
  mxconfig.clkphase = false;
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(1); // Keep dim during sync

  // 2. Initial Sync
  // This now handles reading SSID/Pass and fallback to Portal internally
  TimeSync::getInstance().sync();

  // 3. Load Language & Resume UI
  preferences.begin("wordclockwifi", true); 
  clockTask.lang = preferences.getInt("lang", 0); 
  preferences.end();

  dma_display->setBrightness8(160); 
  dma_display->clearScreen();

  // 4. Start Tasks
  clockTask.start(1);
  TouchTask::start(&clockTask);
}

void loop() {
  // 1. The Touch Trigger (Priority check)
  if (triggerPortal) {
    triggerPortal = false; 
    clockTask.isPaused = true;
    
    // Since we're on Core 1, we call this directly. 
    // It will block the loop until the portal is done or the ESP restarts.
    TimeSync::getInstance().launchPortal();
  }

  // 2. The Nightly Timer (At 3AM)
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck > 1000) { // Check once per second
    lastCheck = millis();
    
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      if (timeinfo.tm_hour == 3 && timeinfo.tm_min == 0 && timeinfo.tm_sec == 0) {

        Serial.println("Scheduled Nightly Resync...");
        TimeSync::getInstance().sync(true);
      }
    }
  }

  vTaskDelay(pdMS_TO_TICKS(50)); 
}