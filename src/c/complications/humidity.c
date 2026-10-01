#include "complications.h"
#include "../weather.h"

// Relative humidity now, laid out like the rain bar.
void comp_humidity_draw(GContext *ctx, const Slot *s) {
  int v = g_weather.valid ? g_weather.humidity : -1;
  char buf[8] = "--";
  if (v >= 0) snprintf(buf, sizeof(buf), "%d%%", v);
  comp_fill_gauge(ctx, s, v >= 0 ? v : 0, GColorCyan, COMP_TRACK, buf, ICON_DROP);
}

// Like rain without the thumb: a drop by the %.
void center_humidity_draw(GContext *ctx, GPoint c) {
  int v = g_weather.valid ? g_weather.humidity : -1;
  char buf[8] = "--";
  if (v >= 0) snprintf(buf, sizeof(buf), "%d", v);
  center_gauge(ctx, c, v, GColorCyan);
  text_draw(ctx, buf, GPoint(c.x, c.y - 1), SUB_TEXT + 2, GColorWhite);
  text_draw(ctx, ICON_DROP, GPoint(c.x - 2, c.y + SUB_LOW), 9, GColorCyan);
  text_draw(ctx, "%", GPoint(c.x + 5, c.y + SUB_LOW + 2), SUB_SMALL - 2, GColorWhite);
}
