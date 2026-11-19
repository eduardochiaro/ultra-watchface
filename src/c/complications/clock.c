#include "complications.h"
#include "../lang.h"

// "10:09" in the watch's 12 or 24h style. Returns "AM" or "PM", "" in 24h style.
// No time (t NULL): "--".
static const char *clock_text(char *buf, size_t n, const struct tm *t) {
  if (!t) {
    snprintf(buf, n, "--");
    return "";
  }
  if (clock_is_24h_style()) {
    snprintf(buf, n, "%02d:%02d", t->tm_hour, t->tm_min);
    return "";
  }
  snprintf(buf, n, "%d:%02d", (t->tm_hour + 11) % 12 + 1, t->tm_min);
  return t->tm_hour < 12 ? "AM" : "PM";
}

static struct tm *local_now(void) {
  time_t now = time(NULL);
  return localtime(&now);
}

// When the watch's next enabled alarm fires, NULL for none. Always NULL where
// the SDK has no alarm service (basalt, chalk, diorite): the page hides it there.
// ponytail: the time only, no day; an alarm is rarely more than a day off.
static struct tm *alarm_next(void) {
  time_t at = 0;
  if (!alarm_service_peek_next(&at)) {
    return NULL;
  }
  return localtime(&at);
}

// A time zone: a fixed offset from UTC, in minutes.
// ponytail: no daylight saving, PST and PDT are picked apart; send the offset
// from the phone by IANA zone if it should switch by itself.
static struct tm *zone_now(int offset) {
  time_t now = time(NULL) + offset * 60;
  return gmtime(&now);
}

// `name` (or "") then the time and am/pm, curved along the arc's middle, shrunk to fit it.
static void corner(GContext *ctx, const Slot *s, const struct tm *t, const char *name) {
  char time[8], buf[24];
  const char *half = clock_text(time, sizeof(time), t);
  snprintf(buf, sizeof(buf), "%s%s%s%s%s", name, name[0] ? " " : "", time, half[0] ? " " : "", half);
  text_draw_along(ctx, buf, slot_mid(s, 0), s->center, comp_fit(ctx, buf, COMP_TEXT + 2, slot_len(s)), GColorWhite);
}

// No ring: the time across the subdial, am/pm under it, `name` (or "") above.
static void subdial(GContext *ctx, GPoint c, const struct tm *t, const char *name) {
  char time[8];
  const char *half = clock_text(time, sizeof(time), t);
  if (name[0]) {
    text_draw(ctx, name, GPoint(c.x, c.y - SUB_R + 5), SUB_SMALL, GColorChromeYellow);
  }
  center_fit_text(ctx, time, GPoint(c.x, c.y + (name[0] ? 2 : half[0] ? -1 : 0)), SUB_TEXT + 3, 2 * SUB_R, GColorWhite);
  text_draw(ctx, half, GPoint(c.x, c.y + SUB_LOW + 2), SUB_SMALL - 1, GColorLightGray);
}

void comp_time_draw(GContext *ctx, const Slot *s) {
  corner(ctx, s, local_now(), "");
}

void center_time_draw(GContext *ctx, GPoint c) {
  subdial(ctx, c, local_now(), "");
}

// The alarm clock for a name, like the status corners (status.c): lit when
// one is set, gray and "--" when not.
void comp_alarm_draw(GContext *ctx, const Slot *s) {
  struct tm *t = alarm_next();
  char time[8], buf[12];
  const char *half = clock_text(time, sizeof(time), t);
  snprintf(buf, sizeof(buf), "%s%s%s", time, half[0] ? " " : "", half);
  comp_icon_fill_text(ctx, s, ICON_ALARM, t ? GColorChromeYellow : GColorDarkGray,
                      ICON_ALARM_FILL, t ? GColorPastelYellow : GColorLightGray, buf);
}

void center_alarm_draw(GContext *ctx, GPoint c) {
  subdial(ctx, c, alarm_next(), words()->alarm);
}

void comp_zone_draw(GContext *ctx, const Slot *s, int offset, const char *name) {
  corner(ctx, s, zone_now(offset), name);
}

void center_zone_draw(GContext *ctx, GPoint c, int offset, const char *name) {
  subdial(ctx, c, zone_now(offset), name);
}

// One part of the time across the subdial, as big as fits.
static void part(GContext *ctx, GPoint c, int v, bool zero) {
  char buf[4];
  snprintf(buf, sizeof(buf), zero ? "%02d" : "%d", v);
  center_fit_text(ctx, buf, c, (SUB_R + 2) * 4 / 5, 2 * SUB_R, GColorWhite);
}

static int hour_now(void) {
  int h = local_now()->tm_hour;
  return clock_is_24h_style() ? h : (h + 11) % 12 + 1;
}

void center_hours_draw(GContext *ctx, GPoint c) {
  part(ctx, c, hour_now(), true);
}

void center_hours_plain_draw(GContext *ctx, GPoint c) {
  part(ctx, c, hour_now(), false);
}

void center_minutes_draw(GContext *ctx, GPoint c) {
  part(ctx, c, local_now()->tm_min, true);
}

void center_minutes_plain_draw(GContext *ctx, GPoint c) {
  part(ctx, c, local_now()->tm_min, false);
}

void center_ampm_draw(GContext *ctx, GPoint c) {
  center_fit_text(ctx, local_now()->tm_hour < 12 ? "AM" : "PM", c, (SUB_R + 2) * 4 / 5, 2 * SUB_R, GColorWhite);
}
