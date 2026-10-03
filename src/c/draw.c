#include "draw.h"
#include "settings.h"
#include <pebble-fctx/fctx.h>
#include <pebble-fctx/ffont.h>

#define PX_ANGLE (TRIG_MAX_ANGLE * 10 / 63)  // angle 1px spans at radius 1 (TRIG_MAX_ANGLE / 2π)

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

static bool color_light(GColor c) {
  return c.r * 299 + c.g * 587 + c.b * 114 > 1500;  // luma, channels 0..3
}

bool theme_light(void) {
  if (g_settings.scheme != SCHEME_ACCENT) return g_settings.scheme & SCHEME_LIGHT;
  return color_light((GColor){ .argb = g_settings.bg });
}

GColor ink_on(GColor fill) {
  // theme(Black) is light iff the scheme is: match it on a fill that isn't.
  return color_light(theme(fill)) == theme_light() ? GColorWhite : GColorBlack;
}

GColor fixed(GColor c) {
  c.a = 2;
  return c;
}

GColor theme(GColor c) {
  if (c.a == 2) { c.a = 3; return c; }  // fixed()
  bool gray = c.r == c.g && c.g == c.b;
  bool bright = c.r >= 2 || c.g >= 2 || c.b >= 2;  // fills vs tracks
  if (g_settings.scheme == SCHEME_ACCENT) {
    if (!gray && bright) return (GColor){ .argb = g_settings.accent };
    if (gray && c.r == 0) return (GColor){ .argb = g_settings.bg };
    if (!gray) c = GColorDarkGray;
    gray = true;
  } else if ((g_settings.scheme & SCHEME_MONO) && !gray) {
    c = bright ? GColorWhite : GColorDarkGray;
    gray = true;
  }
  if (theme_light() && gray) c.r = c.g = c.b = 3 - c.r;
  // Bright yellow washes out on a light background.
  else if (theme_light() && gcolor_equal(c, GColorYellow)) c = GColorChromeYellow;
  return c;
}

int32_t clamp_i32(int32_t v, int32_t lo, int32_t hi) {
  return v < lo ? lo : v > hi ? hi : v;
}

GPoint polar(GPoint c, int32_t angle, int r) {
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
  return px * PX_ANGLE / s->radius;
}

