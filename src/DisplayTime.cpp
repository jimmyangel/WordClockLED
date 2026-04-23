#include "DisplayTime.h"

TimeWords tW;

void displayTime(int hours, int minutes, int lang, uint16_t color) {
  if (lang != SPANISH) lang = ENGLISH;
  String t = tW.getWords(hours, minutes, lang);
  t.trim();

  String lines[6]; 
  int lineCount = 0;
  int start = 0;
  int standardMax = 10; 

  while (start < t.length() && lineCount < 6) {
    int end = t.indexOf(' ', start + 1);
    String firstWord = t.substring(start);
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

  int lineHeight = 10;
  int totalTextHeight = (lineCount * lineHeight) - 2; 
  int yOffset = (64 - totalTextHeight) / 2;

  dma_display->setTextSize(1);

  for (int i = 0; i < lineCount; i++) {
    String line = lines[i];
    int currentY = yOffset + (i * lineHeight);

    int totalWidth = 0;
    for(char c : line) {
      if (c == ' ') totalWidth += 3;
      else if (c == 'I' || c == '-') totalWidth += 4;
      else totalWidth += 6;
    }
    totalWidth -= 1;

    int currentX = (64 - totalWidth) / 2;
    if (currentX < 0) currentX = 0;

    for (char c : line) {
      if (c == ' ') {
        currentX += 3; 
      } else if (c == 'I' || c == '-') {
        dma_display->drawChar(currentX - 1, currentY, c, color, color, 1);
        currentX += 4; 
      } else {
        dma_display->drawChar(currentX, currentY, c, color, color, 1);
        currentX += 6;
      }
    }
  }
}









