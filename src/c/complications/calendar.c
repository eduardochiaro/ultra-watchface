#include "complications.h"

// Yesterday, today and tomorrow as boxes along the arc, left to right, with
// the weekday outside. Today is in light red, the others gray. Gabbro has no
// room beside the arc: the weekday leads the boxes, "WED 30 1 2".
static const char *const DAYS[] = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };

// Box centers apart and half a box, in % of the arc the boxes get.
#if defined(PBL_PLATFORM_GABBRO)
#define BOX_STEP 34
#define BOX_HALF 14
#else
#define BOX_STEP 22
#define BOX_HALF 9
#endif

void comp_calendar_draw(GContext *ctx, const Slot *slot) {
  time_t now = time(NULL);
  const char *wday = DAYS[localtime(&now)->tm_wday];
  Slot b = *slot;
  int left = slot_left_end(&b);
#if defined(PBL_PLATFORM_GABBRO)
  comp_end_label(ctx, &b, left, wday);
#else
  text_draw_along(ctx, wday, slot_point(slot, 50, COMP_THUMB + 2), slot->center, COMP_TEXT + 2, GColorWhite);
#endif
  b.thickness = COMP_TEXT + 5;
  GColor ink = ink_on(GColorSunsetOrange);  // today's digits
  for (int d = -1; d <= 1; d++) {
    time_t day = now + d * SECONDS_PER_DAY;
    char buf[3];
    snprintf(buf, sizeof(buf), "%d", localtime(&day)->tm_mday);
    int pct = 50 + (left ? -d : d) * BOX_STEP;
    slot_box(ctx, &b, pct - BOX_HALF, pct + BOX_HALF, 2, d ? GColorDarkGray : GColorMelon);
    text_draw_along(ctx, buf, slot_point(&b, pct, 0), b.center, COMP_TEXT, d ? GColorLightGray : ink);
  }
}

// A tear-off page: weekday on a red header (the accent or mono ink in those
// schemes), the day below in black on white whatever the scheme.
void center_calendar_draw(GContext *ctx, GPoint c) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  int r = SUB_R + SUB_T / 2, cut = -r / 3;
  if (theme_light()) {
    disc_fill(ctx, c, r + 1, r + 1, GColorLightGray);  // edge on white
  }
  disc_fill(ctx, c, r, r, fixed(GColorWhite));
  disc_fill(ctx, c, r, cut, GColorRed);
  text_draw(ctx, DAYS[t->tm_wday], GPoint(c.x, c.y + (cut - r) / 2), SUB_SMALL - 1, ink_on(GColorRed));
  char buf[3];
  snprintf(buf, sizeof(buf), "%d", t->tm_mday);
  text_draw(ctx, buf, GPoint(c.x, c.y + (cut + r) / 2), SUB_TEXT + 2, fixed(GColorBlack));
}

// No page: the weekday in red (the accent in that scheme) over the day.
void center_calendar_plain_draw(GContext *ctx, GPoint c) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  text_draw(ctx, DAYS[t->tm_wday], GPoint(c.x, c.y - 9), SUB_SMALL + 1, GColorOrange);
  char buf[3];
  snprintf(buf, sizeof(buf), "%d", t->tm_mday);
  text_draw(ctx, buf, GPoint(c.x, c.y + 4), SUB_TEXT + 5, GColorWhite);
}
