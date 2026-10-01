// Settings model, shared by index.js, weather.js and the settings page
// (scripts/inline-config.mjs inlines this file into config.html, so keep it
// plain ES5 with no requires).

var SETTINGS_KEY = 'ultra-settings';  // last saved page result

// Values match ComplicationId in src/c/complications/complications.h.
var COMPLICATIONS = [
  { label: 'Steps', value: 1 },
  { label: 'Temperature', value: 2 },
  { label: 'Battery', value: 3 },
  { label: 'Chance of rain', value: 4 },
  { label: 'Calendar', value: 6 },
  { label: 'None', value: 0 }
];

var CORNERS = [
  { key: 'SLOT_TL', label: 'Top left' },
  { key: 'SLOT_TR', label: 'Top right' },
  { key: 'SLOT_BL', label: 'Bottom left' },
  { key: 'SLOT_BR', label: 'Bottom right' }
];

// Index = SCHEME value, see SCHEME_LIGHT / SCHEME_MONO / SCHEME_ACCENT in src/c/settings.h
var SCHEMES = ['Black', 'White', 'White on black', 'Black on white', 'Accent'];
var SCHEME_ACCENT = 4;

// Mirrors g_settings in C. UNITS never reaches the watch: weather.js asks
// Open-Meteo for the chosen unit. Colors are GColor8 argb (0xC0 = black,
// 0xF8 = chrome yellow).
var DEFAULTS = { SLOT_TL: 1, SLOT_TR: 2, SLOT_BL: 3, SLOT_BR: 4, SCHEME: 0, UNITS: 0, STEP_GOAL: 10000, SECONDS: 0,
  BG_COLOR: 0xC0, ACCENT_COLOR: 0xF8 };

function withDefaults(saved) {
  var s = {};
  for (var k in DEFAULTS) s[k] = saved && saved[k] !== undefined ? Number(saved[k]) : DEFAULTS[k];
  return s;
}

function savedSettings() {
  try {
    return withDefaults(JSON.parse(localStorage.getItem(SETTINGS_KEY)));
  } catch (e) {
    return withDefaults();
  }
}

function toMessage(settings) {
  var msg = {};
  for (var k in DEFAULTS) if (k !== 'UNITS') msg[k] = Number(settings[k]);
  return msg;
}

if (typeof module === 'object') {
  module.exports = {
    SETTINGS_KEY: SETTINGS_KEY, COMPLICATIONS: COMPLICATIONS, CORNERS: CORNERS, SCHEMES: SCHEMES, SCHEME_ACCENT: SCHEME_ACCENT,
    DEFAULTS: DEFAULTS, withDefaults: withDefaults, savedSettings: savedSettings, toMessage: toMessage
  };
}
