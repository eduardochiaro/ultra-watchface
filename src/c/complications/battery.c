#include "complications.h"

void comp_battery_draw(GContext *ctx, const Slot *s) {
  BatteryChargeState b = battery_state_service_peek();
  char buf[8];
  snprintf(buf, sizeof(buf), "%d%%", b.charge_percent);
  GColor fill = b.charge_percent <= 20 ? GColorRed : GColorChromeYellow;
  comp_fill_gauge(ctx, s, b.charge_percent, fill, COMP_TRACK, buf);
}
