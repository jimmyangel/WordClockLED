#include "TimeWords.h"
#include "constants.h"

String TimeWords::preText[4][2] = {
    {"ES", "SON"}, 
    {"IT'S", "IT'S"}, 
    {"C'EST", "C'EST"},
    {"ES IST", "ES IST"} 
};

String TimeWords::postText[4][1] = {
    {"EN PUNTO"}, 
    {"O'CLOCK"}, 
    {"PILE"},
    {"UHR"}
};

String TimeWords::articleText[4][2] = {
    {"LA", "LAS"}, 
    {"", ""}, 
    {"", ""},
    {"", ""}
};

String TimeWords::fractionText[4][4] = {
    {"PASADAS", "BIEN PASADAS", "CASI", "CASI CASI"},
    {"A LITTLE BIT OVER", "A BIT OVER", "ALMOST", "NEARLY"},
    {"PASS\x90\x45S", "BIEN PASS\x90\x45S", "BIENT\x93T", "TOUT PR\x8aS"},
    {"KURZ NACH", "EIN BISSCHEN NACH", "GLEICH", "FAST"} 
};

String TimeWords::numbersText[4][12] = {
    {"UNA", "DOS", "TRES", "CUATRO", "CINCO", "SEIS", "SIETE", "OCHO", "NUEVE", "DIEZ", "ONCE", "DOCE"},
    {"ONE", "TWO", "THREE", "FOUR", "FIVE", "SIX", "SEVEN", "EIGHT", "NINE", "TEN", "ELEVEN", "TWELVE"},
    {"", "", "", "", "", "", "", "", "", "", "", ""},
    {"EINS", "ZWEI", "DREI", "VIER", "FÜNF", "SECHS", "SIEBEN", "ACHT", "NEUN", "ZEHN", "ELF", "ZWÖLF"}
};

String TimeWords::numbersFrench24[24] = {
    "MINUIT", "UNE HEURE", "DEUX HEURES", "TROIS HEURES", "QUATRE HEURES", "CINQ HEURES", 
    "SIX HEURES", "SEPT HEURES", "HUIT HEURES", "NEUF HEURES", "DIX HEURES", "ONZE HEURES", 
    "MIDI", "TREIZE HEURES", "QUATORZE HEURES", "QUINZE HEURES", "SEIZE HEURES", "DIX-SEPT HEURES", 
    "DIX-HUIT HEURES", "DIX-NEUF HEURES", "VINGT HEURES", "VINGT ET UNE HEURES", "VINGT-DEUX HEURES", "VINGT-TROIS HEURES"
};

String TimeWords::numbersModText[4][12] = {
    {"Y CINCO", "Y DIEZ", "Y CUARTO", "Y VEINTE", "Y VEINTICINCO", "Y MEDIA", "VEINTICINCO PARA", "VEINTE PARA", "UN CUARTO PARA", "DIEZ PARA", "CINCO PARA", ""},
    {"FIVE AFTER", "TEN AFTER", "QUARTER PAST", "TWENTY AFTER", "TWENTY-FIVE AFTER", "HALF PAST", "TWENTY-FIVE TO", "TWENTY TO", "QUARTER TO", "TEN TO", "FIVE TO", ""},
    {"CINQ", "DIX", "ET QUART", "VINGT", "VINGT-CINQ", "ET DEMIE", "MOINS VINGT-CINQ", "MOINS VINGT", "MOINS LE QUART", "MOINS DIX", "MOINS CINQ", ""},
    {"FÜNF NACH", "ZEHN NACH", "VIERTEL NACH", "ZWANZIG NACH", "FÜNF VOR HALB", "HALB", "FÜNF NACH HALB", "ZWANZIG VOR", "VIERTEL VOR", "ZEHN VOR", "FÜNF VOR", ""}
};

String TimeWords::fT = "";

String &TimeWords::getWords(int hour, int minutes, int lang) {
  // 1. Process 12-Hour vs 24-Hour Time Layout Base
  int h;
  if (lang == FRENCH) {
      h = hour; 
  } else {
      h = (hour > 12) ? hour - 12 : hour;
      h = (h == 0) ? 12 : h;
  }

  int mF = ((float) minutes / 5) + 0.5;
  int mFf = minutes % 5;
  
  // 2. Hour flip logic
  // German shifts to target hour (h++) starting at minute 25 (mF == 5 -> FÜNF VOR HALB)
  bool hourFlip = (lang == GERMAN) ? (mF >= 5) : (mF > 6);
  
  if (hourFlip) {
    h++; 
    if (lang == FRENCH) {
        if (h == 24) h = 0; 
    } else {
        if (h == 13) h = 1;
    }
  }

  // 3. Determine Opening Fragment
  String opening;
  if (lang == FRENCH) {
      opening = (h == 1 || h == 12 || h == 0) ? preText[lang][0] : preText[lang][1];
  } else {
      opening = (h == 1 && mF <= 6 && lang == SPANISH) ? preText[lang][0] : preText[lang][1];
  }
  fT = opening + " ";

  // 4. Handle Prefix Modifiers (+1, +2 "NACH" or -2, -1 "BEFORE" target anchor)
  if ((lang == ENGLISH || lang == GERMAN) && mFf > 0) { 
      fT += fractionText[lang][mFf - 1] + " "; 
  }
  if ((lang == SPANISH || lang == FRENCH) && (mFf == 3 || mFf == 4)) { 
      fT += fractionText[lang][mFf - 1] + " "; 
  }

  String aT = (h == 1) ? articleText[lang][0] : articleText[lang][1];
  
  // 5. Fetch Hour String safely
  String nT;
  if (lang == FRENCH) {
      nT = numbersFrench24[h]; 
  } else {
      nT = numbersText[lang][h - 1];
  }

  String nMT = (mF == 0) ? "" : numbersModText[lang][mF - 1];

  // 6. Construction Logic
  if (lang == FRENCH) { 
      fT += nT;
      if (mF != 0) fT += " " + nMT;
  } 
  else {
      if (mF > 6 && lang == SPANISH) { fT += nMT + " "; } 
      if (mF >= 1 && (lang == ENGLISH || lang == GERMAN)) { fT += nMT + " "; }
      
      if (aT != "") fT += aT + " ";
      fT += nT;

      if (mF <= 6 && lang == SPANISH) { fT += " " + nMT; }
  }

  // 7. Suffix Application
  if ((lang == SPANISH || lang == FRENCH) && (mFf == 1 || mFf == 2)) {
      fT += " " + fractionText[lang][mFf - 1]; 
  }

  if (minutes == 0) { fT += " " + postText[lang][0]; }

  // 8. French Refinements
  if (lang == FRENCH) {
      fT.replace("MIDI HEURES", "MIDI");
      fT.replace("MINUIT HEURES", "MINUIT");
      if (fT.indexOf("MIDI") >= 0 || fT.indexOf("MINUIT") >= 0) {
          fT.replace("PASS\x90\x45S", "PASS\x90"); 
      }
      if (fT.length() > 45) { 
          fT.replace(preText[FRENCH][1], ""); 
      }
  }
  
  // 9. German Grammar Refinements
  if (lang == GERMAN) {
      // Fix: "ES IST EINS UHR" -> "ES IST EIN UHR"
      fT.replace("EINS UHR", "EIN UHR");
  }
  
  // Cleanup whitespace
  while(fT.indexOf("  ") >= 0) fT.replace("  ", " ");
  fT.trim();
  
  return fT;
}