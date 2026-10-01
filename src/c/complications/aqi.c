#include "complications.h"
#include "../weather.h"

// EPA band of a US AQI; white for unknown (<0).
static GColor aqi_color(int v) {
  static const struct { int16_t to; uint8_t argb; } BANDS[] = {
    { 50, GColorGreenARGB8 }, { 100, GColorYellowARGB8 }, { 150, GColorOrangeARGB8 },
    { 200, GColorRedARGB8 }, { 300, GColorPurpleARGB8 }, { INT16_MAX, GColorBulgarianRoseARGB8 },
  };
  if (v < 0) return GColorWhite;
  unsigned i = 0;
  while (v > BANDS[i].to) i++;
  return (GColor){ .argb = BANDS[i].argb };
}

// 0..300 along the bar in the band's color, "AQI" at its end; the value curved
// outside its middle, where the battery puts its percentage.
void comp_aqi_draw(GContext *ctx, const Slot *s) {
  int v = g_weather.valid ? g_weather.aqi : -1;
  char buf[8] = "--";
  if (v >= 0) snprintf(buf, sizeof(buf), "%d", v);
  comp_fill_gauge(ctx, s, v >= 0 ? v * 100 / 300 : 0, aqi_color(v), COMP_TRACK, "AQI", NULL);
  text_draw_along(ctx, buf, slot_point(s, 50, COMP_THUMB), s->center, COMP_TEXT + 2, GColorWhite);
}

// US AQI, 0..300 around the ring in the color of its EPA band.
void center_aqi_draw(GContext *ctx, GPoint c) {
  int v = g_weather.valid ? g_weather.aqi : -1;
  GColor color = aqi_color(v);
  char buf[8] = "--";
  if (v >= 0) snprintf(buf, sizeof(buf), "%d", v);
  center_gauge(ctx, c, v * 100 / 300, color);
  text_draw(ctx, buf, GPoint(c.x, c.y - 1), SUB_TEXT, color);
  text_draw(ctx, "AQI", GPoint(c.x, c.y + SUB_LOW), SUB_SMALL, GColorWhite);
}
