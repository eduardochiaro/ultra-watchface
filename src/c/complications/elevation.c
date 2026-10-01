#include "complications.h"
#include "../settings.h"
#include "../weather.h"

bool elevation_text(char *buf, size_t n) {
  if (!g_weather.valid) {
    snprintf(buf, n, "--");
    return false;
  }
  snprintf(buf, n, "%d", g_settings.imperial ? g_weather.elevation * 3281 / 1000 : g_weather.elevation);
  return true;
}

// ponytail: nothing to fill against, so the bar is full; a scale could go here.
void comp_elevation_draw(GContext *ctx, const Slot *s) {
  char buf[12];
  bool known = elevation_text(buf, sizeof(buf) - 2);
  if (known) strcat(buf, g_settings.imperial ? "ft" : "m");
  comp_fill_gauge(ctx, s, known ? 100 : 0, GColorRed, COMP_TRACK, buf, ICON_ARROW);
}
