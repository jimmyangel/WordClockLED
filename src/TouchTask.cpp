#include "TouchTask.h"
#include "TimeSync.h"
#include <Preferences.h>

extern bool triggerPortal;

void TouchTask::start(ClockTask* clockInstance) {
    xTaskCreatePinnedToCore(taskEntry, "TouchTask", 3072, clockInstance, 1, NULL, 0);
}

void TouchTask::taskEntry(void* pvParameters) {
    ClockTask* clock = (ClockTask*)pvParameters;
    const int touchPin = 32;
    const int threshold = 750; 
    bool isTouching = false;
    unsigned long startTime = 0;
    Preferences prefs;

    for (;;) {
        int val = touchRead(touchPin);

        if (val < threshold) {
            dma_display->drawPixel(63, 0, dma_display->color565(0, 150, 0));

            if (!isTouching) {
                startTime = millis();
                isTouching = true;
            } else if (millis() - startTime > 3000) {
                if (!triggerPortal) {
                    Serial.println("TOUCH: WiFi Portal Triggered");
                    triggerPortal = true;
                }
            }
        } else if (val > threshold + 50 && isTouching) {
            dma_display->drawPixel(63, 0, dma_display->color565(0, 0, 0));

            if (millis() - startTime < 800) {
                // Quick Tap: Toggle Language
                clock->lang = (clock->lang == 0) ? 1 : 0;
                
                prefs.begin("wordclockwifi", false);
                prefs.putInt("lang", clock->lang);
                prefs.end();
                
                Serial.printf("TOUCH: Lang Switched to %d\n", clock->lang);
            }
            isTouching = false;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}