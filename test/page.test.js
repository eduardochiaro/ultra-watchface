// Run: npm test. Runs the built settings page against a stub DOM and checks
// the preview draws and the message the watch gets.
var assert = require('assert');
var vm = require('vm');
var execSync = require('child_process').execSync;
var config = require('../src/pkjs/config');

// Message: ints, defaults filled in
var msg = config.toMessage(config.withDefaults({ SLOT_TL: '4', SCHEME: 3, UNITS: 1 }));
assert.deepStrictEqual(msg, { SLOT_TL: 4, SLOT_TR: 2, SLOT_BL: 3, SLOT_BR: 4, CENTER_T: 14, CENTER_L: 10, CENTER_R: 12, CENTER_B: 6,
  SCHEME: 3, UNITS: 1, STEP_GOAL: 10000, SECONDS: 0, BG_COLOR: 0xC0, ACCENT_COLOR: 0xF8,
  TEXT_TL: '', TEXT_TR: '', TEXT_BL: '', TEXT_BR: '', TEXT_T: 'PB', TEXT_L: '', TEXT_R: '', TEXT_B: '' });
// Text: only glyphs the watch font has, 12 max
var texts = config.withDefaults({ TEXT_TL: 'Héllo <b>&"x" 12:30 and more', TEXT_B: 'Hello' });
assert.deepStrictEqual([texts.TEXT_TL, texts.TEXT_B], ['Hllo bx 12:3', 'Hell']);

// Custom API complications: cleaned on the way in, kept off the settings message
var apis = config.withDefaults({ APIS: [{ url: ' https://x.test/a ', freq: 0.2, type: '2', name: 'Powér!', text: '{{a}}', min: 5 }, null] }).APIS;
assert.deepStrictEqual(apis, [{ url: 'https://x.test/a', freq: 1, type: 2, name: 'Powr', text: '{{a}}', min: '5', max: '100' },
  { url: '', freq: 10, type: 0, name: '', text: '', min: '0', max: '100' }]);
assert.strictEqual(config.toMessage(config.withDefaults({ APIS: apis })).APIS, undefined);
assert.strictEqual(config.withDefaults({ APIS: new Array(20) }).APIS.length, config.API_MAX);

// Patterns filled from the response; bar and gauge place the first one between min and max
var buildMessage = require('../src/pkjs/api').buildMessage;
var data = { results: { data: { points: [{ value: 1 }, { value: 12.3456, hi: 20 }] } }, on: true };
assert.deepStrictEqual(buildMessage({ type: 2, name: 'KW', text: '{{ results.data.points[1].value }} kW', min: '10', max: '{{results.data.points[1].hi}}' }, 3, data),
  { API_INDEX: 3, API_TYPE: 2, API_NAME: 'KW', API_TEXT: '12.35 kW', API_PCT: 23, API_MIN: '10', API_MAX: '20' });
assert.strictEqual(buildMessage({ type: 0, name: '', text: 'CO2 {{nope.x}} {{on}} é', min: '0', max: '100' }, 0, data).API_TEXT, 'CO2 -- true ');
assert.strictEqual(buildMessage({ type: 1, name: '', text: '{{results.data.points[0].value}}', min: 'abc', max: '100' }, 0, data).API_PCT, -1, 'min not a number');
assert.strictEqual(buildMessage({ type: 1, name: '', text: '{{results.data.points[0].value}}', min: '-50000', max: '2' }, 0, data).API_MIN, '-50k');
assert.deepStrictEqual(buildMessage({ type: 1, name: 'X', text: '{{a}}', min: '0', max: '1' }, 1, null),
  { API_INDEX: 1, API_TYPE: 1, API_NAME: 'X', API_TEXT: '--', API_PCT: -1, API_MIN: '', API_MAX: '' }, 'no response');

execSync('node scripts/inline-config.mjs');
delete require.cache[require.resolve('../src/pkjs/page')];
var page = require('../src/pkjs/page');

