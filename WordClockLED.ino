#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <Preferences.h> 
#include "src/ClockTask.h"
#include "src/TouchTask.h"
#include "src/TimeSync.h" 
#include "src/constants.h"

// Global display pointer
MatrixPanel_I2S_DMA *dma_display = nullptr;

ClockTask clockTask;
Preferences preferences;

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); } 
  delay(1000);                   
  Serial.println("--- BOOT START ---");

  // 1. Initialize Matrix
  HUB75_I2S_CFG mxconfig(64, 64, 1);
  mxconfig.gpio.e = 18; 
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(1); 
  dma_display->clearScreen();
  Serial.println("Matrix Initialized...");

  // 2. WiFi and Time Sync
  // Open Preferences in read-only mode (true)
  preferences.begin("wordclockwifi", true); 
  String ssid = preferences.getString("ssid", "");
  String password = preferences.getString("password", "");
  int finalLang = preferences.getInt("lang", 0); // 0 = SPANISH default
  preferences.end(); // Close preferences

  // Run the sync/portal logic
  timeSync(ssid, password);

  dma_display->setBrightness8(80); 
  dma_display->clearScreen();

  clockTask.lang = finalLang;

  // 3. Start your Clock Task
  clockTask.start(1);
  Serial.printf("System started with Language ID: %d\n", finalLang);

  TouchTask::start(&clockTask);
  
}

bool triggerPortal = false; // Global flag

void loop() {
  if (triggerPortal) {
    triggerPortal = false; // Reset flag
    clockTask.isPaused = true;
    
    // Get current credentials
    preferences.begin("wordclockwifi", true);
    String s = preferences.getString("ssid", "");
    String p = preferences.getString("password", "");
    preferences.end();

    timeSync(s, p, true); // Launch portal
  }
  delay(1000); 
}
