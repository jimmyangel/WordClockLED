#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <Preferences.h>      // Added for credentials
#include "src/ClockTask.h"
#include "src/TimeSync.h"      // Added for the sync function

// Global display pointer
MatrixPanel_I2S_DMA *dma_display = nullptr;

ClockTask clockTask;
Preferences preferences;      // Create preferences instance

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
  dma_display->setBrightness8(30); 
  dma_display->clearScreen();
  Serial.println("Matrix Initialized...");

  // 2. WiFi and Time Sync
  // Open Preferences in read-only mode (true)
  preferences.begin("wordclockwifi", true); 
  String ssid = preferences.getString("ssid", "");
  String password = preferences.getString("password", "");
  preferences.end(); // Close preferences

  if (ssid != "") {
    Serial.println("Credentials found, syncing time...");
    timeSync(ssid, password);
  } else {
    Serial.println("No WiFi credentials found in Preferences!");
  }

  // 3. Start your Clock Task
  clockTask.start(1);
  Serial.println("Clock Task Started!");
}

void loop() {
  delay(1000); 
}




