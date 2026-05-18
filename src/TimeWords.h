#ifndef TIMEWORDS_H
#define TIMEWORDS_H

#include <Arduino.h> 
#include "constants.h"

class TimeWords  {
  public:
    String &getWords(int hour, int minutes, int lang);
  private: 
    static String preText[3][2];
    static String postText[3][1];
    static String articleText[3][2];
    static String fractionText[3][4];
    static String numbersText[3][12];
    static String numbersModText[3][12];
    static String numbersFrench24[24];
    static String fT;
};

#endif
