#include "complications.h"
#include "../weather.h"

void comp_rain_draw(GContext *ctx, const Slot *s) {
  char buf[8] = "--";
  if (g_weather.valid) snprintf(buf, sizeof(buf), "%d%%", g_weather.rain);
  comp_fill_gauge(ctx, s, g_weather.valid ? g_weather.rain : 0,
                  GColorPictonBlue, GColorOxfordBlue, buf, ICON_UMBRELLA);
}
