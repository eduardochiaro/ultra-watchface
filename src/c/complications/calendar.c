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
  GColor ink = ink_on(GColorChromeYellow);  // today's digits
  for (int d = -1; d <= 1; d++) {
    time_t day = now + d * SECONDS_PER_DAY;
    char buf[3];
    snprintf(buf, sizeof(buf), "%d", localtime(&day)->tm_mday);
    int pct = 50 + (left ? -d : d) * 22;
    slot_box(ctx, &b, pct - 9, pct + 9, 2, d ? GColorDarkGray : GColorChromeYellow);
    text_draw_along(ctx, buf, slot_point(&b, pct, 0), b.center, COMP_TEXT, d ? GColorLightGray : ink);
  }
}

// A tear-off page: weekday on a red header (the accent or mono ink in those
// schemes), the day below in black on white whatever the scheme.
void center_calendar_draw(GContext *ctx, GPoint c) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  int r = SUB_R + SUB_T / 2, cut = -r / 3;
  if (theme_light()) disc_fill(ctx, c, r + 1, r + 1, GColorLightGray);  // edge on white
  disc_fill(ctx, c, r, r, fixed(GColorWhite));
  disc_fill(ctx, c, r, cut, GColorRed);
  text_draw(ctx, DAYS[t->tm_wday], GPoint(c.x, c.y + (cut - r) / 2), SUB_SMALL - 1, ink_on(GColorRed));
  char buf[3];
  snprintf(buf, sizeof(buf), "%d", t->tm_mday);
  text_draw(ctx, buf, GPoint(c.x, c.y + (cut + r) / 2), SUB_TEXT + 2, fixed(GColorBlack));
}
