#include "complications.h"
#include "../settings.h"

// Walked today, "3.9" in km or mi.
static void distance_text(char *buf, size_t n) {
  int32_t m = 0;
#if defined(PBL_HEALTH)
  m = health_service_sum_today(HealthMetricWalkedDistanceMeters);
#endif
  int tenths = g_settings.imperial ? m * 10 / 1609 : m / 100;
  snprintf(buf, n, "%d.%d", tenths / 10, tenths % 10);
}

// ponytail: the bar is the step goal's progress; add a distance goal if wanted.
void comp_distance_draw(GContext *ctx, const Slot *s) {
  char buf[12];
  distance_text(buf, sizeof(buf));
  strcat(buf, g_settings.imperial ? "mi" : "km");
  comp_fill_gauge(ctx, s, step_pct(), GColorChromeYellow, COMP_TRACK, buf, ICON_RUNNER);
}

// Step goal around the ring (see comp_distance_draw), runner in the gap.
void center_distance_draw(GContext *ctx, GPoint c) {
  char buf[12];
  distance_text(buf, sizeof(buf));
  center_gauge(ctx, c, step_pct(), GColorChromeYellow);
  text_draw(ctx, g_settings.imperial ? "MI" : "KM", GPoint(c.x, c.y - 7), SUB_SMALL - 1, GColorWhite);
  text_draw(ctx, buf, GPoint(c.x, c.y + 2), SUB_TEXT + 1, GColorWhite);
  text_draw(ctx, ICON_RUNNER, GPoint(c.x, c.y + SUB_LOW + 2), 10, GColorChromeYellow);
}
