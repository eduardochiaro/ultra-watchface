#include "complications.h"
#include "../weather.h"

static bool sun_known(void) {
  return g_weather.valid && g_weather.sunrise != g_weather.sunset;
}

// "6:30", in the watch's 12 or 24h style. No am/pm: the arrow says which.
static void sun_time(char *buf, size_t n, int min) {
  int h = min / 60;
  if (!clock_is_24h_style()) h = (h + 11) % 12 + 1;
  snprintf(buf, n, "%d:%02d", h, min % 60);
}

// No bar: both times curved along the arc's middle, the sun toward the corner
// (gabbro: leading the text).
void comp_sun_draw(GContext *ctx, const Slot *s) {
  char up[8], down[8], buf[32] = "--";
  if (sun_known()) {
    sun_time(up, sizeof(up), g_weather.sunrise);
    sun_time(down, sizeof(down), g_weather.sunset);
    snprintf(buf, sizeof(buf), ICON_UP "%s " ICON_DOWN "%s", up, down);
  }
  comp_icon_text(ctx, s, ICON_SUN, GColorYellow, buf);
}

// The day around the ring, a thumb at now; the next of the two in the middle,
// its arrow below. At night the ring is empty.
void center_sun_draw(GContext *ctx, GPoint c) {
  char buf[8] = "--";
  bool day = false;
  if (sun_known()) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    int now = tm->tm_hour * 60 + tm->tm_min, up = g_weather.sunrise, down = g_weather.sunset;
    day = now >= up && now < down;
    center_gauge(ctx, c, day ? (now - up) * 100 / (down - up) : 0, GColorYellow);
    if (day) {
      Slot s = center_ring(c);
      slot_dot(ctx, &s, (now - up) * 100 / (down - up), SUB_T / 2 + 1, GColorWhite, GColorBlack);
    }
    sun_time(buf, sizeof(buf), day ? down : up);
  } else {
    center_gauge(ctx, c, 0, GColorYellow);
  }
  center_fit_text(ctx, buf, GPoint(c.x, c.y - 1), SUB_TEXT, 2 * SUB_R - SUB_T - 4, GColorWhite);
  text_draw(ctx, day ? ICON_DOWN : ICON_UP, GPoint(c.x, c.y + SUB_LOW), SUB_SMALL + 1, GColorYellow);
}
