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

// Charge around the ring, a bolt below: yellow while charging.
void center_battery_draw(GContext *ctx, GPoint c) {
  BatteryChargeState b = battery_state_service_peek();
  char buf[8];
  snprintf(buf, sizeof(buf), "%d", b.charge_percent);
  center_gauge(ctx, c, b.charge_percent, b.charge_percent <= 20 ? GColorRed : GColorGreen);
  text_draw(ctx, buf, GPoint(c.x, c.y - 1), SUB_TEXT + 1, GColorWhite);
  text_draw(ctx, ICON_BOLT, GPoint(c.x, c.y + SUB_LOW + 1), 12, b.is_charging ? GColorChromeYellow : GColorWhite);
}
