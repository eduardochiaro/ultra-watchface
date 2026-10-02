#include "complications.h"

// Swatch .beat time: the day in 1000 beats, from midnight UTC+1. Red, so the
// accent in that scheme.
// ponytail: redrawn with the face, so up to a minute (0.7 beat) late.
static int beats(void) {
  return (time(NULL) + 3600) % 86400 * 10 / 864;
}

// No bar: ".042" curved along the arc's middle, a big @ toward the corner
// (gabbro: leading the text).
void comp_beat_draw(GContext *ctx, const Slot *s) {
  char buf[8];
  snprintf(buf, sizeof(buf), ".%03d", beats());
#if defined(PBL_PLATFORM_GABBRO)
  comp_icon_text(ctx, s, "@", GColorRed, buf);
#else
  text_draw_along(ctx, buf, slot_point(s, 50, 0), s->center, COMP_TEXT + 2, GColorWhite);
  text_draw(ctx, "@", slot_corner(s, COMP_GAP + COMP_ICON / 2), COMP_ICON - 4, GColorRed);
#endif
}

// No ring: the beats, a small @ under them.
void center_beat_draw(GContext *ctx, GPoint c) {
  char buf[8];
  snprintf(buf, sizeof(buf), ".%03d", beats());
  text_draw(ctx, buf, GPoint(c.x, c.y - 3), SUB_TEXT + 2, GColorWhite);
  text_draw(ctx, "@", GPoint(c.x, c.y + SUB_LOW), SUB_SMALL + 1, GColorRed);
}
