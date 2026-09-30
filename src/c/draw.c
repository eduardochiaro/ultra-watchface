#include "draw.h"
#include "settings.h"
#include <pebble-fctx/fctx.h>
#include <pebble-fctx/ffont.h>

static FFont *s_font;
static GSize s_screen;

void draw_init(GSize screen) {
  s_screen = screen;
  s_font = ffont_create_from_resource(RESOURCE_ID_FONT);
  fctx_enable_aa(true);
}

void draw_deinit(void) {
  ffont_destroy(s_font);
}

GColor theme(GColor c) {
  bool gray = c.r == c.g && c.g == c.b;
  if ((g_settings.scheme & SCHEME_MONO) && !gray) {
    c = (c.r >= 2 || c.g >= 2 || c.b >= 2) ? GColorWhite : GColorDarkGray;  // fills vs tracks
    gray = true;
  }
  if ((g_settings.scheme & SCHEME_LIGHT) && gray) c.r = c.g = c.b = 3 - c.r;
  return c;
}

int32_t clamp_i32(int32_t v, int32_t lo, int32_t hi) {
  return v < lo ? lo : v > hi ? hi : v;
}

static GPoint polar(GPoint c, int32_t angle, int r) {
  return GPoint(c.x + sin_lookup(angle) * r / TRIG_MAX_RATIO,
                c.y - cos_lookup(angle) * r / TRIG_MAX_RATIO);
}

static int32_t slot_angle(const Slot *s, int pct) {
  return s->a0 + (s->a1 - s->a0) * pct / 100;
}

GPoint slot_point(const Slot *s, int pct, int dr) {
  return polar(s->center, slot_angle(s, pct), s->radius + dr);
}

// px -> angle on this slot's circle.
static int32_t px_angle(const Slot *s, int px) {
  return px * (TRIG_MAX_ANGLE * 10 / 63) / s->radius;
}

GPoint slot_past(const Slot *s, int end, int px) {
  int32_t from = end ? s->a0 : s->a1, at = end ? s->a1 : s->a0;
  int32_t step = px_angle(s, px);
  return polar(s->center, at + (at > from ? step : -step), s->radius);
}

void slot_trim(Slot *s, int end, int px) {
  int32_t *at = end ? &s->a1 : &s->a0, other = end ? s->a0 : s->a1;
  *at += *at < other ? px_angle(s, px) : -px_angle(s, px);
}

int slot_left_end(const Slot *s) {
  return slot_point(s, 100, 0).x < slot_point(s, 0, 0).x ? 100 : 0;
}

// Arc with round caps. from/to may come in either order.
void slot_arc(GContext *ctx, const Slot *s, int from_pct, int to_pct, GColor color) {
  int32_t a = slot_angle(s, from_pct), b = slot_angle(s, to_pct);
  if (a > b) { int32_t t = a; a = b; b = t; }
  int outer = s->radius + s->thickness / 2;
  GRect box = GRect(s->center.x - outer, s->center.y - outer, outer * 2, outer * 2);
  graphics_context_set_fill_color(ctx, theme(color));
  if (b > a) graphics_fill_radial(ctx, box, GOvalScaleModeFitCircle, s->thickness, a, b);
  // fill_circle(r) is 2r+1 wide; keep caps no wider than the arc.
  int cap = (s->thickness - 1) / 2;
  graphics_fill_circle(ctx, polar(s->center, a, s->radius), cap);
  graphics_fill_circle(ctx, polar(s->center, b, s->radius), cap);
}

void slot_dot(GContext *ctx, const Slot *s, int pct, int r, GColor fill, GColor ring) {
  GPoint p = slot_point(s, pct, 0);
  graphics_context_set_fill_color(ctx, theme(ring));
  graphics_fill_circle(ctx, p, r + 1);
  graphics_context_set_fill_color(ctx, theme(fill));
  graphics_fill_circle(ctx, p, r);
}

// Pull a box back inside the screen: the bounds on rect, the circle on round.
static GPoint clamp_to_screen(GPoint c, int hw, int hh) {
  const int m = 2;
#if defined(PBL_ROUND)
  int cx = s_screen.w / 2, cy = s_screen.h / 2, r = cx - m;
  int dx = c.x - cx, dy = c.y - cy;
  for (int i = 0; i < 32; i++) {
    int ex = abs(dx) + hw, ey = abs(dy) + hh;
    if (ex * ex + ey * ey <= r * r) break;
    dx = dx * 15 / 16;
    dy = dy * 15 / 16;
  }
  return GPoint(cx + dx, cy + dy);
#else
  return GPoint(clamp_i32(c.x, hw + m, s_screen.w - hw - m),
                clamp_i32(c.y, hh + m, s_screen.h - hh - m));
#endif
}

