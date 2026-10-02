#include "complications.h"
#include "../weather.h"

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
  int pct = temp_range(min, max, now);
  comp_range_draw(ctx, s, pct, min, max, now);
}

void center_temp_draw(GContext *ctx, GPoint c) {
  char min[8] = "", max[8] = "", now[8] = "--";
  int pct = temp_range(min, max, now);
  center_range_draw(ctx, c, pct, min, max, now, NULL);
}
