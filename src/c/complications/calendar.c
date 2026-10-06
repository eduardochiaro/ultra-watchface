#include "complications.h"
#include "../settings.h"

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

static void corner_calendar(GContext *ctx, const Slot *slot) {
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
static void center_calendar(GContext *ctx, GPoint c) {
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

// In full; short is the first 3 letters.
static const char *const MONTHS[] = { "JANUARY", "FEBRUARY", "MARCH", "APRIL", "MAY", "JUNE", "JULY", "AUGUST",
                                      "SEPTEMBER", "OCTOBER", "NOVEMBER", "DECEMBER" };

static int year_days(int year) {
  return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0) ? 366 : 365;
}

// ISO 8601: weeks start on Monday, and a week is of the year its Thursday is in.
static int iso_week(const struct tm *t) {
  int year = t->tm_year + 1900;
  int thu = t->tm_yday - (t->tm_wday + 6) % 7 + 3;
  if (thu < 0) {
    thu += year_days(year - 1);
  } else if (thu >= year_days(year)) {
    thu -= year_days(year);
  }
  return thu / 7 + 1;
}

#define DATE_PART 12  // room for any int: no truncation warnings

// A format as up to three parts, "" for none: a subdial stacks them, a corner
// joins them. Numeric is day/month, month/day with imperial units.
static void date_parts(DateFormat format, const struct tm *t, char *head, char *value, char *low) {
  head[0] = low[0] = 0;
  snprintf(value, DATE_PART, "%d", t->tm_mday);
  switch (format) {
    case DATE_MONTH: {
      snprintf(head, DATE_PART, "%.3s", MONTHS[t->tm_mon]);
      break;
    }
    case DATE_FULL: {
      strcpy(head, DAYS[t->tm_wday]);
      snprintf(low, DATE_PART, "%.3s", MONTHS[t->tm_mon]);
      break;
    }
    case DATE_NUMERIC: {
      int mon = t->tm_mon + 1;
      snprintf(value, DATE_PART, "%d/%d", g_settings.imperial ? mon : t->tm_mday, g_settings.imperial ? t->tm_mday : mon);
      snprintf(low, DATE_PART, "%d", t->tm_year + 1900);
      break;
    }
    case DATE_WEEK: {
      strcpy(head, "WEEK");
      snprintf(value, DATE_PART, "%d", iso_week(t));
      break;
    }
    case DATE_YEARDAY: {
      strcpy(head, "DAY");
      snprintf(value, DATE_PART, "%d", t->tm_yday + 1);
      break;
    }
    case DATE_YEAR: {
      snprintf(value, DATE_PART, "%d", t->tm_year + 1900);
      break;
    }
    default: {  // DATE_WEEKDAY
      strcpy(head, DAYS[t->tm_wday]);
      break;
    }
  }
}

static struct tm *today(void) {
  time_t now = time(NULL);
  return localtime(&now);
}

// DATE_CALENDAR: the boxes. The rest: text along the arc, like the Text
// complication, "OCT 6", "6/10/2026". The full date has its month in full, and
// the weekday beside the arc, where the calendar's is; gabbro has no room
// there: one line, "TUE 6 OCTOBER".
void comp_date_draw(GContext *ctx, const Slot *s, int format) {
  if (format <= DATE_CALENDAR || format >= DATE_COUNT) {  // or a time zone's offset, left in the place
    corner_calendar(ctx, s);
    return;
  }
  char head[DATE_PART], value[DATE_PART], low[DATE_PART], buf[3 * DATE_PART];
  date_parts(format, today(), head, value, low);
  const char *sep = format == DATE_NUMERIC ? "/" : " ";
  snprintf(buf, sizeof(buf), "%s%s%s%s%s", head, head[0] ? sep : "", value, low[0] ? sep : "", low);
  if (format == DATE_FULL) {
#if defined(PBL_PLATFORM_GABBRO)
    snprintf(buf, sizeof(buf), "%s %s %s", head, value, MONTHS[today()->tm_mon]);
#else
    snprintf(buf, sizeof(buf), "%s %s", value, MONTHS[today()->tm_mon]);
    text_draw_along(ctx, head, slot_point(s, 50, COMP_THUMB + 2), s->center, COMP_TEXT + 2, GColorWhite);
#endif
  }
  text_draw_along(ctx, buf, slot_point(s, 50, 0), s->center, comp_fit(ctx, buf, COMP_TEXT + 2, slot_len(s)), GColorWhite);
}

// DATE_CALENDAR: the page. The rest, no ring: the first part small in red (the
// accent in that scheme), the value big, the last part small under it.
void center_date_draw(GContext *ctx, GPoint c, int format) {
  if (format <= DATE_CALENDAR || format >= DATE_COUNT) {
    center_calendar(ctx, c);
    return;
  }
  char head[DATE_PART], value[DATE_PART], low[DATE_PART];
  date_parts(format, today(), head, value, low);
  bool pair = head[0] && !low[0];  // two lines: room for a bigger value
  text_draw(ctx, head, GPoint(c.x, c.y - 9), SUB_SMALL + 1, GColorOrange);
  center_fit_text(ctx, value, GPoint(c.x, c.y + (pair ? 4 : head[0] ? 1 : low[0] ? -2 : 0)), SUB_TEXT + (pair ? 5 : 3), 2 * SUB_R, GColorWhite);
  text_draw(ctx, low, GPoint(c.x, c.y + SUB_LOW + 1), SUB_SMALL, GColorLightGray);
}

// The week as 7 sections, Monday first, lit up to today: the weekday for a
// caption, the day for a value. The bands are days of the month, today's in the lit one.
static void week_bands(const struct tm *t, Band bands[7]) {
  int monday = t->tm_mday - (t->tm_wday + 6) % 7;
  for (int i = 0; i < 7; i++) {
    bands[i] = (Band){ i < 6 ? monday + i : INT16_MAX, GColorChromeYellowARGB8 };
  }
}

void comp_week_draw(GContext *ctx, const Slot *s) {
  struct tm *t = today();
  Band bands[7];
  week_bands(t, bands);
  comp_band_draw(ctx, s, t->tm_mday, bands, DAYS[t->tm_wday]);
}

void center_week_draw(GContext *ctx, GPoint c) {
  struct tm *t = today();
  Band bands[7];
  week_bands(t, bands);
  center_band_draw(ctx, c, t->tm_mday, bands, DAYS[t->tm_wday]);
}

// How much of the year is gone, in %; the year into `year`.
static int year_pct(char year[DATE_PART]) {
  struct tm *t = today();
  snprintf(year, DATE_PART, "%d", t->tm_year + 1900);
  return t->tm_yday * 100 / year_days(t->tm_year + 1900);
}

// A bar like humidity's, the year curved by its middle, where a band's value
// is. Gabbro has no room beside the arc: the year leads, like an icon.
void comp_year_draw(GContext *ctx, const Slot *s) {
  char year[DATE_PART], buf[16];
  int pct = year_pct(year);
  snprintf(buf, sizeof(buf), "%d" CORNER_PCT, pct);
#if defined(PBL_PLATFORM_GABBRO)
  comp_fill_gauge(ctx, s, pct, GColorChromeYellow, COMP_TRACK, buf, year);
#else
  comp_fill_gauge(ctx, s, pct, GColorChromeYellow, COMP_TRACK, buf, NULL);
  text_draw_along(ctx, year, slot_point(s, 50, COMP_THUMB), s->center, COMP_TEXT + 2, GColorWhite);
#endif
}

void center_year_draw(GContext *ctx, GPoint c) {
  char year[DATE_PART], buf[16];
  int pct = year_pct(year);
  snprintf(buf, sizeof(buf), "%d" SMALL_PCT, pct);
  center_gauge(ctx, c, pct, GColorChromeYellow);
  center_fit_text(ctx, buf, GPoint(c.x, c.y - 1), SUB_TEXT + 2, 2 * SUB_R - SUB_T - 4, GColorWhite);
  text_draw(ctx, year, GPoint(c.x, c.y + SUB_LOW), SUB_SMALL, GColorWhite);
}
