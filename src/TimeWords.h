#ifndef TIMEWORDS_H
#define TIMEWORDS_H

#include <Arduino.h> 
#include "constants.h"

class TimeWords  {
  public:
    String &getWords(int hour, int minutes, int lang);
  private: 
    static String preText[4][2];
    static String postText[4][1];
    static String articleText[4][2];
    static String fractionText[4][4];
    static String numbersText[4][12];
    static String numbersModText[4][12];
    static String numbersFrench24[24];
    static String fT;
};

#endif
