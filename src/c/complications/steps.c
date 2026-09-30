#include "complications.h"
#include "../settings.h"

void comp_steps_draw(GContext *ctx, const Slot *s) {
  int32_t steps = 0;
#if defined(PBL_HEALTH)
  steps = health_service_sum_today(HealthMetricStepCount);
#endif
  char buf[12];
  if (steps >= 1000) snprintf(buf, sizeof(buf), "%d,%03d", (int)(steps / 1000), (int)(steps % 1000));
  else snprintf(buf, sizeof(buf), "%d", (int)steps);
  int goal = g_settings.step_goal > 0 ? g_settings.step_goal : 10000;
  comp_fill_gauge(ctx, s, steps * 100 / goal, GColorGreen, COMP_TRACK, buf);
}