int slot_len(const Slot *s) {
  return abs(s->a1 - s->a0) * s->radius / PX_ANGLE;
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

static int isqrt(int n) {
  int x = 0;
  while ((x + 1) * (x + 1) <= n) x++;
  return x;
}

GPoint slot_corner(const Slot *s, int px) {
  GPoint m = slot_point(s, 50, 0);
  GPoint k = GPoint(m.x < s->center.x ? 0 : s_screen.w, m.y < s->center.y ? 0 : s_screen.h);
  int dx = k.x - s->center.x, dy = k.y - s->center.y;
  int r = s->radius + s->thickness / 2 + px, d = isqrt(dx * dx + dy * dy);
  return GPoint(s->center.x + dx * r / d, s->center.y + dy * r / d);
}

int slot_left_end(const Slot *s) {
  return slot_point(s, 100, 0).x < slot_point(s, 0, 0).x ? 100 : 0;
}

// Subpixel polar, for fctx paths.
static FPoint fpolar_f(FPoint c, int32_t angle, fixed_t r) {
  return FPoint(c.x + sin_lookup(angle) * r / TRIG_MAX_RATIO, c.y - cos_lookup(angle) * r / TRIG_MAX_RATIO);
}

static FPoint fpolar(GPoint c, int32_t angle, int r) {
  return fpolar_f(FPointI(c.x, c.y), angle, INT_TO_FIXED(r));
}

// Arc with half-circle caps, one antialiased fctx path: outer edge, cap at b,
// inner edge back, cap at a. Edges are ~4px chords. from/to in either order.
void slot_arc(GContext *ctx, const Slot *s, int from_pct, int to_pct, GColor color) {
  int32_t a = slot_angle(s, from_pct), b = slot_angle(s, to_pct);
  if (a > b) { int32_t t = a; a = b; b = t; }
  FPoint c = FPointI(s->center.x, s->center.y);
  fixed_t r = INT_TO_FIXED(s->radius), h = INT_TO_FIXED(s->thickness) / 2;
  int n = 1 + (b - a) * (s->radius + s->thickness) * 3 / (TRIG_MAX_ANGLE * 2);  // 2πR/4
  const int32_t STEP = TRIG_MAX_ANGLE / 12;  // 30° around a cap
  FContext f;
  fctx_init_context(&f, ctx);
  fctx_set_fill_color(&f, theme(color));
  fctx_begin_fill(&f);
  fctx_move_to(&f, fpolar_f(c, a, r + h));
  for (int i = 1; i <= n; i++) fctx_line_to(&f, fpolar_f(c, a + (b - a) * i / n, r + h));
  for (int k = 1; k <= 6; k++) fctx_line_to(&f, fpolar_f(fpolar_f(c, b, r), b + k * STEP, h));
  for (int i = n - 1; i >= 0; i--) fctx_line_to(&f, fpolar_f(c, a + (b - a) * i / n, r - h));
  for (int k = 7; k < 12; k++) fctx_line_to(&f, fpolar_f(fpolar_f(c, a, r), a + k * STEP, h));
  fctx_close_path(&f);
  fctx_end_fill(&f);
  fctx_deinit_context(&f);
}

// One antialiased fctx path: the four sides, each corner a curve pulled
// toward the sharp corner. Sides are straight chords.
// ponytail: chords sag <0.3px at box widths; sample the arcs if boxes grow.
void slot_box(GContext *ctx, const Slot *s, int from_pct, int to_pct, int r, GColor color) {
  int32_t a = slot_angle(s, from_pct), b = slot_angle(s, to_pct);
  if (a > b) { int32_t t = a; a = b; b = t; }
  int outer = s->radius + s->thickness / 2, inner = outer - s->thickness;
  int32_t io = r * PX_ANGLE / outer, ii = r * PX_ANGLE / inner;
  GPoint c = s->center;
  FContext f;
  fctx_init_context(&f, ctx);
  fctx_set_fill_color(&f, theme(color));
  fctx_begin_fill(&f);
  fctx_move_to(&f, fpolar(c, a + io, outer));
  fctx_line_to(&f, fpolar(c, b - io, outer));
  fctx_curve_to(&f, fpolar(c, b, outer), fpolar(c, b, outer), fpolar(c, b, outer - r));
  fctx_line_to(&f, fpolar(c, b, inner + r));
  fctx_curve_to(&f, fpolar(c, b, inner), fpolar(c, b, inner), fpolar(c, b - ii, inner));
  fctx_line_to(&f, fpolar(c, a + ii, inner));
  fctx_curve_to(&f, fpolar(c, a, inner), fpolar(c, a, inner), fpolar(c, a, inner + r));
  fctx_line_to(&f, fpolar(c, a, outer - r));
  fctx_curve_to(&f, fpolar(c, a, outer), fpolar(c, a, outer), fpolar(c, a + io, outer));
  fctx_close_path(&f);
  fctx_end_fill(&f);
  fctx_deinit_context(&f);
}

// Polygon of 10° chords, the points below the cut pulled up onto it.
void disc_fill(GContext *ctx, GPoint c, int r, int cut, GColor color) {
  fixed_t max_y = INT_TO_FIXED(c.y + cut);
  FContext f;
  fctx_init_context(&f, ctx);
  fctx_set_fill_color(&f, theme(color));
  fctx_begin_fill(&f);
  for (int i = 0; i < 36; i++) {
    FPoint p = fpolar(c, i * TRIG_MAX_ANGLE / 36, r);
    if (p.y > max_y) p.y = max_y;
    if (i) fctx_line_to(&f, p); else fctx_move_to(&f, p);
  }
  fctx_close_path(&f);
  fctx_end_fill(&f);
  fctx_deinit_context(&f);
}

// A capsule of 30° chords: half-circle caps around p and q, `h` the half width.
static void capsule(FContext *f, FPoint p, FPoint q, int32_t angle, fixed_t h) {
  for (int i = 0; i < 14; i++) {
    FPoint pt = fpolar_f(i < 7 ? q : p, angle + (i < 7 ? i - 3 : i - 4) * TRIG_MAX_ANGLE / 12, h);
    if (i) fctx_line_to(f, pt); else fctx_move_to(f, pt);
  }
  fctx_close_path(f);
}

void ray_bar(GContext *ctx, GPoint c, int32_t angle, int r0, int r1, int w, GColor color) {
  fixed_t h = INT_TO_FIXED(w) / 2;
  FPoint o = FPointI(c.x, c.y);
  FPoint p = fpolar_f(o, angle, INT_TO_FIXED(r0) + h), q = fpolar_f(o, angle, INT_TO_FIXED(r1) - h);
  FContext f;
  fctx_init_context(&f, ctx);
  fctx_set_fill_color(&f, theme(color));
  fctx_begin_fill(&f);
  capsule(&f, p, q, angle, h);
  fctx_end_fill(&f);
  fctx_deinit_context(&f);
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

void text_draw(GContext *ctx, const char *txt, GPoint c, int size, GColor color) {
  if (!s_font || !txt[0]) return;
  FContext f;
  fctx_init_context(&f, ctx);
  fctx_set_text_cap_height(&f, s_font, size);
  c = clamp_to_screen(c, FIXED_TO_INT(fctx_string_width(&f, txt, s_font)) / 2, size / 2 + 1);
  fctx_set_rotation(&f, 0);
  fctx_set_offset(&f, FPointI(c.x, c.y));
  fctx_set_fill_color(&f, theme(color));
  fctx_begin_fill(&f);
  fctx_draw_string(&f, txt, s_font, GTextAlignmentCenter, FTextAnchorCapMiddle);
  fctx_end_fill(&f);
  fctx_deinit_context(&f);
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
    int32_t ang = a + (top ? 1 : -1) * (x + w / 2) * PX_ANGLE / INT_TO_FIXED(r);
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
