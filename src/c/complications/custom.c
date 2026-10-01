#include "complications.h"

// The user's text curved along the arc's middle.
void comp_custom_draw(GContext *ctx, const Slot *s, const char *txt) {
  text_draw_along(ctx, txt, slot_point(s, 50, 0), s->center, COMP_TEXT + 2, GColorWhite);
}

// Monogram (4 characters at most, see Settings): as big as fits across the subdial.
void center_custom_draw(GContext *ctx, GPoint c, const char *txt) {
  if (!txt[0]) return;
  int max = (SUB_R + 2) * 4 / 5, w = text_width(ctx, txt, max);  // 14 on emery; width grows with size
  int size = w > 2 * SUB_R ? max * 2 * SUB_R / w : max;
  text_draw(ctx, txt, c, size, GColorWhite);
}
