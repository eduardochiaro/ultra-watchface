#include "complications.h"
#include "../lang.h"

// Watch status: lit when on, a gray icon when off. The phone link redraws the
// face as it changes (ultra-watchface.c); quiet time has no event, so the
// minute's stamp picks it up. `fill`, if any, is the icon's insides, drawn
// under it in a light color: `light` when on, light gray when off.

// Corner: the icon toward the corner, ON or OFF along the arc.
static void status_corner(GContext *ctx, const Slot *s, const char *icon, const char *fill, bool on, GColor color, GColor light) {
  comp_icon_fill_text(ctx, s, icon, on ? color : GColorDarkGray, fill, on ? light : GColorLightGray, on ? words()->on : words()->off);
}

// Subdial: the icon big, ON or OFF under it.
static void status_subdial(GContext *ctx, GPoint c, const char *icon, const char *fill, bool on, GColor color, GColor light) {
  GPoint at = GPoint(c.x, c.y - 2);
  if (fill) {
    text_draw(ctx, fill, at, SUB_R + 2, on ? light : GColorLightGray);
  }
  text_draw(ctx, icon, at, SUB_R + 2, on ? color : GColorDarkGray);
  text_draw(ctx, on ? words()->on : words()->off, GPoint(c.x, c.y + SUB_LOW + 2), SUB_SMALL, GColorWhite);
}

void comp_bluetooth_draw(GContext *ctx, const Slot *s) {
  status_corner(ctx, s, ICON_BLUETOOTH, NULL, connection_service_peek_pebble_app_connection(), GColorPictonBlue, GColorPictonBlue);
}

void center_bluetooth_draw(GContext *ctx, GPoint c) {
  status_subdial(ctx, c, ICON_BLUETOOTH, NULL, connection_service_peek_pebble_app_connection(), GColorPictonBlue, GColorPictonBlue);
}

void comp_quiet_draw(GContext *ctx, const Slot *s) {
  status_corner(ctx, s, ICON_QUIET, ICON_QUIET_FILL, quiet_time_is_active(), GColorLavenderIndigo, GColorRichBrilliantLavender);
}

void center_quiet_draw(GContext *ctx, GPoint c) {
  status_subdial(ctx, c, ICON_QUIET, ICON_QUIET_FILL, quiet_time_is_active(), GColorLavenderIndigo, GColorRichBrilliantLavender);
}
