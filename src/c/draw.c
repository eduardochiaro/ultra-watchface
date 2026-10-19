#include "draw.h"
#include "settings.h"
#include <pebble-fctx/fctx.h>
#include <pebble-fctx/ffont.h>

#define PX_ANGLE (TRIG_MAX_ANGLE * 10 / 63)  // angle 1px spans at radius 1 (TRIG_MAX_ANGLE / 2π)

// The font's header, glyph index and table only: an outline is read from flash
// as its glyph is drawn, into s_glyph. The whole font would take the heap fctx
// needs to antialias on 64KB watches.
static FFont *s_font;
static ResHandle s_font_res;
static uint32_t s_paths;    // where the outlines start in the resource
static uint8_t *s_glyph;    // the largest outline's size
static GSize s_screen;

// draw_zoom()
static int s_zoom = 100;
static GPoint s_pivot;

void draw_zoom(GPoint pivot, int pct) {
  s_pivot = pivot;
  s_zoom = pct;
}

static int32_t zoom(int32_t v) {
  return v * s_zoom / 100;
}

static GPoint zoom_point(GPoint p) {
  return GPoint(s_pivot.x + zoom(p.x - s_pivot.x), s_pivot.y + zoom(p.y - s_pivot.y));
}

// For a path in screen coordinates.
static void zoom_path(FContext *f) {
  fctx_set_scale(f, FPoint(100, 100), FPoint(s_zoom, s_zoom));
  fctx_set_offset(f, FPoint(INT_TO_FIXED(s_pivot.x) - zoom(INT_TO_FIXED(s_pivot.x)),
                            INT_TO_FIXED(s_pivot.y) - zoom(INT_TO_FIXED(s_pivot.y))));
}

void draw_init(GSize screen) {
  s_screen = screen;
  s_font_res = resource_get_handle(RESOURCE_ID_FONT);
  FFont head;
  resource_load_byte_range(s_font_res, 0, (uint8_t *)&head, sizeof(head));
  s_paths = sizeof(FFont) + head.glyph_index_length * sizeof(FGlyphRange) + head.glyph_table_length * sizeof(FGlyph);
  s_font = malloc(s_paths);
  if (s_font) {
    resource_load_byte_range(s_font_res, 0, (uint8_t *)s_font, s_paths);
    const FGlyph *g = (const FGlyph *)((uint8_t *)s_font + s_paths) - head.glyph_table_length;
    int max = 0;
    for (int k = 0; k < head.glyph_table_length; k++) {
      max = g[k].path_data_length > max ? g[k].path_data_length : max;
    }
    s_glyph = malloc(max);
  }
#if defined(PBL_COLOR)
  fctx_enable_aa(true);
#endif
}

void draw_deinit(void) {
  free(s_glyph);
  free(s_font);
}

static bool color_light(GColor c) {
  return c.r * 299 + c.g * 587 + c.b * 114 > 1500;  // luma, channels 0..3
}

bool theme_light(void) {
  if (g_settings.scheme != SCHEME_ACCENT) {
    return g_settings.scheme & SCHEME_LIGHT;
  }
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
    if (!gray && bright) {
      return (GColor){ .argb = g_settings.accent };
    }
    if (gray && c.r == 0) {
      return (GColor){ .argb = g_settings.bg };
    }
    if (!gray) {
      c = GColorDarkGray;
    }
    gray = true;
  } else if ((g_settings.scheme & SCHEME_MONO) && !gray) {
    c = bright ? GColorWhite : GColorDarkGray;
    gray = true;
  }
  if (theme_light() && gray) {
    c.r = c.g = c.b = 3 - c.r;
  } else if (theme_light() && gcolor_equal(c, GColorYellow)) {
    // Bright yellow washes out on a light background.
    c = GColorChromeYellow;
  }
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

