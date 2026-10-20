#pragma once
#include <pebble.h>

// The face's words. Same as FACE in src/pkjs/i18n.js, same order (npm test checks).
typedef struct {
  const char *days[7];      // short, Sunday first
  const char *months[12];   // in full
  const char *mons[12];     // short
  const char *week, *day;   // the Date heads: week number, day of year
  const char *weather[9];   // weather_icon() order
  const char *active, *sleep;
  const char *points[8];    // compass points from N, clockwise
} Words;

// In the LANG setting's language; Auto (0): the watch's, English if not one of these.
const Words *words(void);
