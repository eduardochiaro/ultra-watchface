#include "complications.h"
#include "../settings.h"

// "10:09" in the watch's 12 or 24h style. Returns "AM" or "PM", "" in 24h style.
static const char *clock_text(char *buf, size_t n, const struct tm *t) {
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

// The second time zone: a fixed offset from UTC, in minutes.
// ponytail: no daylight saving; send the offset from the phone by zone name if wanted.
static struct tm *zone_now(void) {
  time_t now = time(NULL) + g_settings.zone_offset * 60;
  return gmtime(&now);
}

// `name` (or "") then the time and am/pm, curved along the arc's middle, shrunk to fit it.
static void corner(GContext *ctx, const Slot *s, const struct tm *t, const char *name) {
  char time[8], buf[24];
  const char *half = clock_text(time, sizeof(time), t);
  snprintf(buf, sizeof(buf), "%s%s%s%s%s", name, name[0] ? " " : "", time, half[0] ? " " : "", half);
  text_draw_along(ctx, buf, slot_point(s, 50, 0), s->center, comp_fit(ctx, buf, COMP_TEXT + 2, slot_len(s)), GColorWhite);
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

void comp_zone_draw(GContext *ctx, const Slot *s) {
  corner(ctx, s, zone_now(), g_settings.zone_name);
}

void center_zone_draw(GContext *ctx, GPoint c) {
  subdial(ctx, c, zone_now(), g_settings.zone_name);
}
