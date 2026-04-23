## A bilingual word clock using an LED Matrix

### Parts list:

* HUB75 RGB LED Matrix Display Panel - P3-64x64
* ElectroDragon RGB Matrix Panel Drive Interface Board for ESP32 DMA
* ESP32 Mini Core Dev. Board, ESP32-DevKitC, V4

### Hardware setup:

* Jumper E-18 to enable 64x64
* Miniswitch set to DEVKitC (the microcontroller being used)


### Sketch to initialize WiFi credentials (temporary)

```
#include <Arduino.h>
#include <Preferences.h>

Preferences preferences;

// Replace with your actual credentials
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

void setup() {
  // Use the same baud rate as your main project
  Serial.begin(115200);
  delay(1000); 
  Serial.println("\n--- Credential Setter Starting ---");

  // Open the "wordclockwifi" namespace (false = read/write)
  preferences.begin("wordclockwifi", false);
  
  // Store the strings
  preferences.putString("ssid", ssid); 
  preferences.putString("password", password);

  Serial.println(">> WordClock Wifi Credentials Saved!");
  Serial.printf(">> SSID: %s\n", ssid);
  
  // Close preferences
  preferences.end();
  
  Serial.println("--- Done. You can now flash your main project. ---");
}

void loop() {
  // Do nothing
}
```
