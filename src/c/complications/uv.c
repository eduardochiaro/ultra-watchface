#include "complications.h"
#include "../weather.h"

// WHO band of a UV index; white for unknown (<0).
static GColor uv_color(int v) {
  static const struct { int16_t to; uint8_t argb; } BANDS[] = {
    { 2, GColorGreenARGB8 }, { 5, GColorYellowARGB8 }, { 7, GColorOrangeARGB8 },
    { 10, GColorRedARGB8 }, { INT16_MAX, GColorPurpleARGB8 },
  };
  if (v < 0) return GColorWhite;
  unsigned i = 0;
  while (v > BANDS[i].to) i++;
  return (GColor){ .argb = BANDS[i].argb };
}

// 0..11 along the bar in the WHO band's color, laid out like the AQI.
void comp_uv_draw(GContext *ctx, const Slot *s) {
  int v = g_weather.valid ? g_weather.uv : -1;
  char buf[8] = "--";
  if (v >= 0) snprintf(buf, sizeof(buf), "%d", v);
  comp_fill_gauge(ctx, s, v >= 0 ? v * 100 / 11 : 0, uv_color(v), COMP_TRACK, "UV", NULL);
  text_draw_along(ctx, buf, slot_point(s, 50, COMP_THUMB), s->center, COMP_TEXT + 2, GColorWhite);
}

// UV index, 0..11 around the ring in its WHO band color.
void center_uv_draw(GContext *ctx, GPoint c) {
  int v = g_weather.valid ? g_weather.uv : -1;
  GColor color = uv_color(v);
  char buf[8] = "--";
  if (v >= 0) snprintf(buf, sizeof(buf), "%d", v);
  center_gauge(ctx, c, v * 100 / 11, color);
  text_draw(ctx, buf, GPoint(c.x, c.y - 1), SUB_TEXT, color);
  text_draw(ctx, "UV", GPoint(c.x, c.y + SUB_LOW), SUB_SMALL, GColorWhite);
}
