#include "complications.h"
#include "../lang.h"

// Days to the place's day, or since it, named by the place's text. The day is
// days since 1970-01-01 (config.js countDay()), 0 for none picked.

// Days since 1970-01-01 of the date in `t`.
static int epoch_day(const struct tm *t) {
  int y = t->tm_year + 1900 - 1;  // the years before it, for their leap days; 477 of them by 1969
  return 365 * (y - 1969) + y / 4 - y / 100 + y / 400 - 477 + t->tm_yday;
}

// The count into `value`, "--" with no day; its words, to come or gone, "" with none.
static const char *count(int day, char value[12]) {
  if (day <= 0) {
    strcpy(value, "--");
    return "";
  }
  time_t now = time(NULL);
  int n = day - epoch_day(localtime(&now));
  snprintf(value, 12, "%d", abs(n));
  return n >= 0 ? words()->left : words()->ago;
}

// Like the full date: "64 DAYS LEFT" on the arc, the name beside it. Gabbro
// has no room there: one line, "XMAS 64 DAYS LEFT".
void comp_countdown_draw(GContext *ctx, const Slot *s, int day, const char *name) {
  char value[12], buf[48];
  const char *unit = count(day, value);
#if defined(PBL_PLATFORM_GABBRO)
  snprintf(buf, sizeof(buf), "%s%s%s%s%s", name, name[0] ? " " : "", value, unit[0] ? " " : "", unit);
#else
  snprintf(buf, sizeof(buf), "%s%s%s", value, unit[0] ? " " : "", unit);
  if (name[0]) {
    text_draw_along(ctx, name, slot_mid(s, COMP_THUMB + 2), s->center, COMP_TEXT + 2, GColorWhite);
  }
#endif
  text_draw_along(ctx, buf, slot_mid(s, 0), s->center, comp_fit(ctx, buf, COMP_TEXT + 2, slot_len(s)), GColorWhite);
}

// Like the Date's parts: the name small in orange, the count big, its words under it.
void center_countdown_draw(GContext *ctx, GPoint c, int day, const char *name) {
  char value[12];
  const char *unit = count(day, value);
  center_fit_text(ctx, name, GPoint(c.x, c.y - 9), SUB_SMALL + 1, 2 * SUB_R, GColorOrange);
  center_fit_text(ctx, value, GPoint(c.x, c.y + (name[0] ? 1 : -2)), SUB_TEXT + 3, 2 * SUB_R, GColorWhite);
  center_fit_text(ctx, unit, GPoint(c.x, c.y + SUB_LOW + 1), SUB_SMALL, 2 * SUB_R, GColorLightGray);
}
