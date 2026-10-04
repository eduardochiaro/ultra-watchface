#include "complications.h"
#include "../settings.h"
#include "../weather.h"

// Ground elevation in the unit setting, no unit; false and "--" before any weather.
static bool elevation_text(char *buf, size_t n) {
  if (!g_weather.valid) {
    snprintf(buf, n, "--");
    return false;
  }
  snprintf(buf, n, "%d", g_settings.imperial ? g_weather.elevation * 3281 / 1000 : g_weather.elevation);
  return true;
}

// No bar, there is nothing to fill against: the arrow and "290m", like the heart.
void comp_elevation_draw(GContext *ctx, const Slot *s) {
  char buf[12];
  if (elevation_text(buf, sizeof(buf) - 2)) strcat(buf, g_settings.imperial ? "ft" : "m");
  comp_icon_text(ctx, s, ICON_ARROW, GColorRed, buf);
}

// Ground height where the phone is: arrow and unit above a red pill holding
// the number, bars fading below.
void center_elevation_draw(GContext *ctx, GPoint c) {
  char buf[12];
  elevation_text(buf, sizeof(buf));
  const char *unit = g_settings.imperial ? "FT" : "M";
  int ux = c.x + 3, ax = ux - text_width(ctx, unit, SUB_SMALL) / 2 - 5, ay = c.y - 11;
  text_draw(ctx, ICON_ARROW, GPoint(ax, ay), 11, GColorRed);
  text_draw(ctx, unit, GPoint(ux, ay), SUB_SMALL, GColorWhite);
  rect_fill(ctx, GRect(c.x - SUB_R + 1, c.y - 5, 2 * SUB_R - 2, 12), 4, GColorRed);
  text_draw(ctx, buf, GPoint(c.x, c.y + 1), SUB_TEXT, ink_on(GColorRed));
  static const struct { int8_t hw; uint8_t argb; } BARS[] = {
    { 10, GColorDarkCandyAppleRedARGB8 }, { 7, GColorBulgarianRoseARGB8 },
  };
  for (unsigned i = 0; i < ARRAY_LENGTH(BARS); i++) {
    rect_fill(ctx, GRect(c.x - BARS[i].hw, c.y + 10 + 3 * i, 2 * BARS[i].hw, 2), 0, (GColor){ .argb = BARS[i].argb });
  }
}
