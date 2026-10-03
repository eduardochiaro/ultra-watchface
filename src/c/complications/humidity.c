#include "complications.h"
#include "../weather.h"

// Relative humidity now, laid out like the rain bar.
void comp_humidity_draw(GContext *ctx, const Slot *s) {
  int v = g_weather.valid ? g_weather.humidity : -1;
  char buf[8] = "--";
  if (v >= 0) snprintf(buf, sizeof(buf), "%d" CORNER_PCT, v);
  comp_fill_gauge(ctx, s, v >= 0 ? v : 0, GColorCyan, COMP_TRACK, buf, ICON_DROP);
}

// Like rain without the thumb: a drop under the value.
void center_humidity_draw(GContext *ctx, GPoint c) {
  int v = g_weather.valid ? g_weather.humidity : -1;
  char buf[8] = "--";
  if (v >= 0) snprintf(buf, sizeof(buf), "%d" SMALL_PCT, v);
  center_gauge(ctx, c, v, GColorCyan);
  center_fit_text(ctx, buf, GPoint(c.x, c.y - 1), SUB_TEXT + 2, 2 * SUB_R - SUB_T - 4, GColorWhite);
  text_draw(ctx, ICON_DROP, GPoint(c.x, c.y + SUB_LOW), 9, GColorCyan);
}
