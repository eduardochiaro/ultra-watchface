// Run: npm test. Runs the built settings page against a stub DOM and checks
// the preview draws and the message the watch gets.
var assert = require('assert');
var vm = require('vm');
var execSync = require('child_process').execSync;
var config = require('../src/pkjs/config');

// Message: ints, defaults filled in
var msg = config.toMessage(config.withDefaults({ SLOT_TL: '4', SCHEME: 3, UNITS: 1 }));
assert.deepStrictEqual(msg, { SLOT_TL: 4, SLOT_TR: 2, SLOT_BL: 3, SLOT_BR: 4, CENTER_T: 2, CENTER_L: 4, CENTER_R: 7, CENTER_B: 6,
  SCHEME: 3, UNITS: 1, STEP_GOAL: 10000, SECONDS: 0, BG_COLOR: 0xC0, ACCENT_COLOR: 0xF8 });

execSync('node scripts/inline-config.mjs');
delete require.cache[require.resolve('../src/pkjs/page')];
var page = require('../src/pkjs/page');
var state = { settings: { SLOT_TL: 1, SLOT_TR: 2, SLOT_BL: 3, SLOT_BR: 6, SCHEME: 1, SECONDS: 1 }, platform: 'gabbro' };
page = page.replace('var STATE = null; //$$STATE$$', 'var STATE = ' + JSON.stringify(state) + ';');

function el() {
  return { innerHTML: '', textContent: '', value: '', attrs: {}, dataset: {}, style: {}, classList: { add: function () {} },
    children: [], setAttribute: function (k, v) { this.attrs[k] = String(v); }, addEventListener: function () {} };
}
var els = {};
var clicks = [];
var document = {
  getElementById: function (id) {
    if (!els[id]) { els[id] = el(); if (id === 'schemes' || id === 'units') els[id].children = [el(), el(), el(), el()].slice(0, id === 'units' ? 2 : 4); }
    return els[id];
  },
  addEventListener: function (type, fn) { if (type === 'click') clicks.push(fn); },
  querySelectorAll: function () { return []; }
};
var ctx = { document: document, location: { search: '', href: '' }, URLSearchParams: URLSearchParams, setInterval: function () {}, Math: Math };
var scripts = page.match(/<script>([\s\S]*?)<\/script>/g).map(function (s) { return s.replace(/<\/?script>/g, ''); });
vm.createContext(ctx);
scripts.forEach(function (s) { vm.runInContext(s, ctx); });

var svg = els.screen.innerHTML;
assert.ok(svg.indexOf('viewBox="0 0 260 260"') > 0, 'gabbro geometry');
assert.ok(svg.indexOf('fill="#ffffff"') > 0, 'white scheme background');
assert.ok(svg.indexOf('>6240</text>') > 0, 'steps label');
assert.ok(svg.indexOf('>24</text>') > 0, 'temp max label');
assert.ok(svg.indexOf('>82%</text>') > 0, 'battery label');
assert.ok(svg.indexOf('>30</text>') > 0, 'rain subdial');
assert.ok(svg.indexOf('>42</text>') > 0 && svg.indexOf('>AQI</text>') > 0, 'AQI subdial');
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
state = { settings: { SCHEME: 4, BG_COLOR: 0xFF, ACCENT_COLOR: 0xF0 }, platform: 'emery' };
var page2 = require('../src/pkjs/page').replace('var STATE = null; //$$STATE$$', 'var STATE = ' + JSON.stringify(state) + ';');
els = {};
var ctx2 = { document: document, location: { search: '', href: '' }, URLSearchParams: URLSearchParams, setInterval: function () {}, Math: Math };
vm.createContext(ctx2);
page2.match(/<script>([\s\S]*?)<\/script>/g).forEach(function (s) { vm.runInContext(s.replace(/<\/?script>/g, ''), ctx2); });
svg = els.screen.innerHTML;
assert.ok(svg.indexOf('<rect width="200" height="228" fill="#ffffff"') > 0, 'custom background');
assert.ok(svg.indexOf('stroke="#ff0000"') > 0, 'accent fills');
assert.ok(svg.indexOf('fill="#000000" transform="rotate(0') > 0, 'numerals black on light bg');
assert.strictEqual(els.colors.hidden, false);

// New subdials and corners, imperial
state = { settings: { SLOT_TL: 8, SLOT_TR: 9, SLOT_BL: 7, SLOT_BR: 10, CENTER_T: 8, CENTER_L: 9, CENTER_R: 10, CENTER_B: 3, UNITS: 1 },
  weather: { TEMP: 70, TEMP_MIN: 60, TEMP_MAX: 75, RAIN: 10, AQI: 20, ELEVATION: 100, imperial: true }, platform: 'emery' };
var page3 = require('../src/pkjs/page').replace('var STATE = null; //$$STATE$$', 'var STATE = ' + JSON.stringify(state) + ';');
els = {};
var ctx3 = { document: document, location: { search: '', href: '' }, URLSearchParams: URLSearchParams, setInterval: function () {}, Math: Math };
vm.createContext(ctx3);
page3.match(/<script>([\s\S]*?)<\/script>/g).forEach(function (s) { vm.runInContext(s.replace(/<\/?script>/g, ''), ctx3); });
svg = els.screen.innerHTML;
assert.ok(svg.indexOf('>BPM</text>') > 0 && svg.indexOf('>72</text>') > 0, 'heart subdial');
assert.ok(svg.indexOf('>MI</text>') > 0 && svg.indexOf('>2.4</text>') > 0, 'distance subdial');
assert.ok(svg.indexOf('>2.4mi</text>') > 0, 'distance corner');
assert.ok(svg.indexOf('>FT</text>') > 0 && svg.indexOf('>328</text>') > 0, 'elevation subdial');
assert.ok(svg.indexOf('>82</text>') > 0, 'battery subdial');
assert.strictEqual(svg.split('>AQI</text>').length - 1, 1, 'AQI word in the corner, no AQI subdial');
assert.ok(svg.indexOf('>20</text>') > 0 && svg.indexOf('>328ft</text>') > 0, 'AQI and elevation corners');
assert.ok(els.centers.innerHTML.indexOf('value="10" selected') > 0, 'center pickers');

console.log('ok');
