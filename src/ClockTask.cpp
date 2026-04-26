#include "ClockTask.h"
#include "DisplayTime.h"
#include <Preferences.h>
#include "TimeSync.h"

// Helper to blend two colors based on a ratio (0.0 to 1.0)
uint16_t blendColor(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2, float ratio) {
  uint8_t r = r1 + (r2 - r1) * ratio;
  uint8_t g = g1 + (g2 - g1) * ratio;
  uint8_t b = b1 + (b2 - b1) * ratio;
  return dma_display->color565(r, g, b);
}

void ClockTask::start(int core) {
  xTaskCreatePinnedToCore(this->taskEntry, "ClockTask", 4096, this, 1, NULL, core);
}

void ClockTask::taskEntry(void* pvParameters) {
  ClockTask* instance = (ClockTask*)pvParameters;
  instance->run();
}

void ClockTask::run() {
  struct tm timeinfo;
  Preferences prefs;

  for (;;) {
    if (getLocalTime(&timeinfo)) {
      int h = timeinfo.tm_hour;
      int m = timeinfo.tm_min;
      int s = timeinfo.tm_sec;

      // 1. WEEKLY SYNC (Sunday 3AM)
      if (timeinfo.tm_wday == 0 && h == 3 && m == 0 && s == 0) {
        prefs.begin("wordclockwifi", true);
        String s_id = prefs.getString("ssid", "");
        String p_wd = prefs.getString("password", "");
        prefs.end();
        if (s_id != "") timeSync(s_id, p_wd);
      }

      // 2. COLOR BLENDING MATH
      uint16_t currentColor;
      if (h >= 6 && h < 12) { // 6am (White) -> 12pm (Orange)
        float ratio = (h - 6 + m/60.0) / 6.0;
        currentColor = blendColor(255, 255, 255, 255, 165, 0, ratio);
      } 
      else if (h >= 12 && h < 18) { // 12pm (Orange) -> 6pm (Light Blue)
        float ratio = (h - 12 + m/60.0) / 6.0;
        currentColor = blendColor(255, 165, 0, 100, 150, 255, ratio);
      } 
      else if (h >= 18 && h < 24) { // 6pm (Light Blue) -> 12am (Dark Blue)
        float ratio = (h - 18 + m/60.0) / 6.0;
        currentColor = blendColor(100, 150, 255, 0, 0, 100, ratio);
      } 
      else { // 12am (Dark Blue) -> 6am (White)
        float ratio = (h < 6) ? (h + m/60.0) / 6.0 : 0; 
        currentColor = blendColor(0, 0, 100, 255, 255, 255, ratio);
      }

      // 3. DISPLAY UPDATE
      if (!isPaused) { // Only draw if not paused
        if (prevM != m || prevH != h || prevL != lang) {
          if (dma_display != nullptr) {
            dma_display->fillScreen(0);
            displayTime(h, m, lang, currentColor);
          }
          prevH = h; prevM = m; prevL = lang;
          Serial.printf("Time: %02d:%02d | Color: %04X\n", h, m, currentColor);
        }
      }
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}




