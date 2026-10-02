#include "complications.h"

// The user's text curved along the arc's middle.
void comp_custom_draw(GContext *ctx, const Slot *s, const char *txt) {
  text_draw_along(ctx, txt, slot_point(s, 50, 0), s->center, COMP_TEXT + 2, GColorWhite);
}

void center_fit_text(GContext *ctx, const char *txt, GPoint c, int size, int width, GColor color) {
  if (!txt[0]) return;
  int w = text_width(ctx, txt, size);  // grows with size
  text_draw(ctx, txt, c, w > width ? size * width / w : size, color);
}

// Monogram (4 characters at most, see Settings): as big as fits across the subdial.
void center_custom_draw(GContext *ctx, GPoint c, const char *txt) {
  center_fit_text(ctx, txt, c, (SUB_R + 2) * 4 / 5, 2 * SUB_R, GColorWhite);  // 14 on emery
}
