#include "complications.h"

// Green over 70%, red under 15%, yellow between.
static GColor level_color(int pct) {
  return pct > 70 ? GColorGreen : pct < 15 ? GColorRed : GColorChromeYellow;
}

// Yellow while charging.
static GColor bolt_color(const BatteryChargeState *b) {
  return b->is_charging ? GColorChromeYellow : GColorWhite;
}

// Like comp_fill_gauge, "BOLT 82% BAR", but the bolt has its own color.
void comp_battery_draw(GContext *ctx, const Slot *slot) {
  BatteryChargeState b = battery_state_service_peek();
  char buf[8];
  snprintf(buf, sizeof(buf), "%d" CORNER_PCT, b.charge_percent);
  Slot s = *slot;
  int left = slot_left_end(&s);
  comp_icon(ctx, &s, ICON_BOLT, bolt_color(&b));
  comp_end_label(ctx, &s, left, buf);
  slot_arc(ctx, &s, 0, 100, COMP_TRACK);
  if (b.charge_percent > 0)
    slot_arc(ctx, &s, left, left ? 100 - b.charge_percent : b.charge_percent, level_color(b.charge_percent));
}

// Charge around the ring, a bolt below.
void center_battery_draw(GContext *ctx, GPoint c) {
  BatteryChargeState b = battery_state_service_peek();
  char buf[8];
  snprintf(buf, sizeof(buf), "%d", b.charge_percent);
  center_gauge(ctx, c, b.charge_percent, level_color(b.charge_percent));
  text_draw(ctx, buf, GPoint(c.x, c.y - 1), SUB_TEXT + 1, GColorWhite);
  text_draw(ctx, ICON_BOLT, GPoint(c.x, c.y + SUB_LOW + 1), 12, bolt_color(&b));
}
