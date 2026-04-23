#include "TimeSync.h"

void timeSync(String ssid, String password) {
  const char* ntpServer = "pool.ntp.org";
  const long gmtOffset_sec = -28800; // Pacific Time
  const int daylightOffset_sec = 3600; 

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  
  Serial.print("Connecting WiFi");
  for (int i = 0; i < 20 && WiFi.status() != WL_CONNECTED; i++) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected! Syncing Time...");
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    // CRITICAL: Wait for the internal clock to actually update
    struct tm timeInfo;
    int retry = 0;
    const int maxRetry = 20; // Wait up to 10 seconds
    
    while (!getLocalTime(&timeInfo) && retry < maxRetry) {
      delay(500);
      Serial.print("Waiting for NTP...");
      retry++;
    }

    if (retry < maxRetry) {
      Serial.println("\nTime Synchronized!");
    } else {
      Serial.println("\nTime Sync Timeout (Check Firewall/Port 123)");
    }

    // Now it is safe to disconnect
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
  } else {
    Serial.println("\nWiFi Connection Failed.");
  }
}

