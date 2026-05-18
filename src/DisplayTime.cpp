#include "DisplayTime.h"

TimeWords tW;

#define TYPE_E_ACUTE 1
#define TYPE_O_CIRC  2
#define TYPE_E_GRAVE 3

/**
 * Draws custom pixels for French accents.
 * E-Grave accent shifted right by 1px to match E-Acute alignment.
 * O-Circumflex uses the custom 5x7 rounded map.
 */
void drawCustomFrench(int x, int y, int type, uint16_t color) {
  if (type == TYPE_E_ACUTE || type == TYPE_E_GRAVE) {
    int base = y + 2;
    // Vertical Stem
    for (int i = 0; i < 5; i++) dma_display->drawPixel(x, base + i, color);
    // Horizontal Bars
    for (int i = 0; i < 5; i++) {
      dma_display->drawPixel(x + i, base, color);
      dma_display->drawPixel(x + i, base + 4, color);
    }
    for (int i = 0; i < 4; i++) dma_display->drawPixel(x + i, base + 2, color);

    if (type == TYPE_E_ACUTE) { // É (\x90)
      dma_display->drawPixel(x + 3, y, color); 
      dma_display->drawPixel(x + 2, y + 1, color);
    } else { 
      dma_display->drawPixel(x + 1, y, color); 
      dma_display->drawPixel(x + 2, y + 1, color);
    }
  } 
  else if (type == TYPE_O_CIRC) { 
    // Row 0: Peak
    dma_display->drawPixel(x + 2, y, color);
    // Row 1: Shoulders
    dma_display->drawPixel(x + 1, y + 1, color);
    dma_display->drawPixel(x + 3, y + 1, color);
    // Row 2: Top of O
    dma_display->drawPixel(x + 1, y + 2, color);
    dma_display->drawPixel(x + 2, y + 2, color);
    dma_display->drawPixel(x + 3, y + 2, color);
    // Rows 3, 4, 5: Sides
    for (int i = 0; i < 3; i++) {
      dma_display->drawPixel(x, y + 3 + i, color);
      dma_display->drawPixel(x + 4, y + 3 + i, color);
    }
    // Row 6: Bottom of O
    dma_display->drawPixel(x + 1, y + 6, color);
    dma_display->drawPixel(x + 2, y + 6, color);
    dma_display->drawPixel(x + 3, y + 6, color);
  }
}

void displayTime(int hours, int minutes, int lang, uint16_t color) {
  int activeLang = (lang >= 0 && lang <= 2) ? lang : 0;
  String t = tW.getWords(hours, minutes, activeLang);
  t.trim();

  String lines[6]; 
  int lineCount = 0;
  int start = 0;
  int standardMax = 10; 

  while (start < t.length() && lineCount < 6) {
    int end = t.indexOf(' ', start + 1);
    if (t.substring(start).startsWith("VEINTICINCO") || 
        t.substring(start).startsWith("TWENTY-FIVE") ||
        t.substring(start).startsWith("VINGT-TROIS")) {
        
        end = start + 11; 
    } else {
        while (end != -1 && end - start <= standardMax) {
          int nextSpace = t.indexOf(' ', end + 1);
          if (nextSpace == -1) {
            if (t.length() - start <= standardMax) end = t.length();
            break;
          }
          if (nextSpace - start <= standardMax) end = nextSpace;
          else break;
        }
    }
    if (end == -1) end = t.length();
    lines[lineCount] = t.substring(start, end);
    lines[lineCount].trim();
    if(lines[lineCount].length() > 0) lineCount++;
    start = end;
    while (start < t.length() && t[start] == ' ') start++; 
  }

  int lineHeight = 10;
  int yOffset = (64 - (lineCount * lineHeight - 2)) / 2;

  dma_display->setTextSize(1);

  for (int i = 0; i < lineCount; i++) {
    String line = lines[i];
    int currentY = yOffset + (i * lineHeight);

    // Calculate width (Custom characters are 6px wide)
    int totalWidth = 0;
    for(int j = 0; j < line.length(); j++) {
      uint8_t c = (uint8_t)line[j];
      if (c == ' ') totalWidth += 3;
      else if (c == 'I' || c == '-') totalWidth += 4;
      else totalWidth += 6;
    }
    int currentX = (64 - (totalWidth - 1)) / 2;

    for (int j = 0; j < line.length(); j++) {
      uint8_t c = (uint8_t)line[j];

      if (c == ' ') {
        currentX += 3; 
      } else if (c == 'I' || c == '-') {
        dma_display->drawChar(currentX - 1, currentY, c, color, color, 1);
        currentX += 4; 
      } 
      // Intercept French Accents
      else if (c == 0x90 || c == 0x93 || c == 0x8a) {
        int customType = (c == 0x90) ? TYPE_E_ACUTE : (c == 0x93 ? TYPE_O_CIRC : TYPE_E_GRAVE);
        drawCustomFrench(currentX, currentY, customType, color);
        currentX += 6;
        continue; 
      }
      else {
        dma_display->drawChar(currentX, currentY, c, color, color, 1);
        currentX += 6;
      }
    }
  }
}