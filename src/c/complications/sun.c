#include "complications.h"
#include "../weather.h"

// Day progress from sunrise (a0) to sunset (a1). Before sunrise the thumb
// rests at a0, after sunset at a1.
// ponytail: no night progress; add a sunset->sunrise run if the gauge should move at night.
static int day_pct(void) {
  if (!g_weather.valid || g_weather.sunset <= g_weather.sunrise) return 0;
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  int m = t->tm_hour * 60 + t->tm_min;
  return clamp_i32((m - g_weather.sunrise) * 100 / (g_weather.sunset - g_weather.sunrise), 0, 100);
}

// Sits just below an arc end.
static void end_badge(GContext *ctx, GPoint c, int r) {
  c.y += r + 2;
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_circle(ctx, c, r);
  graphics_context_set_stroke_color(ctx, GColorDarkGray);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_circle(ctx, c, r);
  icon_block(ctx, c, r, GColorWhite);
}

void comp_sun_draw(GContext *ctx, const Slot *s) {
  // Blue at dawn warming to orange by dusk.
  static const struct { int to; uint8_t argb; } BANDS[] = {
    { 40,  GColorPictonBlueARGB8 },
    { 70,  GColorPastelYellowARGB8 },
    { 100, GColorRajahARGB8 },
  };
  int pct = day_pct();

  slot_arc(ctx, s, 0, 100, COMP_TRACK);
  int from = 0;
  for (unsigned i = 0; i < ARRAY_LENGTH(BANDS) && from < pct; i++) {
    int to = BANDS[i].to < pct ? BANDS[i].to : pct;
    slot_arc(ctx, s, from, to, (GColor){ .argb = BANDS[i].argb });
    from = to;
  }

  int badge = s->thickness + 3;
  end_badge(ctx, slot_point(s, 0, 0), badge);
  end_badge(ctx, slot_point(s, 100, 0), badge);
  slot_dot(ctx, s, pct, s->thickness / 2 + 3, GColorWhite, GColorBlack);
}
