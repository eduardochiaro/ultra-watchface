#include "complications.h"
#include "../settings.h"
#include "../lang.h"

// Yesterday, today and tomorrow as boxes along the arc, left to right, with
// the weekday outside. Today is in light red, the others gray. Gabbro has no
// room beside the arc: the weekday leads the boxes, "WED 30 1 2".

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
  const char *wday = words()->days[localtime(&now)->tm_wday];
  Slot b = *slot;
  b.a0 = b.c0;  // the boxes stay in the corner's arc when it is joined
  b.a1 = b.c1;
  int left = slot_left_end(&b);
#if defined(PBL_PLATFORM_GABBRO)
  comp_end_label(ctx, &b, left, wday);
#else
  text_draw_along(ctx, wday, slot_mid(slot, COMP_THUMB + 2), slot->center, COMP_TEXT + 2, GColorWhite);
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
  text_draw(ctx, words()->days[t->tm_wday], GPoint(c.x, c.y + (cut - r) / 2), SUB_SMALL - 1, ink_on(GColorRed));
  char buf[3];
  snprintf(buf, sizeof(buf), "%d", t->tm_mday);
  text_draw(ctx, buf, GPoint(c.x, c.y + (cut + r) / 2), SUB_TEXT + 2, fixed(GColorBlack));
}

static int year_days(int year) {
  return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0) ? 366 : 365;
}

