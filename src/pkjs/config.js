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
  { label: 'Humidity', value: 13 },
  { label: 'Wind', value: 25 },
  { label: 'Calendar', value: 6 },
  { label: 'Heart rate', value: 8 },
  { label: 'Distance', value: 9 },
  { label: 'Air quality', value: 7 },
  { label: 'UV index', value: 11 },
  { label: 'Elevation', value: 10 },
  { label: 'Sunrise / sunset', value: 23 },
  { label: '.beat time', value: 24 },
  { label: 'Text', value: 14 },
  { label: 'None', value: 0 }
];

// Subdials have their own designs, so their own list.
var CENTER_COMPLICATIONS = [
  { label: 'Temperature', value: 2 },
  { label: 'Chance of rain', value: 4 },
  { label: 'Humidity', value: 13 },
  { label: 'Wind', value: 25 },
  { label: 'Air quality', value: 7 },
  { label: 'UV index', value: 11 },
  { label: 'Weather', value: 12 },
  { label: 'Calendar', value: 6 },
  { label: 'Calendar (plain)', value: 26 },
  { label: 'Battery', value: 3 },
  { label: 'Heart rate', value: 8 },
  { label: 'Distance', value: 9 },
  { label: 'Elevation', value: 10 },
  { label: 'Sunrise / sunset', value: 23 },
  { label: '.beat time', value: 24 },
  { label: 'Text', value: 14 },
  { label: 'None', value: 0 }
];

// Custom API complications (src/pkjs/api.js): settings.APIS[i] is complication
// API_ID + i in any place. Same numbers as COMP_API and API_MAX in complications.h.
var API_ID = 15, API_MAX = 8;
var API_TYPES = ['Text', 'Bar', 'Gauge'];  // index = type, see API_TEXT.. in complications/api.c

var CORNERS = [
  { key: 'SLOT_TL', label: 'Top left' },
  { key: 'SLOT_TR', label: 'Top right' },
  { key: 'SLOT_BL', label: 'Bottom left' },
  { key: 'SLOT_BR', label: 'Bottom right' }
];

var CENTERS = [
  { key: 'CENTER_T', label: 'Top' },
  { key: 'CENTER_L', label: 'Left' },
  { key: 'CENTER_R', label: 'Right' },
  { key: 'CENTER_B', label: 'Bottom' }
];

// Index = SCHEME value, see SCHEME_LIGHT / SCHEME_MONO / SCHEME_ACCENT in src/c/settings.h
var SCHEMES = ['Black', 'White', 'White on black', 'Black on white', 'Accent'];
var SCHEME_ACCENT = 4;

// Index = HANDS value, see HandStyle in src/c/settings.h
var HANDS = ['Line', 'Bar', 'Outline'];

// Mirrors g_settings in C. weather.js asks Open-Meteo for the chosen
// temperature unit; the watch converts distance and elevation. Colors are GColor8 argb (0xC0 = black,
// 0xF8 = chrome yellow); a hand color of 0 follows the scheme.
var DEFAULTS = { SLOT_TL: 1, SLOT_TR: 2, SLOT_BL: 3, SLOT_BR: 4, CENTER_T: 14, CENTER_L: 10, CENTER_R: 12, CENTER_B: 6, SCHEME: 0, UNITS: 0, STEP_GOAL: 10000, SECONDS: 0,
  BG_COLOR: 0xC0, ACCENT_COLOR: 0xF8, HANDS: 0, HAND_COLOR: 0, MINUTE_COLOR: 0, SECOND_COLOR: 0,
  // The Text complication's text, per place: TEXT_ + the SLOT_/CENTER_ suffix.
  TEXT_TL: '', TEXT_TR: '', TEXT_BL: '', TEXT_BR: '', TEXT_T: 'PB', TEXT_L: '', TEXT_R: '', TEXT_B: '',
  // Custom API complications. Stays on the phone: the watch gets what to draw (api.js).
  APIS: [] };

// Only characters the watch font has, `n` at most.
function clean(s, n) {
  return String(s).replace(/[^ %,\-./0-9:A-Za-z]/g, '').slice(0, n);
}

// Corners fit 12, subdials 4 (TEXT_T etc.).
function cleanText(k, s) {
  return clean(s, k.length === 6 ? 4 : 12);
}

// header, text, min and max may hold {{path.to[0].value}} patterns, filled from
// the response. title names it in Settings; header is what the face shows, 4
// characters once filled. Both were one `name` once.
function cleanApis(list) {
  return (Array.isArray(list) ? list : []).slice(0, API_MAX).map(function (a) {
    a = a || {};
    return {
      url: String(a.url || '').trim(),
      freq: a.freq > 0 ? Math.max(1, Math.round(a.freq)) : 10,  // minutes, 1 at least
      type: Math.min(API_TYPES.length - 1, Math.max(0, Math.round(Number(a.type)) || 0)),
      title: String(a.title == null ? a.name || '' : a.title).trim(),
      header: String(a.header == null ? a.name || '' : a.header).trim(),
      text: String(a.text || ''),
      min: String(a.min == null ? 0 : a.min).trim(),
      max: String(a.max == null ? 100 : a.max).trim()
    };
  });
}

function value(k, v) {
  if (k === 'APIS') return cleanApis(v);
  return typeof DEFAULTS[k] === 'string' ? cleanText(k, v) : Number(v);
}

function withDefaults(saved) {
  var s = {};
  for (var k in DEFAULTS) s[k] = value(k, saved && saved[k] !== undefined ? saved[k] : DEFAULTS[k]);
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
  for (var k in DEFAULTS) if (k !== 'APIS') msg[k] = value(k, settings[k]);
  return msg;
}

if (typeof module === 'object') {
  module.exports = {
    SETTINGS_KEY: SETTINGS_KEY, COMPLICATIONS: COMPLICATIONS, CENTER_COMPLICATIONS: CENTER_COMPLICATIONS,
    CORNERS: CORNERS, CENTERS: CENTERS, SCHEMES: SCHEMES, SCHEME_ACCENT: SCHEME_ACCENT, HANDS: HANDS,
    API_ID: API_ID, API_MAX: API_MAX, API_TYPES: API_TYPES,
    DEFAULTS: DEFAULTS, clean: clean, cleanText: cleanText, withDefaults: withDefaults, savedSettings: savedSettings, toMessage: toMessage
  };
}
