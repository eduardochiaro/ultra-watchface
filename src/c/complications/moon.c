#include "complications.h"

#define SYNODIC  2551443    // seconds from one new moon to the next
#define NEW_MOON 947182440  // one of them: 2000-01-06 18:14 UTC

// Where the moon is in its cycle, as a TRIG angle: 0 new, half the circle full.
// ponytail: the mean cycle, up to half a day off the real moon.
static int32_t moon_phase(void) {
  return (int64_t)((time(NULL) - NEW_MOON) % SYNODIC) * TRIG_MAX_ANGLE / SYNODIC;
}

// No bar: the moon and how much of it is lit, an arrow up while it grows.
void comp_moon_draw(GContext *ctx, const Slot *s) {
  int32_t phase = moon_phase();
  int lit = (TRIG_MAX_RATIO - cos_lookup(phase)) * 50 / TRIG_MAX_RATIO;
  char buf[16];
  snprintf(buf, sizeof(buf), "%s%d" CORNER_PCT, phase < TRIG_MAX_ANGLE / 2 ? ICON_UP : ICON_DOWN, lit);
  comp_icon_text(ctx, s, ICON_MOON, GColorPastelYellow, buf);
}

// The moon as it looks tonight, in its own colors in any scheme.
// ponytail: as seen from the northern hemisphere; mirror it for the southern if wanted.
void center_moon_draw(GContext *ctx, GPoint c) {
  const int r = SUB_R * 3 / 4;  // about a weather icon's size
  if (theme_light()) {
    disc_fill(ctx, c, r + 1, r + 1, GColorLightGray);  // edge on white
  }
  disc_fill(ctx, c, r, r, fixed(GColorDarkGray));
  moon_fill(ctx, c, r, moon_phase(), fixed(GColorWhite));
}
