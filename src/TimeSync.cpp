#include "TimeSync.h"
#include <WiFiManager.h>
#include <Preferences.h>
#include "constants.h"

// Access global display pointer for feedback messages
extern MatrixPanel_I2S_DMA *dma_display;

// Global WiFiManager instance for callback access
WiFiManager wm;
int tempLangChoice = 0;
int tempTZHours = -8; // Default to Pacific

// CALLBACK: Captures parameters from the web portal immediately on "Save"
void saveParamsCallback() {
  Serial.println("Save button pressed! Capturing parameters...");
  
  if (wm.server->hasArg("lang")) {
    tempLangChoice = wm.server->arg("lang").toInt();
  }
  if (wm.server->hasArg("tz_hours")) {
    tempTZHours = wm.server->arg("tz_hours").toInt();
  }

  Serial.printf("Captured -> Lang: %d, TZ Hours: %d. Saving...\n", tempLangChoice, tempTZHours);
  
  Preferences p;
  p.begin("wordclockwifi", false);
  p.putInt("lang", tempLangChoice);
  p.putInt("tz_sec", (tempTZHours * 3600)); // Save as seconds for sync logic
  
  if (WiFi.SSID() != "") {
    p.putString("ssid", WiFi.SSID());
    p.putString("password", WiFi.psk());
  }
  p.end();
  Serial.println("Flash Write Complete.");
}

void timeSync(String ssid, String password) {
  Preferences prefs;
  prefs.begin("wordclockwifi", false);
  
  // Track boot count for the double-power-cycle trigger
  int bootCount = prefs.getInt("boot_count", 0);
  bootCount++;
  prefs.putInt("boot_count", bootCount);

  // 1. TRIGGER: Double-Boot or Empty SSID
  if (ssid == "" || bootCount >= 2) {
    if (dma_display != nullptr) {
      dma_display->fillScreen(0);
      dma_display->setTextColor(dma_display->color565(255, 255, 255));
      dma_display->setCursor(2, 12);
      dma_display->print("SETUP MODE");
      dma_display->setCursor(2, 30);
      dma_display->print("WordClock-Setup");
    }

    prefs.putInt("boot_count", 0); 
    prefs.end();

    // Setup WiFiManager Portal
    wm.setSaveParamsCallback(saveParamsCallback);
    wm.setBreakAfterConfig(true); 

    // Custom Menu (Only "Configure Clock" and "Exit")
    std::vector<const char *> menu = {"custom", "exit"};
    wm.setMenu(menu);
    const char* menuhtml = "<form action='/wifi' method='get'><button>Configure Clock</button></form><br/>";
    wm.setCustomMenuHTML(menuhtml);

    // Custom Parameters (Timezone Hours and Language)
    const char* tz_html = "<br/><label for='tz_hours'>GMT Offset (Hours)</label>"
                          "<input type='number' name='tz_hours' id='tz_hours' min='-12' max='14' value='-8' step='1'>";
    const char* lang_html = "<br/><label for='lang'>Select Language</label>"
                            "<select name='lang' id='lang'>"
                            "<option value='0'>Spanish</option>"
                            "<option value='1'>English</option></select>";
    
    WiFiManagerParameter custom_tz_hours(tz_html);
    WiFiManagerParameter custom_lang_picker(lang_html);
    wm.addParameter(&custom_tz_hours);
    wm.addParameter(&custom_lang_picker);

    wm.setConfigPortalTimeout(180); 
    if (!wm.startConfigPortal("WordClock-Setup")) {
       ESP.restart(); 
    }

    if (dma_display != nullptr) {
      dma_display->fillScreen(0);
      dma_display->setCursor(2, 28);
      dma_display->setTextColor(dma_display->color565(0, 255, 255));
      dma_display->print("SAVED!");
    }

    Serial.println("Rebooting in 3 seconds...");
    delay(3000); 

    // --- HARDWARE SHUTDOWN: Prevent random LED sparkle ---
    if (dma_display != nullptr) {
      dma_display->fillScreen(0); 
      dma_display->flipDMABuffer();
      dma_display->fillScreen(0);
      delay(100); 
      pinMode(15, OUTPUT); digitalWrite(15, HIGH); // OE HIGH
      pinMode(4, OUTPUT);  digitalWrite(4, LOW);  // LAT LOW
      pinMode(17, OUTPUT); digitalWrite(17, LOW); // CLK LOW
      delay(50);
    }
    ESP.restart(); 
  }
  
  prefs.end();

  // 2. NORMAL BOOT: Connect and Sync
  WiFi.begin(ssid.c_str(), password.c_str());
  Serial.print("Connecting WiFi");
  for (int i = 0; i < 20 && WiFi.status() != WL_CONNECTED; i++) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    // Load the user's saved timezone offset
    prefs.begin("wordclockwifi", true);
    long gmtOffset = prefs.getInt("tz_sec", -28800); // Default -8h
    prefs.end();

    configTime(gmtOffset, 3600, "pool.ntp.org"); 

    struct tm timeInfo;
    int retry = 0;
    while (!getLocalTime(&timeInfo) && retry < 20) {
      delay(500);
      retry++;
    }

    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    Serial.println("\nTime Synced. WiFi Off.");
  }

  // 4. SAFETY WINDOW (3 seconds)
  delay(3000); 
  prefs.begin("wordclockwifi", false);
  prefs.putInt("boot_count", 0);
  prefs.end();
}