function el() {
  return { innerHTML: '', textContent: '', value: '', attrs: {}, dataset: {}, style: {}, classList: { add: function () {} },
    children: [], setAttribute: function (k, v) { this.attrs[k] = String(v); }, addEventListener: function () {} };
}
var els, clicks;
var document = {
  getElementById: function (id) {
    if (!els[id]) { els[id] = el(); if (id === 'schemes' || id === 'units') els[id].children = [el(), el(), el(), el()].slice(0, id === 'units' ? 2 : 4); }
    return els[id];
  },
  addEventListener: function (type, fn) { if (type === 'click') clicks.push(fn); },
  querySelectorAll: function () { return []; }
};
// Runs the page with `state` written in, on a fresh stub DOM; returns its globals.
function load(state) {
  els = {};
  clicks = [];
  var ctx = { document: document, location: { search: '', href: '' }, URLSearchParams: URLSearchParams, setInterval: function () {}, Math: Math };
  vm.createContext(ctx);
  page.replace('var STATE = null; //$$STATE$$', 'var STATE = ' + JSON.stringify(state) + ';')
    .match(/<script>([\s\S]*?)<\/script>/g).forEach(function (s) { vm.runInContext(s.replace(/<\/?script>/g, ''), ctx); });
  return ctx;
}

// Curved labels are a <textPath> in a <text>: read here as one <text>.
function screen() { return els.screen.innerHTML.replace(/<text><textPath href="#curve\d+" startOffset="50%" text-anchor="middle"/g, '<text').replace(/<\/textPath>/g, ''); }
var ctx = load({ settings: { SLOT_TL: 1, SLOT_TR: 2, SLOT_BL: 3, SLOT_BR: 6, SCHEME: 1, SECONDS: 1 }, platform: 'gabbro' });
var svg = screen();
assert.ok(svg.indexOf('viewBox="0 0 260 260"') > 0, 'gabbro geometry');
assert.ok(svg.indexOf('fill="#ffffff"') > 0, 'white scheme background');
assert.ok(svg.indexOf('>6240</text>') > 0, 'steps label');
assert.ok(svg.indexOf('r="102"') > 0, 'gabbro dial');
assert.ok(/font-size="18.57"[^>]*>21°<\/text>/.test(svg), 'gabbro temp corner: just now, on the arc');
assert.ok(/font-size="15.71"[^>]*>82%<\/text>/.test(svg), 'gabbro battery: value leads the bar');
assert.ok(svg.indexOf('>PB</text>') > 0 && svg.indexOf('>290</text>') > 0 && svg.indexOf(ctx.ICONS.sun_cloud) > 0, 'default subdials: text, elevation, weather');
assert.ok(/>(SUN|MON|TUE|WED|THU|FRI|SAT)<\/text>/.test(svg), 'calendar weekday');
assert.ok(svg.indexOf('stroke="#ffaa00" stroke-width="2"') > 0, 'seconds hand');
assert.ok(els.schemes.innerHTML.indexOf('data-scheme="1" aria-pressed="true"') > 0);
assert.strictEqual(els.colors.hidden, true);
assert.strictEqual(els.seconds.attrs['aria-checked'], 'true');

// Save navigates with the settings as payload
var save = el(); save.id = 'save';
clicks[0]({ target: { closest: function () { return save; } } });
var sent = JSON.parse(decodeURIComponent(ctx.location.href.split('#')[1]));
assert.strictEqual(sent.SCHEME, 1);
assert.strictEqual(sent.SLOT_BR, 6);

// Accent scheme on a light background: black text, accent fills, bg for black
ctx = load({ settings: { SCHEME: 4, BG_COLOR: 0xFF, ACCENT_COLOR: 0xF0, SLOT_TL: 8, CENTER_T: 8, SLOT_TR: 14, CENTER_B: 14, CENTER_L: 14, TEXT_TR: 'Hello <i>', TEXT_B: 'Hello', TEXT_L: 'EC' }, platform: 'emery' });
svg = screen();
assert.ok(svg.indexOf('<rect width="200" height="228" fill="#ffffff"') > 0, 'custom background');
assert.ok(svg.indexOf('stroke="#ff0000"') > 0, 'accent fills');
assert.ok(svg.indexOf('fill="#000000" transform="rotate(0') > 0, 'numerals black on light bg');
assert.strictEqual(els.colors.hidden, false);
assert.ok(/font-size="14.29" fill="#000000"[^>]*>72 BPM<\/text>/.test(svg), 'heart corner: curved text, no bar');
assert.ok(svg.indexOf('>Hello i</text>') > 0 && svg.indexOf('>Hell</text>') > 0 && svg.indexOf('>EC</text>') > 0, 'text per place, cleaned');
assert.ok(els.corners.innerHTML.indexOf('data-text="TEXT_TR" data-slot-key="SLOT_TR" value="Hello i" maxlength="12"') > 0, 'corner text field');
assert.ok(svg.split(ctx.ICONS.heart).length - 1 === 2, 'heart icon in corner and subdial');

