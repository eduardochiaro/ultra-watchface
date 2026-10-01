#include "complications.h"

// Full-length bar filling from the left, value centered outside the arc.
// No icon.
void comp_battery_draw(GContext *ctx, const Slot *s) {
  BatteryChargeState b = battery_state_service_peek();
  int pct = b.charge_percent, left = slot_left_end(s);
  char buf[8];
  snprintf(buf, sizeof(buf), "%d%%", pct);
  slot_arc(ctx, s, 0, 100, COMP_TRACK);
  if (pct > 0) slot_arc(ctx, s, left, left ? 100 - pct : pct, pct <= 20 ? GColorRed : GColorChromeYellow);
  text_draw_along(ctx, buf, slot_point(s, 50, COMP_THUMB), s->center, COMP_TEXT + 2, GColorWhite);
}
