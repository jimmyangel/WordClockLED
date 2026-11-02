#include "TimeSync.h"
#include <WiFiManager.h>
#include <Preferences.h>

// Global for callback use
WiFiManager wm;
extern MatrixPanel_I2S_DMA *dma_display;

WiFiManagerParameter* p_custom_tz = nullptr;

// Callback for the portal
void saveParamsCallback() {
    Preferences p;
    p.begin("wordclockwifi", false);
    
    String tzRule = "";

    // 1. Try reading the distinct HTML selection key directly from the server stream
    if (wm.server->hasArg("tz_selection")) {
        tzRule = wm.server->arg("tz_selection");
    } 
    // 2. Fallback to using the custom parameter reference buffer if server argument fails
    else if (p_custom_tz != nullptr) {
        tzRule = String(p_custom_tz->getValue());
    }

    tzRule.trim();

    if (tzRule.length() > 0) {
        p.putString("tz_rule", tzRule);
        Serial.println("Portal Save: Timezone string updated to: " + tzRule);
    } else {
        Serial.println("Portal Save: Error! Could not extract string from web parameter.");
    }

    // Protect current Wi-Fi settings
    String newSSID = WiFi.SSID();
    if (newSSID.length() > 0) {
        Serial.println("Portal Save: New Wi-Fi network provided. Updating flash...");
        p.putString("ssid", newSSID);
        p.putString("password", WiFi.psk());
    } else {
        Serial.println("Portal Save: No Wi-Fi changes. Preserved current configuration.");
    }
    
    p.end();
    
    Serial.println("Portal Save Complete. Rebooting clock...");
    vTaskDelay(pdMS_TO_TICKS(500)); 
    ESP.restart();
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

    // FORCE MAXIMUM ANTENNA POWER (Disables power-saving throttling)
    WiFi.setSleep(WIFI_PS_NONE);

    vTaskDelay(pdMS_TO_TICKS(200)); 

    if (!isResync) showUI("CONNECTING");

    WiFi.begin(ssid.c_str(), pass.c_str());

    int retry = 0;
    while (WiFi.status() != WL_CONNECTED && retry < 80) { 
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
    
    // Notice we changed name and id to 'tz_selection' inside the HTML string
    const char* tz_dropdown_html = 
        "<br/><label for='tz_selection'>Select Your Timezone</label><br/>"
        "<select name='tz_selection' id='tz_selection' style='width:100%; padding:10px; font-size:16px; margin-bottom:10px;'>"
        "  <option value='PST8PDT,M3.2.0,M11.1.0'>US Pacific Time (PST/PDT)</option>"
        "  <option value='EST5EDT,M3.2.0,M11.1.0'>US Eastern Time (EST/EDT)</option>"
        "  <option value='CST6CDT,M3.2.0,M11.1.0'>US Central Time (CST/CDT)</option>"
        "  <option value='MST7MDT,M3.2.0,M11.1.0'>US Mountain Time (MST/MDT)</option>"
        "  <option value='AST4ADT,M3.2.0,M11.1.0'>Atlantic Time (AST/ADT)</option>"
        "  <option value='AKST9AKDT,M3.2.0,M11.1.0'>Alaska Time (AKST/AKDT)</option>"
        "  <option value='HST10'>Hawaii Time (HST - No DST)</option>"
        "  <option value='CET-1CEST-2,M3.5.0/2,M10.5.0/3'>Central Europe (CET/CEST)</option>"
        "  <option value='GMT0BST-1,M3.5.0/1,M10.5.0/2'>United Kingdom (GMT/BST)</option>"
        "  <option value='AEST-10AEDT,M10.1.0,M4.1.0'>Eastern Australia (AEST/AEDT)</option>"
        "</select>"
        "<style>#custom_tz_t, input[name='custom_tz'] { display: none !important; }</style>";

    // Changed buffer length constraint from 64 to 256
    WiFiManagerParameter custom_tz_param("custom_tz", "", "", 256, tz_dropdown_html, WFM_LABEL_BEFORE);
    wm.addParameter(&custom_tz_param);

    p_custom_tz = &custom_tz_param; 

    String uniqueSSID = "TicTalk-" + String((uint32_t)ESP.getEfuseMac(), HEX).substring(0, 4);

    wm.startConfigPortal(uniqueSSID.c_str());

    Serial.println("Portal Timeout or Exit hit without saving. Restarting clock...");
    ESP.restart();
}

bool TimeSync::attemptNTP(bool isResync) {
    if (!isResync) showUI("NTP_SYNC");
    
    Preferences p;
    p.begin("wordclockwifi", true);
    // Read the string rule directly, fallback to Pacific Time if empty
    String tzRule = p.getString("tz_rule", "PST8PDT,M3.2.0,M11.1.0");
    p.end();

    Serial.printf("Applying System Timezone Rule: %s\n", tzRule.c_str());

    // Pass the saved string straight to the Espressif framework
    configTzTime(tzRule.c_str(), "162.159.200.1", "pool.ntp.org");
    
    // Standard spinner wait loop
    struct tm timeInfo;
    int retry = 0;
    while (!getLocalTime(&timeInfo) && retry < 20) {
        if (!isResync) drawSpinner(retry, 31, 25); 
        vTaskDelay(pdMS_TO_TICKS(500));
        retry++;
    }
    
    if (retry < 20) {
        Serial.println("NTP Sync Successful.");
        Serial.printf("Local Time: %02d:%02d:%02d (DST Status: %s)\n", 
                      timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec,
                      timeInfo.tm_isdst > 0 ? "Daylight Time" : "Standard Time");
        return true;
    }
    return false;
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