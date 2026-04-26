#include "ClockTask.h"
#include "DisplayTime.h"
#include <Preferences.h>
#include "TimeSync.h"

// Normalized Color Blend Function
uint16_t blendColorNormalized(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2, float ratio) {
    // Standard linear blend
    float r = r1 + (r2 - r1) * ratio;
    float g = g1 + (g2 - g1) * ratio;
    float b = b1 + (b2 - b1) * ratio;

    // Calculate current sum
    float currentSum = r + g + b;

    // Prevent division by zero if all are 0
    if (currentSum > 0) {
        float scale = 255.0 / currentSum;
        r *= scale;
        g *= scale;
        b *= scale;
    }

    return dma_display->color565((uint8_t)r, (uint8_t)g, (uint8_t)b);
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
      float ratio;

      if (h >= 6 && h < 12) { // 6am (White) -> 12pm (Orange)
          ratio = (h - 6 + m/60.0) / 6.0;
          // White (85,85,85) -> Orange (200, 55, 0) | Both sum to 255
          currentColor = blendColorNormalized(85, 85, 85, 200, 55, 0, ratio);
      } 
      else if (h >= 12 && h < 18) { // 12pm (Orange) -> 6pm (Light Blue)
          ratio = (h - 12 + m/60.0) / 6.0;
          // Orange -> Light Blue (50, 100, 105)
          currentColor = blendColorNormalized(200, 55, 0, 50, 100, 105, ratio);
      } 
      else if (h >= 18 && h < 24) { // 6pm (Light Blue) -> 12am (Dark Blue)
          ratio = (h - 18 + m/60.0) / 6.0;
          // Light Blue -> Dark Blue (0, 0, 255)
          currentColor = blendColorNormalized(50, 100, 105, 0, 0, 255, ratio);
      } 
      else { // 12am (Dark Blue) -> 6am (White)
          ratio = (h < 6) ? (h + m/60.0) / 6.0 : 0; 
          currentColor = blendColorNormalized(0, 0, 255, 85, 85, 85, ratio);
      }

      // 3. DISPLAY UPDATE
      if (!isPaused) { // Only draw if not paused
        if (prevM != m || prevH != h || prevL != lang) {
          if (dma_display != nullptr) {
            dma_display->fillScreen(0);
            displayTime(h, m, lang, currentColor);
          }
          prevH = h; prevM = m; prevL = lang;
          //Serial.printf("Time: %02d:%02d | Color: %04X\n", h, m, currentColor);
        }
      }
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}