// Days since the week's first, g_settings.week_start after Monday.
static int week_day(const struct tm *t) {
  return (t->tm_wday + 6 - g_settings.week_start) % 7;
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

// ISO from Monday. From another day, as in the US: week 1 is the one 1 January is in.
static int week_number(const struct tm *t) {
  if (!g_settings.week_start) {
    return iso_week(t);
  }
  int jan1 = (week_day(t) - t->tm_yday % 7 + 7) % 7;  // its day of the week
  return (t->tm_yday + jan1) / 7 + 1;
}

#define DATE_PART 12  // room for any int: no truncation warnings

// A format as up to three parts, "" for none: a subdial stacks them, a corner
// joins them. Numeric is day/month, month/day with imperial units.
static void date_parts(DateFormat format, const struct tm *t, char *head, char *value, char *low) {
  const Words *w = words();
  head[0] = low[0] = 0;
  snprintf(value, DATE_PART, "%d", t->tm_mday);
  switch (format) {
    case DATE_MONTH: {
      strcpy(head, w->mons[t->tm_mon]);
      break;
    }
    case DATE_FULL: {
      strcpy(head, w->days[t->tm_wday]);
      strcpy(low, w->mons[t->tm_mon]);
      break;
    }
    case DATE_NUMERIC: {
      int mon = t->tm_mon + 1;
      snprintf(value, DATE_PART, "%d/%d", g_settings.imperial ? mon : t->tm_mday, g_settings.imperial ? t->tm_mday : mon);
      snprintf(low, DATE_PART, "%d", t->tm_year + 1900);
      break;
    }
    case DATE_WEEK: {
      strcpy(head, w->week);
      snprintf(value, DATE_PART, "%d", week_number(t));
      break;
    }
    case DATE_YEARDAY: {
      strcpy(head, w->day);
      snprintf(value, DATE_PART, "%d", t->tm_yday + 1);
      break;
    }
    case DATE_YEAR: {
      snprintf(value, DATE_PART, "%d", t->tm_year + 1900);
      break;
    }
    default: {  // DATE_WEEKDAY
      strcpy(head, w->days[t->tm_wday]);
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
    snprintf(buf, sizeof(buf), "%s %s %s", head, value, words()->months[today()->tm_mon]);
#else
    snprintf(buf, sizeof(buf), "%s %s", value, words()->months[today()->tm_mon]);
    text_draw_along(ctx, head, slot_mid(s, COMP_THUMB + 2), s->center, COMP_TEXT + 2, GColorWhite);
#endif
  }
  text_draw_along(ctx, buf, slot_mid(s, 0), s->center, comp_fit(ctx, buf, COMP_TEXT + 2, slot_len(s)), GColorWhite);
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
  center_fit_text(ctx, head, GPoint(c.x, c.y - 9), SUB_SMALL + 1, 2 * SUB_R, GColorOrange);
  center_fit_text(ctx, value, GPoint(c.x, c.y + (pair ? 4 : head[0] ? 1 : low[0] ? -2 : 0)), SUB_TEXT + (pair ? 5 : 3), 2 * SUB_R, GColorWhite);
  center_fit_text(ctx, low, GPoint(c.x, c.y + SUB_LOW + 1), SUB_SMALL, 2 * SUB_R, GColorLightGray);
}

// The week as 7 sections from its first day, lit up to today: the weekday for a
// caption, the day for a value. The bands are days of the month, today's in the lit one.
static void week_bands(const struct tm *t, Band bands[7]) {
  int first = t->tm_mday - week_day(t);
  for (int i = 0; i < 7; i++) {
    bands[i] = (Band){ i < 6 ? first + i : INT16_MAX, GColorChromeYellowARGB8 };
  }
}

void comp_week_draw(GContext *ctx, const Slot *s) {
  struct tm *t = today();
  Band bands[7];
  week_bands(t, bands);
  comp_band_draw(ctx, s, t->tm_mday, bands, words()->days[t->tm_wday]);
}

void center_week_draw(GContext *ctx, GPoint c) {
  struct tm *t = today();
  Band bands[7];
  week_bands(t, bands);
  center_band_draw(ctx, c, t->tm_mday, bands, words()->days[t->tm_wday]);
}

static int month_days(const struct tm *t) {
  static const uint8_t DAYS[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
  return DAYS[t->tm_mon] + (t->tm_mon == 1 && year_days(t->tm_year + 1900) == 366);
}

// How much of the year, month, week or day is gone, in %; its name into `name`:
// the year, the month, the Date's week head ("WEEK", 4 letters at most), the weekday.
// The week's from its first day.
static int progress(ComplicationId id, char name[DATE_PART]) {
  struct tm *t = today();
  if (id == COMP_DAY) {
    strcpy(name, words()->days[t->tm_wday]);
    return (t->tm_hour * 60 + t->tm_min) * 100 / (24 * 60);
  }
  if (id == COMP_WEEK_PROGRESS) {
    strcpy(name, words()->week);
    return (week_day(t) * 24 * 60 + t->tm_hour * 60 + t->tm_min) * 100 / (7 * 24 * 60);
  }
  if (id == COMP_MONTH) {
    strcpy(name, words()->mons[t->tm_mon]);
    return (t->tm_mday - 1) * 100 / month_days(t);
  }
  snprintf(name, DATE_PART, "%d", t->tm_year + 1900);
  return t->tm_yday * 100 / year_days(t->tm_year + 1900);
}

// A bar like humidity's, its name curved by its middle, where a band's value
// is. Gabbro has no room beside the arc: the name leads, like an icon.
static void progress_draw(GContext *ctx, const Slot *s, ComplicationId id) {
  char name[DATE_PART], buf[16];
  int pct = progress(id, name);
  snprintf(buf, sizeof(buf), "%d" CORNER_PCT, pct);
#if defined(PBL_PLATFORM_GABBRO)
  comp_fill_gauge(ctx, s, pct, GColorChromeYellow, COMP_TRACK, buf, name);
#else
  comp_fill_gauge(ctx, s, pct, GColorChromeYellow, COMP_TRACK, buf, NULL);
  text_draw_along(ctx, name, slot_mid(s, COMP_THUMB), s->center, COMP_TEXT + 2, GColorWhite);
#endif
}

static void center_progress_draw(GContext *ctx, GPoint c, ComplicationId id) {
  char name[DATE_PART], buf[16];
  int pct = progress(id, name);
  snprintf(buf, sizeof(buf), "%d" SMALL_PCT, pct);
  center_gauge(ctx, c, pct, GColorChromeYellow);
  center_fit_text(ctx, buf, GPoint(c.x, c.y - 1), SUB_TEXT + 2, 2 * SUB_R - SUB_T - 4, GColorWhite);
  text_draw(ctx, name, GPoint(c.x, c.y + SUB_LOW), SUB_SMALL, GColorWhite);
}

void comp_year_draw(GContext *ctx, const Slot *s) {
  progress_draw(ctx, s, COMP_YEAR);
}

void comp_month_draw(GContext *ctx, const Slot *s) {
  progress_draw(ctx, s, COMP_MONTH);
}

void comp_day_draw(GContext *ctx, const Slot *s) {
  progress_draw(ctx, s, COMP_DAY);
}

void comp_week_progress_draw(GContext *ctx, const Slot *s) {
  progress_draw(ctx, s, COMP_WEEK_PROGRESS);
}

void center_year_draw(GContext *ctx, GPoint c) {
  center_progress_draw(ctx, c, COMP_YEAR);
}

void center_month_draw(GContext *ctx, GPoint c) {
  center_progress_draw(ctx, c, COMP_MONTH);
}

void center_day_draw(GContext *ctx, GPoint c) {
  center_progress_draw(ctx, c, COMP_DAY);
}

void center_week_progress_draw(GContext *ctx, GPoint c) {
  center_progress_draw(ctx, c, COMP_WEEK_PROGRESS);
}
