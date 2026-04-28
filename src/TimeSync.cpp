#include "TimeSync.h"
#include <WiFiManager.h>
#include <Preferences.h>
#include "constants.h"

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
  p.putInt("tz_sec", (tempTZHours * 3600)); 
  
  if (WiFi.SSID() != "") {
    p.putString("ssid", WiFi.SSID());
    p.putString("password", WiFi.psk());
  }
  p.end();
  Serial.println("Flash Write Complete.");
}

void timeSync(String ssid, String password, bool forcePortal) {
  Preferences prefs;

  // 1. TRIGGER: Setup Mode (Empty SSID or Brass Post Long-Touch)
  if (ssid == "" || forcePortal) {
    if (dma_display != nullptr) {
      dma_display->fillScreen(0);
        
      dma_display->setTextColor(dma_display->color565(100, 100, 100));
      dma_display->setCursor(11, 2); 
      dma_display->print("Help me");

      dma_display->setCursor(17, 11);
      dma_display->print("start");

      dma_display->setCursor(2, 25);
      dma_display->print("Go to WiFi");

      dma_display->setTextColor(dma_display->color565(0, 100, 155)); 
      dma_display->setCursor(11, 35);
      dma_display->print("TicTalk");

      dma_display->setTextColor(dma_display->color565(100, 100, 100));
      dma_display->setCursor(2, 47);
      dma_display->print("Or wait 3m");
      dma_display->setCursor(2, 55); 
      dma_display->print("to restart");
    }

    // Setup WiFiManager Portal
    wm.setSaveParamsCallback(saveParamsCallback);
    wm.setBreakAfterConfig(true); 

    // Custom Menu
    std::vector<const char *> menu = {"custom", "exit"};
    wm.setMenu(menu);
    const char* menuhtml = "<form action='/wifi' method='get'><button>Configure Clock</button></form><br/>";
    wm.setCustomMenuHTML(menuhtml);

    // Custom Parameters
    const char* tz_html = "<br/><label for='tz_hours'>GMT Offset (Hours)</label>"
                          "<input type='number' name='tz_hours' id='tz_hours' min='-12' max='14' value='-8' step='1'>";
    
    WiFiManagerParameter custom_tz_hours(tz_html);
    wm.addParameter(&custom_tz_hours);

    wm.setConfigPortalTimeout(180); 
    if (!wm.startConfigPortal("TicTalk")) {
       Serial.println("Portal Timeout. Restarting...");
       ESP.restart(); 
    }

    if (dma_display != nullptr) {
      dma_display->fillScreen(0);
      dma_display->setCursor(2, 28);
      dma_display->setTextColor(dma_display->color565(0, 255, 255));
      dma_display->print("SAVED!");
    }

    vTaskDelay(pdMS_TO_TICKS(3000));

    // --- HARDWARE SHUTDOWN ---
    if (dma_display != nullptr) {
      dma_display->fillScreen(0); 
      dma_display->flipDMABuffer();
      dma_display->fillScreen(0);
      vTaskDelay(pdMS_TO_TICKS(100));
      pinMode(18, OUTPUT); digitalWrite(18, HIGH); // OE HIGH 
      vTaskDelay(pdMS_TO_TICKS(50));
    }
    ESP.restart(); 
  }
  
  // 2. NORMAL BOOT: Connect and Sync
  WiFi.begin(ssid.c_str(), password.c_str());
  Serial.print("Connecting WiFi");

  if (dma_display != nullptr) {
    dma_display->fillScreen(0);
    dma_display->setTextColor(dma_display->color565(100, 100, 100));
    dma_display->setCursor(2, 10);
    dma_display->print("Connecting");
  }

  for (int i = 0; i < 20 && WiFi.status() != WL_CONNECTED; i++) {
    vTaskDelay(pdMS_TO_TICKS(500));
    Serial.print(".");
    if (dma_display != nullptr) dma_display->print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    prefs.begin("wordclockwifi", true);
    long gmtOffset = prefs.getInt("tz_sec", -28800); 
    prefs.end();

    configTime(gmtOffset, 3600, "pool.ntp.org"); 

    if (dma_display != nullptr) {
      dma_display->fillScreen(0);
      dma_display->setCursor(2, 10);
      dma_display->print("Sync Time");
    }

    struct tm timeInfo;
    int retry = 0;
    while (!getLocalTime(&timeInfo) && retry < 20) {
      vTaskDelay(pdMS_TO_TICKS(500));
      Serial.print(".");
      if (dma_display != nullptr) dma_display->print(".");
      retry++;
    }

    // --- NTP FAILURE HANDLER ---
    if (retry >= 20) {
      Serial.println("WiFi OK, Internet/NTP blocked.");
      if (dma_display != nullptr) {
        dma_display->fillScreen(0);
        
        // "No Internet" 
        dma_display->setTextColor(dma_display->color565(150, 0, 0)); 
        dma_display->setCursor(26, 12); 
        dma_display->print("No");
        dma_display->setCursor(8, 22);
        dma_display->print("Internet");

        dma_display->setTextColor(dma_display->color565(50, 50, 50));
        dma_display->setCursor(17, 36);
        dma_display->print("Check");
        dma_display->setCursor(14, 46);
        dma_display->print("Router");
        
        vTaskDelay(pdMS_TO_TICKS(5000)); 
      }
      
      // Trigger Portal
      timeSync("", "", true); 
      return;
    }

    if (dma_display != nullptr) dma_display->fillScreen(0);

    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    Serial.println("\nTime Synced. WiFi Off.");
  } else {
      // 3. FAILURE RECOVERY: WiFi failed after 10 seconds of retries
      if (dma_display != nullptr) {
        dma_display->fillScreen(0);
        dma_display->setTextColor(dma_display->color565(255, 0, 0)); // Red Alert

        dma_display->setCursor(20, 22); 
        dma_display->print("WiFi");
        dma_display->setCursor(14, 34); 
        dma_display->print("Failed");

        vTaskDelay(pdMS_TO_TICKS(5000));
      }
      
      // Trigger Portal
      Serial.println("WiFi Failed. Entering Config Portal as fallback...");
      timeSync("", "", true); // Recursive call to trigger SETUP MODE

      return;
    }
    if (dma_display != nullptr) dma_display->fillScreen(0);
}
