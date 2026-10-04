#include "complications.h"

// Minutes active today.
static int active_minutes(void) {
#if defined(PBL_HEALTH)
  return health_service_sum_today(HealthMetricActiveSeconds) / 60;
#else
  return 0;
#endif
}

// No bar: the runner and "48 MIN".
void comp_active_draw(GContext *ctx, const Slot *s) {
  char buf[12];
  snprintf(buf, sizeof(buf), "%d MIN", active_minutes());
  comp_icon_text(ctx, s, ICON_RUNNER, GColorYellow, buf);
}

// No ring: a caption, the minutes big under it, the unit below.
void center_active_draw(GContext *ctx, GPoint c) {
  char buf[8];
  snprintf(buf, sizeof(buf), "%d", active_minutes());
  text_draw(ctx, "ACTIVE", GPoint(c.x, c.y - SUB_R + 5), SUB_SMALL - 1, GColorYellow);
  center_fit_text(ctx, buf, GPoint(c.x, c.y + 1), SUB_TEXT + 3, 2 * SUB_R - 4, GColorWhite);
  text_draw(ctx, "MIN", GPoint(c.x, c.y + SUB_LOW + 2), SUB_SMALL + 1, GColorWhite);
}
