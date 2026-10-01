#include "complications.h"
#include "../weather.h"

// Gauge: today's range with a thumb at the current temperature, labelled at
// the arc's middle. Min on the left end, max on the right. Gabbro: no gauge,
// only the current temperature at the arc's middle.
void comp_temp_draw(GContext *ctx, const Slot *slot) {
  Slot b = *slot, *s = &b;
  const Weather *w = &g_weather;
  char buf[12] = "--";
#if defined(PBL_PLATFORM_GABBRO)
  if (w->valid) snprintf(buf, sizeof(buf), "%d°", w->temp);
  text_draw_along(ctx, buf, slot_point(s, 50, 0), s->center, COMP_TEXT + 2, GColorWhite);
#else
  if (!w->valid) {
    slot_arc(ctx, s, 0, 100, COMP_TRACK);
    text_draw_along(ctx, buf, slot_point(s, 50, COMP_THUMB), s->center, COMP_TEXT, GColorWhite);
    return;
  }
  int lo = slot_left_end(s);
  int span = w->temp_max - w->temp_min;
  int pct = span > 0 ? clamp_i32((w->temp - w->temp_min) * 100 / span, 0, 100) : 50;
  if (lo) pct = 100 - pct;

  snprintf(buf, sizeof(buf), "%d", w->temp_min);
  comp_end_label(ctx, s, lo, buf);
  snprintf(buf, sizeof(buf), "%d", w->temp_max);
  comp_end_label(ctx, s, 100 - lo, buf);
  slot_arc(ctx, s, 0, 100, GColorChromeYellow);
  slot_dot(ctx, s, pct, s->thickness / 2 + 2, GColorWhite, GColorBlack);
  snprintf(buf, sizeof(buf), "%d°", w->temp);
  // Centered, not on the thumb: near min/max it would run off the arc end.
  text_draw_along(ctx, buf, slot_point(s, 50, COMP_THUMB), s->center, COMP_TEXT + 2, GColorWhite);
#endif
}

// Today's range in one color, a thumb at now, min and max under the value.
void center_temp_draw(GContext *ctx, GPoint c) {
  Slot s = center_ring(c);
  const Weather *w = &g_weather;
  char buf[12] = "--";
  if (!w->valid) {
    slot_arc(ctx, &s, 0, 100, COMP_TRACK);
  } else {
    int span = w->temp_max - w->temp_min;
    slot_arc(ctx, &s, 0, 100, GColorChromeYellow);
    slot_dot(ctx, &s, span > 0 ? clamp_i32((w->temp - w->temp_min) * 100 / span, 0, 100) : 50,
             SUB_T / 2 + 1, GColorWhite, GColorBlack);
    snprintf(buf, sizeof(buf), "%d", w->temp_min);
    text_draw(ctx, buf, GPoint(c.x - 7, c.y + SUB_LOW), SUB_SMALL, GColorLightGray);
    snprintf(buf, sizeof(buf), "%d", w->temp_max);
    text_draw(ctx, buf, GPoint(c.x + 7, c.y + SUB_LOW), SUB_SMALL, GColorLightGray);
    snprintf(buf, sizeof(buf), "%d°", w->temp);
  }
  text_draw(ctx, buf, GPoint(c.x, c.y - 1), SUB_TEXT, GColorWhite);
}
