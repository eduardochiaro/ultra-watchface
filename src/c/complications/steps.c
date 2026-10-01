#include "complications.h"
#include "../settings.h"

static int32_t steps_today(void) {
#if defined(PBL_HEALTH)
  return health_service_sum_today(HealthMetricStepCount);
#else
  return 0;
#endif
}

int step_pct(void) {
  int goal = g_settings.step_goal > 0 ? g_settings.step_goal : 10000;
  return steps_today() * 100 / goal;
}

void comp_steps_draw(GContext *ctx, const Slot *s) {
  int32_t steps = steps_today();
  char buf[12];
  if (steps >= 1000) snprintf(buf, sizeof(buf), "%d,%03d", (int)(steps / 1000), (int)(steps % 1000));
  else snprintf(buf, sizeof(buf), "%d", (int)steps);
  comp_fill_gauge(ctx, s, step_pct(), GColorGreen, COMP_TRACK, buf, ICON_RUNNER);
}
