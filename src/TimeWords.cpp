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
    {"PASS\x90\x45S", "BIEN PASS\x90\x45S", "BIENT\x93T", "TOUT PR\x8aS"} // PASSÉES, BIENTÔT, PRÈS
};

// Index 2 (French) is cleared out here because it uses numbersFrench24 instead
String TimeWords::numbersText[3][12] = {
    {"UNA", "DOS", "TRES", "CUATRO", "CINCO", "SEIS", "SIETE", "OCHO", "NUEVE", "DIEZ", "ONCE", "DOCE"},
    {"ONE", "TWO", "THREE", "FOUR", "FIVE", "SIX", "SEVEN", "EIGHT", "NINE", "TEN", "ELEVEN", "TWELVE"},
    {"", "", "", "", "", "", "", "", "", "", "", ""} 
};

// Continuous 0-23 lookup array for French grammar requirements
String TimeWords::numbersFrench24[24] = {
    "MINUIT", "UNE HEURE", "DEUX HEURES", "TROIS HEURES", "QUATRE HEURES", "CINQ HEURES", 
    "SIX HEURES", "SEPT HEURES", "HUIT HEURES", "NEUF HEURES", "DIX HEURES", "ONZE HEURES", 
    "MIDI", "TREIZE HEURES", "QUATORZE HEURES", "QUINZE HEURES", "SEIZE HEURES", "DIX-SEPT HEURES", 
    "DIX-HUIT HEURES", "DIX-NEUF HEURES", "VINGT HEURES", "VINGT ET UNE HEURES", "VINGT-DEUX HEURES", "VINGT-TROIS HEURES"
};

String TimeWords::numbersModText[3][12] = {
    {"Y CINCO", "Y DIEZ", "Y CUARTO", "Y VEINTE", "Y VEINTICINCO", "Y MEDIA", "VEINTICINCO PARA", "VEINTE PARA", "UN CUARTO PARA", "DIEZ PARA", "CINCO PARA", ""},
    {"FIVE AFTER", "TEN AFTER", "QUARTER PAST", "TWENTY AFTER", "TWENTY-FIVE AFTER", "HALF PAST", "TWENTY-FIVE TO", "TWENTY TO", "QUARTER TO", "TEN TO", "FIVE TO", ""},
    {"CINQ", "DIX", "ET QUART", "VINGT", "VINGT-CINQ", "ET DEMIE", "MOINS VINGT-CINQ", "MOINS VINGT", "MOINS LE QUART", "MOINS DIX", "MOINS CINQ", ""}
};

// Define static internal tracking string
String TimeWords::fT = "";

String &TimeWords::getWords(int hour, int minutes, int lang) {
  // 1. Process 12-Hour vs 24-Hour Time Layout Base
  int h;
  if (lang == FRENCH) {
      h = hour; // Keep raw 24-hour input directly
  } else {
      h = (hour > 12) ? hour - 12 : hour;
      h = (h == 0) ? 12 : h;
  }

  int mF = ((float) minutes / 5) + 0.5;
  int mFf = minutes % 5;
  
  // 2. Hour flip for Subtraction phase (e.g., 14:33 rolls forward to 15 hours minus...)
  if (mF > 6) {
    h++; 
    if (lang == FRENCH) {
        if (h == 24) h = 0; // Seamlessly roll 23:35+ over into Midnight (Index 0)
    } else {
        if (h == 13) h = 1;
    }
  }

  // 3. Determine Opening Fragment (ES/SON, IT'S, C'EST)
  String opening;
  if (lang == FRENCH) {
      // Singular indicator parsing for 1, Midi (12), and Minuit (0)
      opening = (h == 1 || h == 12 || h == 0) ? preText[lang][0] : preText[lang][1];
  } else {
      opening = (h == 1 && mF <= 6) ? preText[lang][0] : preText[lang][1];
  }
  fT = opening + " ";

  // 4. Handle Prefix Modifiers
  if (lang == ENGLISH && mFf > 0) { 
      fT += fractionText[lang][mFf - 1] + " "; 
  }
  if ((lang == SPANISH || lang == FRENCH) && (mFf == 3 || mFf == 4)) { 
      fT += fractionText[lang][mFf - 1] + " "; 
  }

  String aT = (h == 1) ? articleText[lang][0] : articleText[lang][1];
  
  // 5. Fetch Hour String safely via separate tracking channels
  String nT;
  if (lang == FRENCH) {
      nT = numbersFrench24[h]; // Fast array pointer index
  } else {
      nT = numbersText[lang][h - 1];
  }

  String nMT = (mF == 0) ? "" : numbersModText[lang][mF - 1];

  // 6. Construction Logic
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

  // 7. Suffix Application (French/Spanish PASSEES/PASADAS)
  if ((lang == SPANISH || lang == FRENCH) && (mFf == 1 || mFf == 2)) {
      fT += " " + fractionText[lang][mFf - 1]; 
  }

  if (minutes == 0) { fT += " " + postText[lang][0]; }

  // 8. Final French Refinements & Grammar Cleanups
  if (lang == FRENCH) {
      fT.replace("MIDI HEURES", "MIDI");
      fT.replace("MINUIT HEURES", "MINUIT");

      // Grammar Fix: Masculine Singular for Midi/Minuit vs Feminine Plural for Heures
      if (fT.indexOf("MIDI") >= 0 || fT.indexOf("MINUIT") >= 0) {
          fT.replace("PASS\x90\x45S", "PASS\x90"); // "PASSÉES" -> "PASSÉ"
      }

      // Emergency length brake for the space-constrained display profile
      if (fT.length() > 45) { 
          fT.replace(preText[FRENCH][1], ""); 
      }
  }
  
  // Cleanup whitespace
  while(fT.indexOf("  ") >= 0) fT.replace("  ", " ");
  fT.trim();
  
  return fT;
}