// New subdials and corners, imperial
ctx = load({ settings: { SLOT_TL: 13, SLOT_TR: 11, SLOT_BL: 7, SLOT_BR: 10, CENTER_T: 11, CENTER_L: 9, CENTER_R: 12, CENTER_B: 3, UNITS: 1 },
  weather: { TEMP: 70, TEMP_MIN: 60, TEMP_MAX: 75, RAIN: 10, AQI: 20, UV: 7, HUMIDITY: 55, CONDITION: 6, ELEVATION: 100, imperial: true }, platform: 'emery' });
svg = screen();
assert.ok(svg.indexOf('>55%</text>') > 0 && svg.indexOf('stroke="#00ffff" stroke-width="6"') > 0, 'humidity corner');
assert.ok(svg.indexOf('>MI</text>') > 0 && svg.indexOf('>2.4</text>') > 0, 'distance subdial');
assert.strictEqual(svg.split('>UV</text>').length - 1, 2, 'UV corner and subdial');
// Orange is UV's alone here; width 6 is the corner bar (subdial rings are 3).
assert.ok(svg.indexOf('stroke="#ff5500" stroke-width="6"') > 0, 'UV corner bar in its band color');
assert.ok(/font-size="14.29" fill="#ffffff"[^>]*>7<\/text>/.test(svg), 'UV corner value, curved, white');
assert.ok(svg.indexOf(ctx.ICONS.rain) > 0 && svg.indexOf('>FT</text>') < 0, 'weather subdial');
assert.ok(svg.indexOf('>82</text>') > 0, 'battery subdial');
assert.strictEqual(svg.split('>AQI</text>').length - 1, 1, 'AQI word in the corner, no AQI subdial');
assert.ok(svg.indexOf('>20</text>') > 0, 'AQI corner');
assert.ok(/font-size="14.29" fill="#ffffff"[^>]*>328ft<\/text>/.test(svg) && svg.indexOf('stroke="#ff0000" stroke-width="6"') < 0, 'elevation corner: curved text, no bar');
assert.ok(els.centers.innerHTML.indexOf('value="12" selected') > 0, 'center pickers');

// Custom API complications: listed in the pickers, drawn from the last message or a sample
ctx = load({ settings: { SLOT_TL: 15, SLOT_TR: 16, SLOT_BL: 17, CENTER_T: 17, CENTER_L: 16, CENTER_R: 15, APIS: [
  { url: 'https://x.test/"a"', type: 2, name: 'KW', text: '{{a}}', min: '0', max: '50' }, { type: 1, name: 'rain' }, { type: 0 }] },
  api: [{ API_TEXT: '12.3', API_PCT: 25, API_MIN: '0', API_MAX: '50' }], platform: 'emery' });
