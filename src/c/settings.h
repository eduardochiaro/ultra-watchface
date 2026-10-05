#pragma once
#include <pebble.h>

// Corner slots, clockwise from top-left. Same order as the SLOT_* message keys.
typedef enum { SLOT_POS_TL, SLOT_POS_TR, SLOT_POS_BR, SLOT_POS_BL, SLOT_POS_COUNT } SlotPos;
// Subdials, same order as the CENTER_* message keys.
typedef enum { CENTER_POS_T, CENTER_POS_L, CENTER_POS_R, CENTER_POS_B, CENTER_POS_COUNT } CenterPos;

// Color scheme bits. 0 black, 1 white, 2 white on black, 3 black on white.
// SCHEME_ACCENT is a value, not a bit: custom background + accent colors.
enum { SCHEME_LIGHT = 1, SCHEME_MONO = 2, SCHEME_ACCENT = 4 };

// Hour and minute hands: plain lines, or a thin stem and a rounded bar, solid or
// an outline around the background color.
// Pointer: the bar with a pointed tip. Sword: widest by the stem, tapering to the tip.
// Dauphine: no stem, a long kite out of the pin, one half of it shaded.
// None: no hour or minute hand, for a face that tells the time in a complication.
typedef enum { HANDS_LINE, HANDS_BAR, HANDS_OUTLINE, HANDS_POINTER, HANDS_SWORD, HANDS_DAUPHINE, HANDS_NONE, HANDS_COUNT } HandStyle;

// The dial's ring. Minimal: ticks only, no numerals. Sport: a white band of
// minute numerals over an accent ring. Chronograph: the band alone, in black and
// white, the hours upright on it. All three leave a larger center: the subdials
// grow to fill it and the hands run longer. Roman: the default, its hours in
// Roman numerals along the dial. Tachymeter: ticks for the seconds around a
// band in the accent, 10..60 on it and a dot between each; the larger center too.
typedef enum { RING_DEFAULT, RING_MINIMAL, RING_SPORT, RING_CHRONO, RING_ROMAN, RING_TACHY, RING_COUNT } RingStyle;

typedef struct {
  uint8_t slots[SLOT_POS_COUNT];   // ComplicationId per corner
  bool seconds;
  int32_t step_goal;
  uint8_t scheme;                  // appended: older saves read without it
  uint8_t bg, accent;              // GColor8 argb, used by SCHEME_ACCENT
  // Appended. imperial lands on older saves' trailing pad byte, which was 0.
  bool imperial;                   // distance and elevation units
  uint8_t center[CENTER_POS_COUNT]; // ComplicationId per subdial
  // COMP_CUSTOM text per corner and subdial; font glyphs only (config.js cleans it).
  char slot_text[SLOT_POS_COUNT][13];
  char center_text[CENTER_POS_COUNT][5];  // a monogram: 4 at most
  // Appended: 0 on older saves.
  uint8_t hands;                   // HandStyle
  // GColor8 argb, fixed in any scheme; 0 = the scheme's own. hand_color is the
  // hour hand's; second_color is also the pin's and the 12/3/6/9 notches'.
  uint8_t hand_color, second_color, minute_color;
  uint8_t ring;                    // RingStyle
  uint8_t band_color;              // the band's; 0 = the scheme's white, tachymeter's the seconds color
  // COMP_ZONE: minutes from UTC, per corner then per subdial. Its name ("PST")
  // is that place's slot_text or center_text.
  int16_t zone[SLOT_POS_COUNT + CENTER_POS_COUNT];
  bool sweep;                      // the seconds hand glides, not ticks; with `seconds` only
} Settings;

extern Settings g_settings;