void icon_block(GContext *ctx, GPoint c, int size, GColor color) {
  c = clamp_to_screen(c, size / 2, size / 2);
  graphics_context_set_fill_color(ctx, theme(color));
  graphics_fill_rect(ctx, GRect(c.x - size / 2, c.y - size / 2, size, size), 2, GCornersAll);
}

static void draw_text(GContext *ctx, const char *txt, GPoint c, int size, GColor color,
                      int32_t rot) {
  if (!s_font) return;
  FContext f;
  fctx_init_context(&f, ctx);
  fctx_set_text_cap_height(&f, s_font, size);
  int hw = FIXED_TO_INT(fctx_string_width(&f, txt, s_font)) / 2, hh = size / 2 + 1;
  int sn = abs(sin_lookup(rot)), cs = abs(cos_lookup(rot));
  c = clamp_to_screen(c, (cs * hw + sn * hh) / TRIG_MAX_RATIO, (sn * hw + cs * hh) / TRIG_MAX_RATIO);
  fctx_set_rotation(&f, rot);
  fctx_set_offset(&f, FPointI(c.x, c.y));
  fctx_set_fill_color(&f, theme(color));
  fctx_begin_fill(&f);
  fctx_draw_string(&f, txt, s_font, GTextAlignmentCenter, FTextAnchorCapMiddle);
  fctx_end_fill(&f);
  fctx_deinit_context(&f);
}

void text_draw(GContext *ctx, const char *txt, GPoint c, int size, GColor color) {
  draw_text(ctx, txt, c, size, color, 0);
}

static int isqrt(int n) {
  int x = 0;
  while ((x + 1) * (x + 1) <= n) x++;
  return x;
}

// Curved text centered on `c`, on the circle through it around `center`: each
// glyph is placed and turned on its own. The bottom half runs the other way so
// it never reads upside down.
// ponytail: per-glyph widths, no kerning; fine for digits and %.
void text_draw_along(GContext *ctx, const char *txt, GPoint c, GPoint center, int size,
                     GColor color) {
  if (!s_font) return;
  int dx = c.x - center.x, dy = c.y - center.y;
  int r = isqrt(dx * dx + dy * dy);
  if (r == 0) return;
  int32_t a = atan2_lookup(dx, -dy);
  bool top = cos_lookup(a) >= 0;
  FContext f;
  fctx_init_context(&f, ctx);
  fctx_set_text_cap_height(&f, s_font, size);
  fctx_set_fill_color(&f, theme(color));
  fixed_t x = -fctx_string_width(&f, txt, s_font) / 2;  // arc offset of the next glyph
  for (const char *p = txt; *p;) {
    char ch[5];
    int n = 1;
    while (n < 4 && (p[n] & 0xC0) == 0x80) n++;  // one UTF-8 char
    memcpy(ch, p, n);
    ch[n] = '\0';
    p += n;
    fixed_t w = fctx_string_width(&f, ch, s_font);
    int32_t ang = a + (top ? 1 : -1) * (x + w / 2) * (TRIG_MAX_ANGLE * 10 / 63) / INT_TO_FIXED(r);
    x += w;
    fctx_set_rotation(&f, top ? ang : ang + TRIG_MAX_ANGLE / 2);
    fctx_set_offset(&f, FPoint(INT_TO_FIXED(center.x) + sin_lookup(ang) * INT_TO_FIXED(r) / TRIG_MAX_RATIO,
                               INT_TO_FIXED(center.y) - cos_lookup(ang) * INT_TO_FIXED(r) / TRIG_MAX_RATIO));
    fctx_begin_fill(&f);
    fctx_draw_string(&f, ch, s_font, GTextAlignmentCenter, FTextAnchorCapMiddle);
    fctx_end_fill(&f);
  }
  fctx_deinit_context(&f);
}

int text_width(GContext *ctx, const char *txt, int size) {
  if (!s_font) return 0;
  FContext f;
  fctx_init_context(&f, ctx);
  fctx_set_text_cap_height(&f, s_font, size);
  int w = FIXED_TO_INT(fctx_string_width(&f, txt, s_font));
  fctx_deinit_context(&f);
  return w;
}
