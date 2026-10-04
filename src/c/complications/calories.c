#include "complications.h"

// Burned today in kcal: active, plus resting for the total.
static void calories_text(char *buf, size_t n, bool resting) {
  int kcal = 0;
#if defined(PBL_HEALTH)
  kcal = health_service_sum_today(HealthMetricActiveKCalories);
  if (resting) {
    kcal += health_service_sum_today(HealthMetricRestingKCalories);
  }
#endif
  snprintf(buf, n, "%d", kcal);
}

// Like the distance: the bar is the step goal's progress, there is no calorie goal.
static void corner(GContext *ctx, const Slot *s, bool resting) {
  char buf[12];
  calories_text(buf, sizeof(buf), resting);
  comp_fill_gauge(ctx, s, step_pct(), GColorOrange, COMP_TRACK, buf, ICON_FLAME);
}

// Step goal around the ring, the unit over the value, a flame in the gap.
static void subdial(GContext *ctx, GPoint c, bool resting) {
  char buf[12];
  calories_text(buf, sizeof(buf), resting);
  center_gauge(ctx, c, step_pct(), GColorOrange);
  text_draw(ctx, "KCAL", GPoint(c.x, c.y - 7), SUB_SMALL - 1, GColorWhite);
  center_fit_text(ctx, buf, GPoint(c.x, c.y + 2), SUB_TEXT + 1, 2 * SUB_R - SUB_T - 4, GColorWhite);
  text_draw(ctx, ICON_FLAME, GPoint(c.x, c.y + SUB_LOW + 2), 10, GColorOrange);
}

void comp_calories_draw(GContext *ctx, const Slot *s) { corner(ctx, s, true); }
void comp_calories_active_draw(GContext *ctx, const Slot *s) { corner(ctx, s, false); }
void center_calories_draw(GContext *ctx, GPoint c) { subdial(ctx, c, true); }
void center_calories_active_draw(GContext *ctx, GPoint c) { subdial(ctx, c, false); }