// Bit by bit, from the highest pair down.
static int isqrt(int n) {
  int x = 0;
  for (int b = 1 << 30; b; b >>= 2) {
    if (n >= x + b) {
      n -= x + b;
      x = (x >> 1) + b;
    } else {
      x >>= 1;
    }
  }
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

// The rectangular ring, see ring_shape(). s_sq_hw 0: round.
static int16_t s_sq_out, s_sq_hw, s_sq_hh, s_sq_rc;

void ring_shape(int out, int hw, int hh, int rc) {
  s_sq_out = out;
  s_sq_hw = out ? hw : 0;
  s_sq_hh = hh;
  s_sq_rc = rc;
}

static int32_t abs_i32(int32_t v) {
  return v < 0 ? -v : v;
}

// The rounded rect `r` stands for: `out - r` in from the outer one. Concentric,
// so its corners round less, 4px at least.
static void ring_rect(fixed_t r, fixed_t *hw, fixed_t *hh, fixed_t *rc) {
  fixed_t d = INT_TO_FIXED(s_sq_out) - r;
  *hw = INT_TO_FIXED(s_sq_hw) - d;
  *hh = INT_TO_FIXED(s_sq_hh) - d;
  *rc = clamp_i32(INT_TO_FIXED(s_sq_rc) - d, INT_TO_FIXED(4), *hw < *hh ? *hw : *hh);
}

// fpolar_f() on the ring: where the ray at `angle` meets that rounded rect.
static FPoint ring_fpoint(FPoint o, int32_t angle, fixed_t r) {
  if (!s_sq_hw) {
    return fpolar_f(o, angle, r);
  }
  fixed_t hw, hh, rc;
  ring_rect(r, &hw, &hh, &rc);
  int32_t sx = sin_lookup(angle), sy = -cos_lookup(angle), ax = abs_i32(sx), ay = abs_i32(sy);
  // Where it leaves the square-cornered rect; in a corner, its circle instead.
  fixed_t t = ax * hh > ay * hw ? hw * TRIG_MAX_RATIO / ax : hh * TRIG_MAX_RATIO / ay;
  fixed_t px = t * sx / TRIG_MAX_RATIO, py = t * sy / TRIG_MAX_RATIO;
  if (abs_i32(px) > hw - rc && abs_i32(py) > hh - rc) {
    fixed_t kx = px < 0 ? rc - hw : hw - rc, ky = py < 0 ? rc - hh : hh - rc;
    int32_t b = (sx * kx + sy * ky) / TRIG_MAX_RATIO;
    t = b + isqrt(b * b - kx * kx - ky * ky + rc * rc);
  }
  return FPoint(o.x + t * sx / TRIG_MAX_RATIO, o.y + t * sy / TRIG_MAX_RATIO);
}

GPoint ring_point(GPoint c, int32_t angle, int r) {
  FPoint p = ring_fpoint(FPointI(c.x, c.y), angle, INT_TO_FIXED(r));
  return GPoint((p.x + FIX1 / 2) / FIX1, (p.y + FIX1 / 2) / FIX1);
}

void ring_band(GContext *ctx, GPoint c, int r, int thickness, GColor color) {
  if (!s_sq_hw) {
    graphics_context_set_fill_color(ctx, theme(color));
    graphics_fill_radial(ctx, GRect(c.x - r, c.y - r, 2 * r + 1, 2 * r + 1), GOvalScaleModeFitCircle, thickness, 0, TRIG_MAX_ANGLE);
    return;
  }
  // The outer rounded rect clockwise, the inner one back: the band between. Each
  // corner in 15° steps, from 12 o'clock's side on; the sides join them straight.
  static const int8_t SX[4] = { 1, 1, -1, -1 }, SY[4] = { -1, 1, 1, -1 };
  FPoint o = FPointI(c.x, c.y);
  FContext f;
  fctx_init_context(&f, ctx);
  fctx_set_fill_color(&f, theme(color));
  fctx_begin_fill(&f);
  for (int inner = 0; inner < 2; inner++) {
    fixed_t hw, hh, rc;
    ring_rect(INT_TO_FIXED(inner ? r - thickness : r), &hw, &hh, &rc);
    for (int i = 0; i < 28; i++) {
      int k = inner ? 27 - i : i, q = k / 7;
      FPoint corner = FPoint(o.x + SX[q] * (hw - rc), o.y + SY[q] * (hh - rc));
      FPoint p = fpolar_f(corner, q * TRIG_MAX_ANGLE / 4 + k % 7 * TRIG_MAX_ANGLE / 24, rc);
      if (i) {
        fctx_line_to(&f, p);
      } else {
        fctx_move_to(&f, p);
      }
    }
    fctx_close_path(&f);
  }
  fctx_end_fill(&f);
  fctx_deinit_context(&f);
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
  zoom_path(&f);
  fctx_set_fill_color(&f, theme(color));
  fctx_begin_fill(&f);
  fctx_move_to(&f, fpolar_f(c, a, r + h));
  for (int i = 1; i <= n; i++) {
    fctx_line_to(&f, fpolar_f(c, a + (b - a) * i / n, r + h));
  }
  for (int k = 1; k <= 6; k++) {
    fctx_line_to(&f, fpolar_f(fpolar_f(c, b, r), b + k * STEP, h));
  }
  for (int i = n - 1; i >= 0; i--) {
    fctx_line_to(&f, fpolar_f(c, a + (b - a) * i / n, r - h));
  }
  for (int k = 7; k < 12; k++) {
    fctx_line_to(&f, fpolar_f(fpolar_f(c, a, r), a + k * STEP, h));
  }
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
  zoom_path(&f);
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
  zoom_path(&f);
  fctx_set_fill_color(&f, theme(color));
  fctx_begin_fill(&f);
  for (int i = 0; i < 36; i++) {
    FPoint p = fpolar(c, i * TRIG_MAX_ANGLE / 36, r);
    if (p.y > max_y) {
      p.y = max_y;
    }
    if (i) {
      fctx_line_to(&f, p);
    } else {
      fctx_move_to(&f, p);
    }
  }
  fctx_close_path(&f);
  fctx_end_fill(&f);
  fctx_deinit_context(&f);
}

// The limb down the lit side in 15° chords, then back up the terminator: the
// same half ellipse squeezed by cos(phase), on the dark side once past half.
void moon_fill(GContext *ctx, GPoint c, int r, int32_t phase, GColor color) {
  const int N = 12;
  int side = phase < TRIG_MAX_ANGLE / 2 ? 1 : -1;
  int32_t k = cos_lookup(phase);
  FPoint o = FPointI(c.x, c.y);
  fixed_t R = INT_TO_FIXED(r);
  FContext f;
  fctx_init_context(&f, ctx);
  zoom_path(&f);
  fctx_set_fill_color(&f, theme(color));
  fctx_begin_fill(&f);
  for (int i = 0; i < 2 * N; i++) {
    int32_t a = (i <= N ? i : 2 * N - i) * TRIG_MAX_ANGLE / (2 * N);
    fixed_t w = R * sin_lookup(a) / TRIG_MAX_RATIO;
    if (i > N) {
      w = w * k / TRIG_MAX_RATIO;
    }
    FPoint p = FPoint(o.x + side * w, o.y - R * cos_lookup(a) / TRIG_MAX_RATIO);
    if (i) {
      fctx_line_to(&f, p);
    } else {
      fctx_move_to(&f, p);
    }
  }
  fctx_close_path(&f);
  fctx_end_fill(&f);
  fctx_deinit_context(&f);
}

// A capsule of 30° chords: half-circle caps around p and q, `h` the half width.
static void capsule(FContext *f, FPoint p, FPoint q, int32_t angle, fixed_t h) {
  for (int i = 0; i < 14; i++) {
    FPoint pt = fpolar_f(i < 7 ? q : p, angle + (i < 7 ? i - 3 : i - 4) * TRIG_MAX_ANGLE / 12, h);
    if (i) {
      fctx_line_to(f, pt);
    } else {
      fctx_move_to(f, pt);
    }
  }
  fctx_close_path(f);
}

void rect_fill(GContext *ctx, GRect rect, int r, GColor color) {
  graphics_context_set_fill_color(ctx, theme(color));
  graphics_fill_rect(ctx, (GRect){ zoom_point(rect.origin), GSize(zoom(rect.size.w), zoom(rect.size.h)) },
                     zoom(r), r ? GCornersAll : GCornerNone);
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

void ray_ticks(GContext *ctx, GPoint c, int32_t a0, int n, int r0, int r1, int w, GColor color) {
  FPoint o = FPointI(c.x, c.y);
  fixed_t h = INT_TO_FIXED(w) / 2;
  FContext f;
  fctx_init_context(&f, ctx);
  fctx_set_fill_color(&f, theme(color));
  fctx_begin_fill(&f);
  for (int i = 0; i < n; i++) {
    int32_t a = a0 + i * TRIG_MAX_ANGLE / n, across = a + TRIG_MAX_ANGLE / 4;
    FPoint p = ring_fpoint(o, a, INT_TO_FIXED(r0)), q = ring_fpoint(o, a, INT_TO_FIXED(r1));
    fctx_move_to(&f, fpolar_f(p, across, h));
    fctx_line_to(&f, fpolar_f(q, across, h));
    fctx_line_to(&f, fpolar_f(q, across, -h));
    fctx_line_to(&f, fpolar_f(p, across, -h));
    fctx_close_path(&f);
  }
  fctx_end_fill(&f);
  fctx_deinit_context(&f);
}

void ray_poly(GContext *ctx, GPoint c, int32_t angle, const GPoint *pts, int n, GColor color) {
  FPoint o = FPointI(c.x, c.y);
  FContext f;
  fctx_init_context(&f, ctx);
  fctx_set_fill_color(&f, theme(color));
  fctx_begin_fill(&f);
  for (int i = 0; i < n; i++) {
    FPoint p = fpolar_f(fpolar_f(o, angle, INT_TO_FIXED(pts[i].x)), angle + TRIG_MAX_ANGLE / 4, INT_TO_FIXED(pts[i].y));
    if (i) {
      fctx_line_to(&f, p);
    } else {
      fctx_move_to(&f, p);
    }
  }
  fctx_close_path(&f);
  fctx_end_fill(&f);
  fctx_deinit_context(&f);
}

// fctx, not graphics_fill_circle: that one snaps to a pixel, off the arc's centerline.
void slot_dot(GContext *ctx, const Slot *s, int pct, int r, int edge, GColor fill, GColor ring) {
  FPoint p = fpolar(s->center, slot_angle(s, pct), s->radius), o = FPointI(s_pivot.x, s_pivot.y);
  p = FPoint(o.x + zoom(p.x - o.x), o.y + zoom(p.y - o.y));
  FContext f;
  fctx_init_context(&f, ctx);
  for (int i = 0; i < 2; i++) {
    fctx_set_fill_color(&f, theme(i ? fill : ring));
    fctx_begin_fill(&f);
    fctx_plot_circle(&f, &p, zoom(INT_TO_FIXED(i ? r : r + edge)));
    fctx_end_fill(&f);
  }
  fctx_deinit_context(&f);
}

// Pull a box back inside the screen: the bounds on rect, the circle on round.
static GPoint clamp_to_screen(GPoint c, int hw, int hh) {
  const int m = 2;
#if defined(PBL_ROUND)
  int cx = s_screen.w / 2, cy = s_screen.h / 2, r = cx - m;
  int dx = c.x - cx, dy = c.y - cy;
  for (int i = 0; i < 32; i++) {
    int ex = abs(dx) + hw, ey = abs(dy) + hh;
    if (ex * ex + ey * ey <= r * r) {
      break;
    }
    dx = dx * 15 / 16;
    dy = dy * 15 / 16;
  }
  return GPoint(cx + dx, cy + dy);
#else
  return GPoint(clamp_i32(c.x, hw + m, s_screen.w - hw - m),
                clamp_i32(c.y, hh + m, s_screen.h - hh - m));
#endif
}

// fctx_draw_string, centered on the cap middle, each outline read from flash
// as it is drawn (s_font has none).
static void draw_string(FContext *f, const char *txt) {
  FPoint advance = { 0, -s_font->cap_height / 2 };
  fixed_t width = 0;
  for (int pass = 0; pass < 2; pass++) {
    advance.x = -width / 2;  // the first pass measures it
    for (const uint8_t *p = (const uint8_t *)txt; *p;) {
      uint16_t cp = *p++;
      if (cp >= 0xE0) {  // UTF-8: the font has nothing past U+FFFF
        cp = (cp & 0x0F) << 12 | (p[0] & 0x3F) << 6 | (p[1] & 0x3F);
        p += 2;
      } else if (cp >= 0xC0) {
        cp = (cp & 0x1F) << 6 | (p[0] & 0x3F);
        p++;
      }
      FGlyph *g = ffont_glyph_info(s_font, cp);
      if (!g) {
        continue;
      }
      if (pass) {
        resource_load_byte_range(s_font_res, s_paths + g->path_data_offset, s_glyph, g->path_data_length);
        fctx_draw_commands(f, advance, s_glyph, g->path_data_length);
        advance.x += g->horiz_adv_x;
      } else {
        width += g->horiz_adv_x;
      }
    }
  }
}

void text_draw(GContext *ctx, const char *txt, GPoint c, int size, GColor color) {
  if (!s_glyph || !txt[0]) {
    return;
  }
  c = zoom_point(c);
  size = zoom(size);
  FContext f;
  fctx_init_context(&f, ctx);
  fctx_set_text_cap_height(&f, s_font, size);
  c = clamp_to_screen(c, FIXED_TO_INT(fctx_string_width(&f, txt, s_font)) / 2, size / 2 + 1);
  fctx_set_rotation(&f, 0);
  fctx_set_offset(&f, FPointI(c.x, c.y));
  fctx_set_fill_color(&f, theme(color));
  fctx_begin_fill(&f);
  draw_string(&f, txt);
  fctx_end_fill(&f);
  fctx_deinit_context(&f);
}

// Curved text centered on `c`, on the circle through it around `center`: each
// glyph is placed and turned on its own. The bottom half runs the other way so
// it never reads upside down.
// ponytail: per-glyph widths, no kerning; fine for digits and %.
void text_draw_along(GContext *ctx, const char *txt, GPoint c, GPoint center, int size,
                     GColor color) {
  int dx = c.x - center.x, dy = c.y - center.y;
  text_draw_arc(ctx, txt, center, atan2_lookup(dx, -dy), isqrt(dx * dx + dy * dy), size, color);
}

void text_draw_arc(GContext *ctx, const char *txt, GPoint center, int32_t a, int r, int size,
                   GColor color) {
  if (!s_glyph || r == 0) {
    return;
  }
  bool top = cos_lookup(a) >= 0;
  FContext f;
  fctx_init_context(&f, ctx);
  fctx_set_text_cap_height(&f, s_font, size);
  fctx_set_fill_color(&f, theme(color));
  fixed_t x = -fctx_string_width(&f, txt, s_font) / 2;  // arc offset of the next glyph
  for (const char *p = txt; *p;) {
    char ch[5];
    int n = 1;
    while (n < 4 && (p[n] & 0xC0) == 0x80) {
      n++;  // one UTF-8 char
    }
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
    draw_string(&f, ch);
    fctx_end_fill(&f);
  }
  fctx_deinit_context(&f);
}

int text_width(GContext *ctx, const char *txt, int size) {
  if (!s_font) {
    return 0;
  }
  FContext f;
  fctx_init_context(&f, ctx);
  fctx_set_text_cap_height(&f, s_font, size);
  int w = FIXED_TO_INT(fctx_string_width(&f, txt, s_font));
  fctx_deinit_context(&f);
  return w;
}
