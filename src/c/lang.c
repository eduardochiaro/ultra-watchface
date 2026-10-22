#include "lang.h"
#include "settings.h"

// In LANG order: LANG_CODES in src/pkjs/i18n.js.
static const char CODES[] = "enfrdenlites";

static const Words WORDS[] = {
  { // en
    { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" },
    { "JANUARY", "FEBRUARY", "MARCH", "APRIL", "MAY", "JUNE",
      "JULY", "AUGUST", "SEPTEMBER", "OCTOBER", "NOVEMBER", "DECEMBER" },
    { "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" },
    "WEEK", "DAY",
    { "SUNNY", "CLEAR", "PT CLOUDY", "PT CLOUDY", "CLOUDY", "FOGGY", "RAINY", "SNOWY", "STORMY" },
    "ACTIVE", "SLEEP",
    { "N", "NE", "E", "SE", "S", "SW", "W", "NW" },
    "DAYS LEFT", "DAYS AGO",
  },
  { // fr
    { "DIM", "LUN", "MAR", "MER", "JEU", "VEN", "SAM" },
    { "JANVIER", "FÉVRIER", "MARS", "AVRIL", "MAI", "JUIN",
      "JUILLET", "AOÛT", "SEPTEMBRE", "OCTOBRE", "NOVEMBRE", "DÉCEMBRE" },
    { "JAN", "FÉV", "MAR", "AVR", "MAI", "JUIN", "JUIL", "AOÛ", "SEP", "OCT", "NOV", "DÉC" },
    "SEM", "JOUR",
    { "SOLEIL", "DÉGAGÉ", "PEU NUAGEUX", "PEU NUAGEUX", "NUAGEUX", "BROUILLARD", "PLUIE", "NEIGE", "ORAGE" },
    "ACTIF", "SOMMEIL",
    { "N", "NE", "E", "SE", "S", "SO", "O", "NO" },
    "JOURS RESTANTS", "JOURS PASSÉS",
  },
  { // de
    { "SO", "MO", "DI", "MI", "DO", "FR", "SA" },
    { "JANUAR", "FEBRUAR", "MÄRZ", "APRIL", "MAI", "JUNI",
      "JULI", "AUGUST", "SEPTEMBER", "OKTOBER", "NOVEMBER", "DEZEMBER" },
    { "JAN", "FEB", "MÄR", "APR", "MAI", "JUN", "JUL", "AUG", "SEP", "OKT", "NOV", "DEZ" },
    "KW", "TAG",
    { "SONNIG", "KLAR", "HEITER", "HEITER", "BEWÖLKT", "NEBEL", "REGEN", "SCHNEE", "GEWITTER" },
    "AKTIV", "SCHLAF",
    { "N", "NO", "O", "SO", "S", "SW", "W", "NW" },
    "TAGE ÜBRIG", "TAGE HER",
  },
  { // nl
    { "ZO", "MA", "DI", "WO", "DO", "VR", "ZA" },
    { "JANUARI", "FEBRUARI", "MAART", "APRIL", "MEI", "JUNI",
      "JULI", "AUGUSTUS", "SEPTEMBER", "OKTOBER", "NOVEMBER", "DECEMBER" },
    { "JAN", "FEB", "MRT", "APR", "MEI", "JUN", "JUL", "AUG", "SEP", "OKT", "NOV", "DEC" },
    "WEEK", "DAG",
    { "ZONNIG", "HELDER", "HALF BEWOLKT", "HALF BEWOLKT", "BEWOLKT", "MIST", "REGEN", "SNEEUW", "ONWEER" },
    "ACTIEF", "SLAAP",
    { "N", "NO", "O", "ZO", "Z", "ZW", "W", "NW" },
    "DAGEN TE GAAN", "DAGEN GELEDEN",
  },
  { // it
    { "DOM", "LUN", "MAR", "MER", "GIO", "VEN", "SAB" },
    { "GENNAIO", "FEBBRAIO", "MARZO", "APRILE", "MAGGIO", "GIUGNO",
      "LUGLIO", "AGOSTO", "SETTEMBRE", "OTTOBRE", "NOVEMBRE", "DICEMBRE" },
    { "GEN", "FEB", "MAR", "APR", "MAG", "GIU", "LUG", "AGO", "SET", "OTT", "NOV", "DIC" },
    "SETT", "GIORNO",
    { "SOLE", "SERENO", "POCO NUVOLOSO", "POCO NUVOLOSO", "NUVOLOSO", "NEBBIA", "PIOGGIA", "NEVE", "TEMPORALE" },
    "ATTIVO", "SONNO",
    { "N", "NE", "E", "SE", "S", "SO", "O", "NO" },
    "GIORNI MANCANTI", "GIORNI FA",
  },
  { // es
    { "DOM", "LUN", "MAR", "MIÉ", "JUE", "VIE", "SÁB" },
    { "ENERO", "FEBRERO", "MARZO", "ABRIL", "MAYO", "JUNIO",
      "JULIO", "AGOSTO", "SEPTIEMBRE", "OCTUBRE", "NOVIEMBRE", "DICIEMBRE" },
    { "ENE", "FEB", "MAR", "ABR", "MAY", "JUN", "JUL", "AGO", "SEP", "OCT", "NOV", "DIC" },
    "SEM", "DÍA",
    { "SOLEADO", "DESPEJADO", "POCO NUBOSO", "POCO NUBOSO", "NUBLADO", "NIEBLA", "LLUVIA", "NIEVE", "TORMENTA" },
    "ACTIVO", "SUEÑO",
    { "N", "NE", "E", "SE", "S", "SO", "O", "NO" },
    "DÍAS RESTANTES", "DÍAS ATRÁS",
  },
};

const Words *words(void) {
  int i = g_settings.lang - 1;
  if (i < 0 || i >= (int)ARRAY_LENGTH(WORDS)) {
    const char *locale = i18n_get_system_locale();  // "fr_FR"
    i = ARRAY_LENGTH(WORDS) - 1;
    while (i > 0 && strncmp(locale, CODES + 2 * i, 2) != 0) {
      i--;  // down to English
    }
  }
  return &WORDS[i];
}
