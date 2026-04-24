#include "TimeSync.h"
#include <WiFiManager.h>
#include <Preferences.h>
#include "constants.h"

extern MatrixPanel_I2S_DMA *dma_display;

// Global instance so callback can access it
WiFiManager wm;

// THE CALLBACK: Move all saving logic INTO the callback
void saveParamsCallback() {
  Serial.println("Save button pressed! Capturing parameters...");
  
  if (wm.server->hasArg("lang")) {
    int langID = wm.server->arg("lang").toInt();
    Serial.printf("Language captured: %d. Saving to Flash...\n", langID);
    
    Preferences p;
    p.begin("wordclockwifi", false);
    p.putInt("lang", langID);
    
    if (WiFi.SSID() != "") {
      p.putString("ssid", WiFi.SSID());
      p.putString("password", WiFi.psk());
    }
    p.end(); 
    Serial.println("Flash Write Complete.");
  }
}

void timeSync(String ssid, String password) {
  Preferences prefs;
  prefs.begin("wordclockwifi", false);
  
  int bootCount = prefs.getInt("boot_count", 0);
  bootCount++;
  prefs.putInt("boot_count", bootCount);

  if (ssid == "" || bootCount >= 2) {
    if (dma_display != nullptr) {
      dma_display->fillScreen(0);
      dma_display->setTextColor(dma_display->color565(255, 255, 255));
      dma_display->setTextSize(1);
      dma_display->setCursor(2, 12);
      dma_display->print("SETUP MODE");
      dma_display->setCursor(2, 30);
      dma_display->print("WordClock-Setup");
    }

    prefs.putInt("boot_count", 0); 
    prefs.end();

    wm.setSaveParamsCallback(saveParamsCallback);
    wm.setBreakAfterConfig(true); 

    std::vector<const char *> menu = {"custom", "exit"};
    wm.setMenu(menu);
    const char* menuhtml = "<form action='/wifi' method='get'><button>Configure Clock</button></form><br/>";
    wm.setCustomMenuHTML(menuhtml);

    const char* custom_html = "<br/><label for='lang'>Select Language</label>"
                              "<select name='lang' id='lang'>"
                              "<option value='0'>Spanish</option>"
                              "<option value='1'>English</option></select>";
    WiFiManagerParameter custom_lang_picker(custom_html);
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

    // --- FINAL REBOOT CLEANUP ---
    if (dma_display != nullptr) {
      dma_display->fillScreen(0); 
      dma_display->flipDMABuffer();
      dma_display->fillScreen(0);
      delay(100); // Give DMA time to push the black frame
      
      // HARDWARE LOCK: Force pins to safe states
      // OE (GPIO 15) -> HIGH (Disable output)
      // LAT (GPIO 4) -> LOW (Stop latching data)
      // CLK (GPIO 17) -> LOW (Stop shifting data)
      
      pinMode(15, OUTPUT); digitalWrite(15, HIGH); 
      pinMode(4, OUTPUT);  digitalWrite(4, LOW);  
      pinMode(17, OUTPUT); digitalWrite(17, LOW); 
      
      delay(50); // Small pause to let voltages settle
    }

    Serial.println("Rebooting...");
    ESP.restart(); 

  }
  
  prefs.end();

  WiFi.begin(ssid.c_str(), password.c_str());
  Serial.print("Connecting WiFi");
  for (int i = 0; i < 20 && WiFi.status() != WL_CONNECTED; i++) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected! Syncing with NTP...");
    configTime(-28800, 3600, "pool.ntp.org"); 

    struct tm timeInfo;
    int retry = 0;
    const int maxRetry = 20; 
    
    while (!getLocalTime(&timeInfo) && retry < maxRetry) {
      delay(500);
      Serial.print("Waiting for NTP...");
      retry++;
    }

    if (retry < maxRetry) {
      Serial.println("\nTime Synchronized!");
    } else {
      Serial.println("\nNTP Sync Failed!");
    }

    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    Serial.println("WiFi Disconnected.");
  } else {
    Serial.println("\nWiFi Connection Failed.");
  }

  delay(3000); 
  prefs.begin("wordclockwifi", false);
  prefs.putInt("boot_count", 0);
  prefs.end();
}




