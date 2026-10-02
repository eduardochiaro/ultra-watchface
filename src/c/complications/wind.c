#include "complications.h"
#include "../settings.h"
#include "../weather.h"

// Wind speed in the unit setting, -1 before any reading.
static int wind_speed(void) {
  if (!g_weather.valid || g_weather.wind < 0) return -1;
  return g_settings.imperial ? g_weather.wind * 1000 / 1609 : g_weather.wind;
}

// Where it blows from, to the nearest of 8 points.
static const char *wind_from(void) {
  static const char *const POINT[] = { "N", "NE", "E", "SE", "S", "SW", "W", "NW" };
  return POINT[(g_weather.wind_dir * 2 + 45) / 90 % 8];
}

// No bar: "14km/h NW" curved along the arc's middle, the icon toward the corner
// (gabbro: leading the text).
void comp_wind_draw(GContext *ctx, const Slot *s) {
  int v = wind_speed();
  char buf[16] = "--";
  if (v >= 0) snprintf(buf, sizeof(buf), "%d%s %s", v, g_settings.imperial ? "mph" : "km/h", wind_from());
  comp_icon_text(ctx, s, ICON_WIND, GColorPictonBlue, buf);
}

// No ring: the speed over where it blows from, the icon below.
void center_wind_draw(GContext *ctx, GPoint c) {
  int v = wind_speed();
  char buf[8] = "--";
  if (v >= 0) snprintf(buf, sizeof(buf), "%d", v);
  text_draw(ctx, buf, GPoint(c.x, c.y - 7), SUB_TEXT + 1, GColorWhite);
  if (v >= 0) text_draw(ctx, wind_from(), GPoint(c.x, c.y + 2), SUB_SMALL, GColorPictonBlue);
  text_draw(ctx, ICON_WIND, GPoint(c.x, c.y + SUB_LOW + 2), 10, GColorPictonBlue);
}
