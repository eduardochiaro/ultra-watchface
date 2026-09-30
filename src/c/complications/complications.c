#include "complications.h"

typedef void (*ComplicationDraw)(GContext *ctx, const Slot *s);

static const ComplicationDraw DRAW[COMP_COUNT] = {
  [COMP_STEPS]   = comp_steps_draw,
  [COMP_TEMP]    = comp_temp_draw,
  [COMP_BATTERY] = comp_battery_draw,
  [COMP_RAIN]    = comp_rain_draw,
  [COMP_SUN]     = comp_sun_draw,
};

void complication_draw(ComplicationId id, GContext *ctx, const Slot *s) {
  if (id < COMP_COUNT && DRAW[id]) DRAW[id](ctx, s);
}

// Bar: value label on the left, filling from the left.
void comp_fill_gauge(GContext *ctx, const Slot *s, int pct, GColor fill, GColor track,
                     const char *label) {
  pct = clamp_i32(pct, 0, 100);
  int left = slot_left_end(s);
  Slot b = *s;
  comp_end_label(ctx, &b, left, label);
  slot_arc(ctx, &b, 0, 100, track);
  if (pct > 0) slot_arc(ctx, &b, left, left ? 100 - pct : pct, fill);
  comp_icon(ctx, s, fill);
}

// Label laid along the arc at an end; the arc is trimmed to make room so
// label + arc stay inside the slot's original span.
void comp_end_label(GContext *ctx, Slot *s, int end, const char *txt) {
  int w = text_width(ctx, txt, COMP_TEXT);
  text_draw_along(ctx, txt, slot_past(s, end, -w / 2), s->center, COMP_TEXT, GColorWhite);
  slot_trim(s, end, w + COMP_GAP + s->thickness / 2);
}

// Outside the arc, toward the screen corner.
void comp_icon(GContext *ctx, const Slot *s, GColor color) {
  icon_block(ctx, slot_point(s, 50, s->thickness / 2 + COMP_GAP + COMP_ICON / 2), COMP_ICON, color);
}
