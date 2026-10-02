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

var ctx = load({ settings: { SLOT_TL: 1, SLOT_TR: 2, SLOT_BL: 3, SLOT_BR: 6, SCHEME: 1, SECONDS: 1 }, platform: 'gabbro' });
var svg = els.screen.innerHTML;
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
svg = els.screen.innerHTML;
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
svg = els.screen.innerHTML;
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

var condition = require('../src/pkjs/weather').condition;
assert.deepStrictEqual([[0, true], [0, false], [2, false], [3, true], [48, true], [61, true], [81, true], [73, true], [86, true], [95, true], [7, true]]
  .map(function (a) { return condition(a[0], a[1]); }), [0, 1, 3, 4, 5, 6, 6, 7, 7, 8, -1], 'WMO codes');

console.log('ok');
