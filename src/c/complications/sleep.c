#include "complications.h"

// Slept last night, "7h32"; "--" with none recorded.
static void sleep_text(char *buf, size_t n) {
  int min = 0;
#if defined(PBL_HEALTH)
  min = health_service_sum_today(HealthMetricSleepSeconds) / 60;
#endif
  if (min > 0) {
    snprintf(buf, n, "%dh%02d", min / 60, min % 60);
  } else {
    snprintf(buf, n, "--");
  }
}

// No bar: "ZZ" for an icon and the time slept.
void comp_sleep_draw(GContext *ctx, const Slot *s) {
  char buf[12];
  sleep_text(buf, sizeof(buf));
  comp_icon_text(ctx, s, "ZZ", GColorLavenderIndigo, buf);
}

// No ring: a caption, the time slept big under it.
void center_sleep_draw(GContext *ctx, GPoint c) {
  char buf[12];
  sleep_text(buf, sizeof(buf));
  text_draw(ctx, "SLEEP", GPoint(c.x, c.y - SUB_R + 5), SUB_SMALL - 1, GColorLavenderIndigo);
  center_fit_text(ctx, buf, GPoint(c.x, c.y + 2), SUB_TEXT + 2, 2 * SUB_R - 4, GColorWhite);
}
