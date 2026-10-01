#include "complications.h"

// 0 = no reading.
static int heart_bpm(void) {
#if defined(PBL_HEALTH)
  return health_service_peek_current_value(HealthMetricHeartRateBPM);
#else
  return 0;
#endif
}

// No bar: "72 BPM" curved along the arc's middle, the heart toward the corner.
void comp_heart_draw(GContext *ctx, const Slot *s) {
  int bpm = heart_bpm();
  char buf[16] = "-- BPM";
  if (bpm > 0) snprintf(buf, sizeof(buf), "%d BPM", bpm);
  text_draw_along(ctx, buf, slot_point(s, 50, 0), s->center, COMP_TEXT + 2, GColorWhite);
  comp_icon(ctx, s, ICON_HEART, GColorRed);
}

// No ring: a heart, the caption above, the reading over its tip.
void center_heart_draw(GContext *ctx, GPoint c) {
  int bpm = heart_bpm();
  char buf[12] = "--";
  if (bpm > 0) snprintf(buf, sizeof(buf), "%d", bpm);
  text_draw(ctx, "BPM", GPoint(c.x, c.y - SUB_R + 5), SUB_SMALL - 1, GColorMelon);
  text_draw(ctx, ICON_HEART, c, SUB_R + 4, GColorRed);
  text_draw(ctx, buf, GPoint(c.x, c.y + SUB_R / 2 + 1), SUB_TEXT, GColorWhite);
}
