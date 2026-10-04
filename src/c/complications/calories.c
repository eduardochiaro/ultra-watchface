#include "complications.h"

// Burned today in kcal: active, plus resting for the total.
static void calories_text(char *buf, size_t n, const char *unit, bool resting) {
  int kcal = 0;
#if defined(PBL_HEALTH)
  kcal = health_service_sum_today(HealthMetricActiveKCalories);
  if (resting) kcal += health_service_sum_today(HealthMetricRestingKCalories);
#endif
  snprintf(buf, n, "%d%s", kcal, unit);
}

// ponytail: no bar, there is no calorie goal; add one with a setting if wanted.
static void corner(GContext *ctx, const Slot *s, bool resting) {
  char buf[16];
  calories_text(buf, sizeof(buf), " KCAL", resting);
  comp_icon_text(ctx, s, ICON_FLAME, GColorOrange, buf);
}

// No ring: a flame, the value big under it, the unit below.
static void subdial(GContext *ctx, GPoint c, bool resting) {
  char buf[12];
  calories_text(buf, sizeof(buf), "", resting);
  text_draw(ctx, ICON_FLAME, GPoint(c.x, c.y - SUB_R + 5), 10, GColorOrange);
  center_fit_text(ctx, buf, GPoint(c.x, c.y + 1), SUB_TEXT + 2, 2 * SUB_R - 4, GColorWhite);
  text_draw(ctx, "KCAL", GPoint(c.x, c.y + SUB_LOW + 2), SUB_SMALL - 1, GColorWhite);
}

void comp_calories_draw(GContext *ctx, const Slot *s) { corner(ctx, s, true); }
void comp_calories_active_draw(GContext *ctx, const Slot *s) { corner(ctx, s, false); }
void center_calories_draw(GContext *ctx, GPoint c) { subdial(ctx, c, true); }
void center_calories_active_draw(GContext *ctx, GPoint c) { subdial(ctx, c, false); }
