#include "complications.h"

int heart_bpm(void) {
#if defined(PBL_HEALTH)
  return health_service_peek_current_value(HealthMetricHeartRateBPM);
#else
  return 0;
#endif
}

// 40..180 bpm along the bar.
void comp_heart_draw(GContext *ctx, const Slot *s) {
  int bpm = heart_bpm();
  char buf[12] = "--";
  if (bpm > 0) snprintf(buf, sizeof(buf), "%d", bpm);
  comp_fill_gauge(ctx, s, bpm > 0 ? (bpm - 40) * 100 / 140 : 0, GColorRed, COMP_TRACK, buf, ICON_HEART);
}
