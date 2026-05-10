#include "TimeWords.h"
#include "constants.h"

// Index 0: ES, Index 1: EN, Index 2: FR
String TimeWords::preText[3][2] = {
    {"ES", "SON"}, 
    {"IT'S", "IT'S"}, 
    {"C'EST", "C'EST"}
};

String TimeWords::postText[3][1] = {
    {"EN PUNTO"}, 
    {"O'CLOCK"}, 
    {"PILE"}
};

String TimeWords::articleText[3][2] = {
    {"LA", "LAS"}, 
    {"", ""}, 
    {"", ""}
};

String TimeWords::fractionText[3][4] = {
    {"PASADAS", "BIEN PASADAS", "CASI", "CASI CASI"},
    {"A LITTLE BIT OVER", "A BIT OVER", "ALMOST", "NEARLY"},
    {"PASS\x90\x45S", "BIEN PASS\x90\x45S", "BIENT\x93T", "TOUT PR\x8aS"}
};

String TimeWords::numbersText[3][12] = {
    {"UNA", "DOS", "TRES", "CUATRO", "CINCO", "SEIS", "SIETE", "OCHO", "NUEVE", "DIEZ", "ONCE", "DOCE"},
    {"ONE", "TWO", "THREE", "FOUR", "FIVE", "SIX", "SEVEN", "EIGHT", "NINE", "TEN", "ELEVEN", "TWELVE"},
    {"UNE HEURE", "DEUX HEURES", "TROIS HEURES", "QUATRE HEURES", "CINQ HEURES", "SIX HEURES", "SEPT HEURES", "HUIT HEURES", "NEUF HEURES", "DIX HEURES", "ONZE HEURES", "MIDI"}
};

String TimeWords::numbersModText[3][12] = {
    {"Y CINCO", "Y DIEZ", "Y CUARTO", "Y VEINTE", "Y VEINTICINCO", "Y MEDIA", "VEINTICINCO PARA", "VEINTE PARA", "UN CUARTO PARA", "DIEZ PARA", "CINCO PARA", ""},
    {"FIVE AFTER", "TEN AFTER", "QUARTER PAST", "TWENTY AFTER", "TWENTY-FIVE AFTER", "HALF PAST", "TWENTY-FIVE TO", "TWENTY TO", "QUARTER TO", "TEN TO", "FIVE TO", ""},
    {"CINQ", "DIX", "ET QUART", "VINGT", "VINGT-CINQ", "ET DEMIE", "MOINS VINGT-CINQ", "MOINS VINGT", "MOINS LE QUART", "MOINS DIX", "MOINS CINQ", ""}
};

String &TimeWords::getWords(int hour, int minutes, int lang) {
  int h = (hour > 12) ? hour - 12 : hour;
  h = (h == 0) ? 12 : h;
  int mF = ((float) minutes / 5) + 0.5;
  int mFf = minutes % 5;
  
  // Hour flip for Subtraction phase (e.g., 7:33 and up)
  if (mF > 6) {
    h++; 
    if (h == 13) h = 1;
  }

  // 1. Determine Opening (ES/SON, IT'S, C'EST)
  String opening = (h == 1 && mF <= 6) ? preText[lang][0] : preText[lang][1];
  fT = opening + " ";

  // 2. Handle Prefix 
  // English: ALL (A BIT OVER, ALMOST, etc.)
  if (lang == ENGLISH && mFf > 0) { 
      fT += fractionText[lang][mFf - 1] + " "; 
  }
  // Spanish/French: ONLY "Approaching" phrases (CASI / BIENTOT)
  if ((lang == SPANISH || lang == FRENCH) && (mFf == 3 || mFf == 4)) { 
      fT += fractionText[lang][mFf - 1] + " "; 
  }

  String aT = (h == 1) ? articleText[lang][0] : articleText[lang][1];
  
  // French MIDI/MINUIT Logic
  String nT;
  if (lang == FRENCH && h == 12) {
      // Use raw 'hour' to distinguish noon from midnight
      nT = (hour >= 11 && hour < 23) ? "MIDI" : "MINUIT";
  } else {
      nT = numbersText[lang][h - 1];
  }

  String nMT = (mF == 0) ? "" : numbersModText[lang][mF - 1];

  // 3. Construction Logic
  if (lang == FRENCH) { 
      // FRENCH ORDER: [Prefix] + [Hour] + [Minutes]
      fT += nT;
      if (mF != 0) fT += " " + nMT;
  } 
  else {
      // SPANISH/ENGLISH ORDER
      if (mF > 6 && lang == SPANISH) { fT += nMT + " "; } 
      if (mF >= 1 && lang == ENGLISH) { fT += nMT + " "; }
      
      if (aT != "") fT += aT + " ";
      fT += nT;

      if (mF <= 6 && lang == SPANISH) { fT += " " + nMT; }
  }

  // 4. Suffix (French/Spanish PASSEES/PASADAS)
  if ((lang == SPANISH || lang == FRENCH) && (mFf == 1 || mFf == 2)) {
      fT += " " + fractionText[lang][mFf - 1]; 
  }

  if (minutes == 0) { fT += " " + postText[lang][0]; }

  // 5. Final French Refinements
  if (lang == FRENCH) {
      // 5a. Handle Midi/Minuit vs Heures redundancy
      fT.replace("MIDI HEURES", "MIDI");
      fT.replace("MINUIT HEURES", "MINUIT");

      // 5b. Grammar Fix: Masculine Singular for Midi/Minuit
      if (fT.indexOf("MIDI") >= 0 || fT.indexOf("MINUIT") >= 0) {
          fT.replace("PASS\x90\x45S", "PASS\x90"); 
      }

      // 5c. Emergency length brake for the 64x64 display
      if (fT.length() > 45) { 
          // Removes "C'EST" to save vertical space
          fT.replace(preText[FRENCH][1], ""); 
      }
  }
  
  // Cleanup whitespace
  while(fT.indexOf("  ") >= 0) fT.replace("  ", " ");
  fT.trim();
  
  return fT;
}
