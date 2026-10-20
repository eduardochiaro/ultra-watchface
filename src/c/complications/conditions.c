#include "complications.h"
#include "../settings.h"
#include "../weather.h"

// The conditions now: their icon, in a corner their name too. The Icon + temp
// kind adds the temperature. Any bright color themes to the accent; white stays
// the text color.
static GColor icon_color(void) {
  return g_settings.scheme == SCHEME_ACCENT ? GColorChromeYellow : GColorWhite;
}

// The icon toward the corner, "21° CLOUDY" curved along the arc (gabbro: the icon leads it).
static void corner(GContext *ctx, const Slot *s, bool temp) {
  const char *icon = weather_icon();
  if (!icon) {
    text_draw_along(ctx, "--", slot_point(s, 50, 0), s->center, COMP_TEXT + 2, GColorWhite);
    return;
  }
  char buf[20];
  if (temp) {
    snprintf(buf, sizeof(buf), "%d° %s", g_weather.temp, weather_name());
  } else {
    snprintf(buf, sizeof(buf), "%s", weather_name());
  }
  comp_icon_text(ctx, s, icon, icon_color(), buf);
}

void comp_weather_draw(GContext *ctx, const Slot *s) {
  corner(ctx, s, false);
}

void comp_weather_temp_draw(GContext *ctx, const Slot *s) {
  corner(ctx, s, true);
}

// One icon, no ring.
void center_weather_draw(GContext *ctx, GPoint c) {
  const char *icon = weather_icon();
  if (icon) {
    text_draw(ctx, icon, c, 7 * SUB_R / 4, icon_color());  // 28px on emery
  } else {
    text_draw(ctx, "--", c, SUB_TEXT, icon_color());
  }
}

// The icon smaller, the temperature under it.
void center_weather_temp_draw(GContext *ctx, GPoint c) {
  const char *icon = weather_icon();
  if (!icon) {
    text_draw(ctx, "--", c, SUB_TEXT, icon_color());
    return;
  }
  char buf[8];
  snprintf(buf, sizeof(buf), "%d°", g_weather.temp);
  text_draw(ctx, icon, GPoint(c.x, c.y - SUB_R / 3), SUB_R, icon_color());
  text_draw(ctx, buf, GPoint(c.x, c.y + SUB_R / 2 + 1), SUB_TEXT, GColorWhite);
}
