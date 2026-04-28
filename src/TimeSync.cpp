#include "TimeSync.h"
#include <WiFiManager.h>
#include <Preferences.h>

// Global for callback use
WiFiManager wm;
extern MatrixPanel_I2S_DMA *dma_display;

// Callback for the portal
void saveParamsCallback() {
    Preferences p;
    p.begin("wordclockwifi", false);
    
    if (wm.server->hasArg("tz_hours")) {
        int hours = wm.server->arg("tz_hours").toInt();
        p.putInt("tz_sec", hours * 3600);
    }
    p.end();
}

TimeSync& TimeSync::getInstance() {
    static TimeSync instance;
    return instance;
}

void TimeSync::sync(bool isResync) {
    Preferences p;
    p.begin("wordclockwifi", true);
    String ssid = p.getString("ssid", "");
    String pass = p.getString("password", "");
    gmtOffset_sec = p.getInt("tz_sec", -28800);
    p.end();

    if (ssid == "") {
        if (!isResync) launchPortal();
        return;
    }

    WiFi.disconnect(true);    
    WiFi.mode(WIFI_OFF);      
    vTaskDelay(pdMS_TO_TICKS(200)); 
    
    WiFi.mode(WIFI_STA);      
    vTaskDelay(pdMS_TO_TICKS(200)); 

    if (!isResync) showUI("CONNECTING");

    WiFi.begin(ssid.c_str(), pass.c_str());

    int retry = 0;
    while (WiFi.status() != WL_CONNECTED && retry < 40) { 
        if (!isResync) drawSpinner(retry, 31, 20); 
        vTaskDelay(pdMS_TO_TICKS(500));
        retry++;
    }

    if (WiFi.status() == WL_CONNECTED && attemptNTP(isResync)) {
        Serial.println("\nSync Success");
        if (!isResync) clearDisplay();
        
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF); 
    } else {
        Serial.println("\nSync Failed.");
        if (!isResync) {
            showUI("WIFI_FAILED");
            vTaskDelay(pdMS_TO_TICKS(3000));
            launchPortal();
        }
    }
}

void TimeSync::launchPortal() {
    showUI("PORTAL_HELP");

    std::vector<const char *> menu = {"wifi", "exit"}; 
    wm.setMenu(menu);

    wm.setTitle("TicTalk Setup");

    wm.setSaveParamsCallback(saveParamsCallback);
    wm.setConfigPortalTimeout(180);
    
    // Custom Parameter for TZ
    char tz_val[5];
    itoa(gmtOffset_sec / 3600, tz_val, 10);
    WiFiManagerParameter custom_tz("tz_hours", "GMT Offset", tz_val, 4);
    wm.addParameter(&custom_tz);

    if (!wm.startConfigPortal("TicTalk")) {
        Serial.println("Portal Timeout/Exit. Restarting...");
        ESP.restart();
    }

    // Success Path: Save credentials and reboot
    Preferences p;
    p.begin("wordclockwifi", false);
    p.putString("ssid", WiFi.SSID());
    p.putString("password", WiFi.psk());
    p.end();

    showUI("SAVED");
    vTaskDelay(pdMS_TO_TICKS(2000));
    ESP.restart(); 
}

bool TimeSync::attemptNTP(bool isResync) {
    if (!isResync) showUI("NTP_SYNC");
    
    configTime(gmtOffset_sec, 3600, "pool.ntp.org");
    
    struct tm timeInfo;
    int retry = 0;
    
    while (!getLocalTime(&timeInfo) && retry < 20) {
        if (!isResync) {
            drawSpinner(retry, 31, 25); 
        }
        
        vTaskDelay(pdMS_TO_TICKS(500));
        retry++;
    }
    
    if (retry < 20) {
        Serial.println("NTP Sync Successful.");
        return true;
    } else {
        Serial.println("NTP Sync Timed Out.");
        return false;
    }
}

void TimeSync::showUI(const char* state) {
    if (dma_display == nullptr) return;
    dma_display->fillScreen(0);
    
    dma_display->setTextColor(dma_display->color565(100, 100, 100));

    if (strcmp(state, "PORTAL_HELP") == 0) {
        dma_display->setCursor(11, 2); dma_display->print("Help me");
        dma_display->setCursor(17, 11); dma_display->print("start");
        dma_display->setCursor(2, 25); dma_display->print("Go to WiFi");
        dma_display->setTextColor(dma_display->color565(0, 100, 155));
        dma_display->setCursor(11, 35); dma_display->print("TicTalk");
        dma_display->setTextColor(dma_display->color565(100, 100, 100));
        dma_display->setCursor(2, 47); dma_display->print("Or wait 3m");
        dma_display->setCursor(2, 55); dma_display->print("to restart");
    } 
    else if (strcmp(state, "CONNECTING") == 0) {
        dma_display->setCursor(2, 10);
        dma_display->print("Connecting");

    }
    else if (strcmp(state, "NTP_SYNC") == 0) {
        dma_display->setCursor(2, 10);
        dma_display->print("Sync Time");
    }
    else if (strcmp(state, "SAVED") == 0) {
        dma_display->setCursor(2, 28);
        dma_display->setTextColor(dma_display->color565(0, 255, 255)); 
        dma_display->print("SAVED!");
    }
    else if (strcmp(state, "WIFI_FAILED") == 0) {
        dma_display->setTextColor(dma_display->color565(255, 0, 0)); 
        dma_display->setCursor(20, 22); dma_display->print("WiFi");
        dma_display->setCursor(14, 34); dma_display->print("Failed");
    }
    else if (strcmp(state, "NO_INTERNET") == 0) {
        dma_display->setTextColor(dma_display->color565(150, 0, 0)); 
        dma_display->setCursor(26, 12); dma_display->print("No");
        dma_display->setCursor(8, 22); dma_display->print("Internet");
        dma_display->setTextColor(dma_display->color565(50, 50, 50));
        dma_display->setCursor(17, 36); dma_display->print("Check");
        dma_display->setCursor(14, 46); dma_display->print("Router");
    }
}

void TimeSync::clearDisplay() {
    if (dma_display) dma_display->fillScreen(0);
}

void TimeSync::drawSpinner(int frame, int x, int y) {
    if (dma_display == nullptr) return;

    // Define 4 positions for a tiny 2x2 "orbit"
    int offsetsX[] = {0, 1, 1, 0};
    int offsetsY[] = {0, 0, 1, 1};

    dma_display->fillRect(x, y, 2, 2, dma_display->color565(0, 0, 0));

    int step = frame % 4;
    dma_display->drawPixel(x + offsetsX[step], y + offsetsY[step], dma_display->color565(0, 100, 155));
}