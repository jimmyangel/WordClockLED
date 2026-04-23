#include "DisplayTime.h"

TimeWords tW;

void displayTime(int hours, int minutes, int lang) {
  if (lang != SPANISH) lang = ENGLISH;
  
  String t = tW.getWords(hours, minutes, lang);
  t.trim(); // Remove leading/trailing spaces from the whole phrase

  String lines[6]; // Storage for split lines
  int lineCount = 0;
  int start = 0;
  int standardMax = 10; 

  // 1. SPLIT LOGIC (Determine lines and count)
  while (start < t.length() && lineCount < 6) {
    int end = t.indexOf(' ', start + 1);
    String firstWord = t.substring(start);

    // Exception for the extra-long words
    if (firstWord.startsWith("VEINTICINCO") || firstWord.startsWith("TWENTY-FIVE")) {
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

  // 2. VERTICAL CENTERING CALCULATION
  int lineHeight = 10;
  int totalTextHeight = (lineCount * lineHeight) - 2; 
  int yOffset = (64 - totalTextHeight) / 2;

  dma_display->setTextSize(1);
  uint16_t color = dma_display->color565(0, 255, 255); // Cyan

  // 3. DRAWING LOGIC (With tight kerning and word spacing)
  for (int i = 0; i < lineCount; i++) {
    String line = lines[i];
    int currentY = yOffset + (i * lineHeight);

    // Calculate total line width first for centering
    // Normal = 6px, Thin ('I', '-') = 4px, Word Space = 3px
    int totalWidth = 0;
    for(char c : line) {
      if (c == ' ') totalWidth += 3;
      else if (c == 'I' || c == '-') totalWidth += 4;
      else totalWidth += 6;
    }
    totalWidth -= 1; // Remove the trailing 1px padding from the math

    int currentX = (64 - totalWidth) / 2;
    if (currentX < 0) currentX = 0;

    for (char c : line) {
      if (c == ' ') {
        currentX += 3; // Tight word spacing for "TO ONE"
      } else if (c == 'I') {
        // Kerned 'I' - 4px slot, shifted left to center
        dma_display->drawChar(currentX - 1, currentY, c, color, color, 1);
        currentX += 4; 
      } else if (c == '-') {
        // THE DASH FIX: 
        // 1. Shifted left by only 1 pixel (instead of 2) to clear the 'Y'
        // 2. Advanced 5 pixels to leave room for the 'F'
        dma_display->drawChar(currentX - 1, currentY, c, color, color, 1); 
        currentX += 5; 
      } else {
        // Standard char - 6px slot
        dma_display->drawChar(currentX, currentY, c, color, color, 1);
        currentX += 6;
      }
    }
  }
}