svg = screen();
assert.ok(svg.indexOf('>KW 12.3</text>') > 0 && svg.indexOf('>12.3</text>') > 0, 'gauge: last value, named in the corner, and subdial');
assert.strictEqual(svg.split('>50</text>').length - 1, 2, 'gauge: max label, corner and subdial');
assert.strictEqual(svg.split('>rain</text>').length - 1, 2, 'bar: its name for an icon, not the icon of that name');
assert.strictEqual(svg.split('>42</text>').length - 1, 4, 'bar and text: sample value');
assert.ok(els.corners.innerHTML.indexOf('<option value="15" selected>KW</option>') > 0 && els.centers.innerHTML.indexOf('<option value="17" selected>Custom 3</option>') > 0, 'API options');
assert.ok(els.apis.innerHTML.indexOf('<details class="card api" data-card="0"><summary>KW \u00B7 Gauge</summary>') === 0, 'saved cards start collapsed');
assert.ok(els.apis.innerHTML.indexOf('value="https://x.test/&quot;a&quot;"') > 0 && els.apis.innerHTML.split('data-api-remove').length - 1 === 3, 'API cards');
// Removing the second: its places empty, the third takes its number
var remove = el(); remove.dataset.apiRemove = '1';
clicks[0]({ target: { closest: function () { return remove; } } });
clicks[0]({ target: { closest: function () { return save; } } });
sent = JSON.parse(decodeURIComponent(ctx.location.href.split('#')[1]));
assert.deepStrictEqual([sent.SLOT_TL, sent.SLOT_TR, sent.SLOT_BL, sent.CENTER_L, sent.APIS.length], [15, 0, 16, 0, 2]);

var condition = require('../src/pkjs/weather').condition;
assert.deepStrictEqual([[0, true], [0, false], [2, false], [3, true], [48, true], [61, true], [81, true], [73, true], [86, true], [95, true], [7, true]]
  .map(function (a) { return condition(a[0], a[1]); }), [0, 1, 3, 4, 5, 6, 6, 7, 7, 8, -1], 'WMO codes');

// Sunrise and sunset: minutes after midnight on the wire, both times in the corner
var weather = require('../src/pkjs/weather').buildMessage({ current: {}, daily: { temperature_2m_min: [1], temperature_2m_max: [2],
  precipitation_probability_max: [3], sunrise: ['2026-10-02T06:30'], sunset: ['2026-10-02T19:55'] } }, null);
assert.deepStrictEqual([weather.SUNRISE, weather.SUNSET], [390, 1195]);
ctx = load({ settings: { SLOT_TL: 23, CENTER_T: 23 }, weather: weather, platform: 'emery' });
svg = screen();
assert.ok(svg.indexOf('>\u25B46:30 \u25BE19:55</text>') > 0 && /(up|down)/.test(Object.keys(ctx.ICONS).filter(function (k) { return svg.indexOf(ctx.ICONS[k]) > 0; }).join()), 'sun corner and subdial');
ctx = load({ settings: { SLOT_TL: 23 }, weather: { TEMP: 1 }, platform: 'emery' });
assert.ok(screen().indexOf('>--</text>') > 0, 'weather saved before 1.1 has no sun times');

// .beat time: ".042" and the @, corner and subdial
ctx = load({ settings: { SLOT_TL: 24, CENTER_T: 24 }, platform: 'emery' });
svg = screen();
assert.ok(svg.split('>@</text>').length - 1 === 2 && svg.split(/>\.\d{3}<\/text>/).length - 1 === 2, '.beat corner and subdial');

// Wind: km/h on the wire, the page converts; speed and direction in the corner, two rows in the subdial
weather = require('../src/pkjs/weather').buildMessage({ current: { wind_speed_10m: 13.6, wind_direction_10m: 338 }, daily: { temperature_2m_min: [1],
  temperature_2m_max: [2], precipitation_probability_max: [3] } }, null);
assert.deepStrictEqual([weather.WIND, weather.WIND_DIR], [14, 338]);
ctx = load({ settings: { SLOT_TL: 25, CENTER_T: 25 }, weather: weather, platform: 'emery' });
svg = screen();
assert.ok(svg.indexOf('>14km/h N</text>') > 0 && svg.indexOf('>14</text>') > 0 && svg.split(ctx.ICONS.wind).length - 1 === 2, 'wind corner and subdial');
ctx = load({ settings: { SLOT_TL: 25, UNITS: 1 }, weather: weather, platform: 'gabbro' });
assert.ok(screen().indexOf('>8mph N</text>') > 0, 'wind in mph');
ctx = load({ settings: { SLOT_TL: 25 }, weather: { TEMP: 1 }, platform: 'emery' });
assert.ok(screen().indexOf('>--</text>') > 0, 'weather saved before wind has none');

console.log('ok');
