#include "TouchTask.h"
#include <Preferences.h>

void TouchTask::start(ClockTask* clockInstance) {
    xTaskCreatePinnedToCore(taskEntry, "TouchTask", 3072, clockInstance, 1, NULL, 0);
}

void TouchTask::taskEntry(void* pvParameters) {
    ClockTask* clock = (ClockTask*)pvParameters;
    const int touchPin = 32;
    const int threshold = 850; 
    bool isTouching = false;
    unsigned long startTime = 0;
    Preferences prefs;

    for (;;) {
        int val = touchRead(touchPin);

        if (val < threshold && !isTouching) {
            startTime = millis();
            isTouching = true;
        } 
        else if (val < threshold && isTouching) {
            if (millis() - startTime > 3000) {
                Serial.println("TOUCH: WiFi Portal Triggered");
                // Note: timeSync might need display access; ensure it doesn't crash ClockTask
                // timeSync(ssid, pass); 
                isTouching = false;
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
        } 
        else if (val > threshold + 50 && isTouching) {
            unsigned long duration = millis() - startTime;
            if (duration < 800) {
                // Toggle Language
                clock->lang = (clock->lang == 0) ? 1 : 0;
                
                // Persist choice
                prefs.begin("wordclockwifi", false);
                prefs.putInt("lang", clock->lang);
                prefs.end();
                
                Serial.printf("TOUCH: Lang Switched to %d\n", clock->lang);
            }
            isTouching = false;
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}