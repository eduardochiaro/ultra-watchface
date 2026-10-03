#include "complications.h"
#include "../settings.h"
#include "../weather.h"

// How warm, blue to red: every 64-color step around the hue wheel, one per
// 3°C, below 0 to 33 and up. The arc is a gradient of them from today's min to
// its max.
static const uint8_t SHADES[] = {
  GColorBlueMoonARGB8, GColorVividCeruleanARGB8, GColorCyanARGB8, GColorMediumSpringGreenARGB8,
  GColorMalachiteARGB8, GColorGreenARGB8, GColorBrightGreenARGB8, GColorSpringBudARGB8,
  GColorYellowARGB8, GColorChromeYellowARGB8, GColorOrangeARGB8, GColorRedARGB8,
};

#define STOPS 12  // samples along the arc

static void temp_shades(uint8_t out[STOPS]) {
  const Weather *w = &g_weather;
  for (int i = 0; i < STOPS; i++) {
    int t = w->temp_min + (w->temp_max - w->temp_min) * (2 * i + 1) / (2 * STOPS);
    int c = g_settings.imperial ? (t - 32) * 5 / 9 : t;
    out[i] = SHADES[clamp_i32((c + 3) / 3, 0, ARRAY_LENGTH(SHADES) - 1)];
  }
}

// Today's range as min and max, the thumb and the label at the current temperature.
static int temp_range(char min[8], char max[8], char now[8]) {
  const Weather *w = &g_weather;
  if (!w->valid) return -1;
  int span = w->temp_max - w->temp_min;
  snprintf(min, 8, "%d", w->temp_min);
  snprintf(max, 8, "%d", w->temp_max);
  snprintf(now, 8, "%d°", w->temp);
  return span > 0 ? clamp_i32((w->temp - w->temp_min) * 100 / span, 0, 100) : 50;
}

void comp_temp_draw(GContext *ctx, const Slot *s) {
  char min[8] = "", max[8] = "", now[8] = "--";
  uint8_t fill[STOPS];
  int pct = temp_range(min, max, now);
  temp_shades(fill);
  comp_range_draw(ctx, s, pct, min, max, now, fill, STOPS);
}

void center_temp_draw(GContext *ctx, GPoint c) {
  char min[8] = "", max[8] = "", now[8] = "--";
  uint8_t fill[STOPS];
  int pct = temp_range(min, max, now);
  temp_shades(fill);
  center_range_draw(ctx, c, pct, min, max, now, NULL, fill, STOPS);
}
