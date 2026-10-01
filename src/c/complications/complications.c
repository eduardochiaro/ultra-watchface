#include "complications.h"

typedef void (*ComplicationDraw)(GContext *ctx, const Slot *s);

static const ComplicationDraw DRAW[COMP_COUNT] = {
  [COMP_STEPS]   = comp_steps_draw,
  [COMP_TEMP]    = comp_temp_draw,
  [COMP_BATTERY] = comp_battery_draw,
  [COMP_RAIN]    = comp_rain_draw,
  [COMP_CALENDAR] = comp_calendar_draw,
  [COMP_HEART]   = comp_heart_draw,
  [COMP_DISTANCE] = comp_distance_draw,
  [COMP_AQI]     = comp_aqi_draw,
  [COMP_ELEVATION] = comp_elevation_draw,
  [COMP_UV]      = comp_uv_draw,
  [COMP_HUMIDITY] = comp_humidity_draw,
};

void complication_draw(ComplicationId id, GContext *ctx, const Slot *s) {
  if (id < COMP_COUNT && DRAW[id]) DRAW[id](ctx, s);
}

// Bar: value label on the left, filling from the left.
void comp_fill_gauge(GContext *ctx, const Slot *s, int pct, GColor fill, GColor track,
                     const char *label, const char *icon) {
  pct = clamp_i32(pct, 0, 100);
  int left = slot_left_end(s);
  Slot b = *s;
  comp_end_label(ctx, &b, left, label);
  slot_arc(ctx, &b, 0, 100, track);
  if (pct > 0) slot_arc(ctx, &b, left, left ? 100 - pct : pct, fill);
  if (icon) comp_icon(ctx, s, icon, fill);
}

// Label laid along the arc at an end; the arc is trimmed to make room so
// label + arc stay inside the slot's original span.
void comp_end_label(GContext *ctx, Slot *s, int end, const char *txt) {
  int w = text_width(ctx, txt, COMP_TEXT);
  text_draw_along(ctx, txt, slot_past(s, end, -w / 2), s->center, COMP_TEXT, GColorWhite);
  slot_trim(s, end, w + COMP_GAP + s->thickness / 2);
}

// Just past the arc, toward the screen corner; round screens have none, so
// at the arc's middle there.
void comp_icon(GContext *ctx, const Slot *s, const char *icon, GColor color) {
#if defined(PBL_ROUND)
  GPoint p = slot_point(s, 50, s->thickness / 2 + COMP_GAP + COMP_ICON / 2);
#else
  GPoint p = slot_corner(s, COMP_GAP + COMP_ICON / 2);
#endif
  text_draw(ctx, icon, p, COMP_ICON, color);
}
