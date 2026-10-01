#include "complications.h"

// Yesterday, today and tomorrow as boxes along the arc, left to right, with
// the weekday outside. Today is in the accent, the others gray.
static const char *const DAYS[] = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };

void comp_calendar_draw(GContext *ctx, const Slot *slot) {
  time_t now = time(NULL);
  text_draw_along(ctx, DAYS[localtime(&now)->tm_wday], slot_point(slot, 50, COMP_THUMB + 2),
                  slot->center, COMP_TEXT + 2, GColorWhite);
  Slot b = *slot;
  b.thickness = COMP_TEXT + 5;
  int left = slot_left_end(&b);
  // Today's digits contrast with the accent: theme(Black) is light iff the scheme is.
  GColor ink = color_light(theme(GColorChromeYellow)) == theme_light() ? GColorWhite : GColorBlack;
  for (int d = -1; d <= 1; d++) {
    time_t day = now + d * SECONDS_PER_DAY;
    char buf[3];
    snprintf(buf, sizeof(buf), "%d", localtime(&day)->tm_mday);
    int pct = 50 + (left ? -d : d) * 22;
    slot_box(ctx, &b, pct - 9, pct + 9, 2, d ? GColorDarkGray : GColorChromeYellow);
    text_draw_along(ctx, buf, slot_point(&b, pct, 0), b.center, COMP_TEXT, d ? GColorLightGray : ink);
  }
}
