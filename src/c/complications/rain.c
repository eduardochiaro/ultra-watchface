#include "complications.h"
#include "../weather.h"

void comp_rain_draw(GContext *ctx, const Slot *s) {
  char buf[8] = "--";
  if (g_weather.valid) snprintf(buf, sizeof(buf), "%d%%", g_weather.rain);
  comp_fill_gauge(ctx, s, g_weather.valid ? g_weather.rain : 0,
                  GColorPictonBlue, GColorOxfordBlue, buf, ICON_UMBRELLA);
}

// A thumb at the fill's end; the % sits by the umbrella, not the value.
void center_rain_draw(GContext *ctx, GPoint c) {
  char buf[8] = "--";
  if (g_weather.valid) {
    Slot s = center_ring(c);
    center_gauge(ctx, c, g_weather.rain, GColorPictonBlue);
    slot_dot(ctx, &s, clamp_i32(g_weather.rain, 0, 100), SUB_T / 2 + 1, GColorCeleste, GColorBlack);
    snprintf(buf, sizeof(buf), "%d", g_weather.rain);
  } else {
    center_gauge(ctx, c, 0, GColorPictonBlue);
  }
  text_draw(ctx, buf, GPoint(c.x, c.y - 1), SUB_TEXT + 2, GColorWhite);
  text_draw(ctx, ICON_UMBRELLA, GPoint(c.x - 2, c.y + SUB_LOW), 9, GColorWhite);
  text_draw(ctx, "%", GPoint(c.x + 5, c.y + SUB_LOW + 2), SUB_SMALL - 2, GColorWhite);
}
