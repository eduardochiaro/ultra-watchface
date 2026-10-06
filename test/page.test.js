// Run: npm test. Runs the built settings page against a stub DOM and checks
// the preview draws and the message the watch gets.
var assert = require('assert');
var vm = require('vm');
var execSync = require('child_process').execSync;
var config = require('../src/pkjs/config');

// Message: ints, defaults filled in
var msg = config.toMessage(config.withDefaults({ SLOT_TL: '4', SCHEME: 3, UNITS: 1 }));
assert.deepStrictEqual(msg, { SLOT_TL: 4, SLOT_TR: 2, SLOT_BL: 3, SLOT_BR: 4, CENTER_T: 14, CENTER_L: 10, CENTER_R: 12, CENTER_B: 6,
  SCHEME: 3, UNITS: 1, STEP_GOAL: 10000, SECONDS: 0, SWEEP: 0, SHAKE_HIDE: 0, BG_COLOR: 0xC0, ACCENT_COLOR: 0xF8, HANDS: 0, RING: 0, BAND_COLOR: 0, HAND_COLOR: 0, MINUTE_COLOR: 0, SECOND_COLOR: 0,
  ZONE_TL: 0, ZONE_TR: 0, ZONE_BL: 0, ZONE_BR: 0, ZONE_T: 0, ZONE_L: 0, ZONE_R: 0, ZONE_B: 0,
  TEXT_TL: '', TEXT_TR: '', TEXT_BL: '', TEXT_BR: '', TEXT_T: 'PB', TEXT_L: '', TEXT_R: '', TEXT_B: '' });
// Time zone: the watch gets the offset, and the name as the text of a place showing it; an unknown zone is UTC
msg = config.toMessage(config.withDefaults({ SLOT_TL: 32, ZONE_TL: 'PST', TEXT_TL: 'mine', CENTER_T: 32, ZONE_T: 'CST-CN', CENTER_B: 0, ZONE_B: 'IST', ZONE_L: 'nope' }));
assert.deepStrictEqual([msg.ZONE_TL, msg.TEXT_TL, msg.ZONE_T, msg.TEXT_T, msg.ZONE_B, msg.TEXT_B, msg.ZONE_L], [-480, 'PST', 480, 'CST', 330, '', 0]);
assert.ok(config.ZONES.every(function (z) { return z.name.length <= 4 && config.zone(z.value) === z; }), 'zone names fit a subdial, values are unique');
// Text: only glyphs the watch font has, 12 max
var texts = config.withDefaults({ TEXT_TL: 'Héllo <b>&"x" 12:30 and more', TEXT_B: 'Hello' });
assert.deepStrictEqual([texts.TEXT_TL, texts.TEXT_B], ['Hllo bx 12:3', 'Hell']);

// Custom API complications: cleaned on the way in, kept off the settings message
var apis = config.withDefaults({ APIS: [{ url: ' https://x.test/a ', freq: 0.2, type: '2', title: ' Solar power ', header: '{{u}}', text: '{{a}}', min: 5 }, null, { name: 'KW' }] }).APIS;
assert.deepStrictEqual(apis.slice(0, 2), [{ url: 'https://x.test/a', freq: 1, type: 2, title: 'Solar power', header: '{{u}}', text: '{{a}}', min: '5', max: '100' },
  { url: '', freq: 10, type: 0, title: '', header: '', text: '', min: '0', max: '100' }]);
assert.deepStrictEqual([apis[2].title, apis[2].header, apis[2].name], ['KW', 'KW', undefined], 'the old name is both');
assert.strictEqual(config.toMessage(config.withDefaults({ APIS: apis })).APIS, undefined);
assert.strictEqual(config.withDefaults({ APIS: new Array(20) }).APIS.length, config.API_MAX);

// Patterns filled from the response; bar and gauge place the first one between min and max
var buildMessage = require('../src/pkjs/api').buildMessage;
var data = { results: { data: { points: [{ value: 1 }, { value: 12.3456, hi: 20 }] } }, on: true };
assert.deepStrictEqual(buildMessage({ type: 2, header: 'KW', text: '{{ results.data.points[1].value }} kW', min: '10', max: '{{results.data.points[1].hi}}' }, 3, data),
  { API_INDEX: 3, API_TYPE: 2, API_NAME: 'KW', API_TEXT: '12.35 kW', API_PCT: 23, API_MIN: '10', API_MAX: '20' });
assert.strictEqual(buildMessage({ type: 0, header: '', text: 'CO2 {{nope.x}} {{on}} é', min: '0', max: '100' }, 0, data).API_TEXT, 'CO2 -- true ');
assert.strictEqual(buildMessage({ type: 1, header: '', text: '{{results.data.points[0].value}}', min: 'abc', max: '100' }, 0, data).API_PCT, -1, 'min not a number');
assert.strictEqual(buildMessage({ type: 1, header: '', text: '{{results.data.points[0].value}}', min: '-50000', max: '2' }, 0, data).API_MIN, '-50k');
assert.deepStrictEqual(buildMessage({ type: 1, header: 'X', text: '{{a}}', min: '0', max: '1' }, 1, null),
  { API_INDEX: 1, API_TYPE: 1, API_NAME: 'X', API_TEXT: '--', API_PCT: -1, API_MIN: '', API_MAX: '' }, 'no response');
var long = buildMessage({ type: 0, header: '{{unit}}!', text: 'Living room {{results.data.points[1].value}} ppm' }, 0, { unit: 'kWh/d', results: data.results });
assert.deepStrictEqual([long.API_NAME, long.API_TEXT], ['kWh/', 'Living room 12.35 pp'], 'header pattern, 4 characters; text 20');
assert.strictEqual(buildMessage({ type: 0, header: '{{unit}}', text: '' }, 0, null).API_NAME, '--', 'header pattern, no response');

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
    if (!els[id]) { els[id] = el(); if (id === 'schemes' || id === 'units') {
      els[id].children = [el(), el(), el(), el()].slice(0, id === 'units' ? 2 : 4);
    }
    }
    return els[id];
  },
  addEventListener: function (type, fn) { if (type === 'click') {
    clicks.push(fn);
  }
  },
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
assert.ok(/font-size="15.71"[^>]*>21°<\/text>/.test(svg) && svg.split(ctx.ICONS.sun_cloud).length - 1 === 2, 'gabbro temp corner: the conditions icon and now, no gauge');
assert.ok(/font-size="15.71"[^>]*>82<tspan font-size="75%">%<\/tspan><\/text>/.test(svg), 'gabbro battery: value leads the bar, its % the small one');
assert.ok(svg.indexOf('>PB</text>') > 0 && svg.indexOf('>290</text>') > 0 && svg.indexOf(ctx.ICONS.sun_cloud) > 0, 'default subdials: text, elevation, weather');
assert.ok(/>(SUN|MON|TUE|WED|THU|FRI|SAT)<\/text>/.test(svg), 'calendar weekday');
assert.ok(svg.indexOf('stroke="#ffaa00" stroke-width="2"') > 0, 'seconds hand');
assert.ok(els.schemes.innerHTML.indexOf('data-scheme="1" aria-pressed="true"') > 0);
assert.strictEqual(els.colors.hidden, true);
assert.strictEqual(els.seconds.attrs['aria-checked'], 'true');
assert.ok(els.freq.value === 30 && els.freq.innerHTML.indexOf('<option value="180">3 hours') > 0 && els['place-now'].textContent === "The phone's location" && els['place-results'].innerHTML === '', 'weather: every 30 minutes, at the phone');
assert.ok(els['sweep-row'].hidden === false && els.sweep.attrs['aria-checked'] === 'false', 'sweep: offered with the seconds hand, off');

// Save navigates with the settings as payload
var save = el(); save.id = 'save';
clicks[0]({ target: { closest: function () { return save; } } });
var sent = JSON.parse(decodeURIComponent(ctx.location.href.split('#')[1]));
assert.strictEqual(sent.SCHEME, 1);
assert.strictEqual(sent.SLOT_BR, 6);

// Hands: outline style in picked colors, fixed in any scheme; picking a scheme resets the colors
ctx = load({ settings: { SCHEME: 1, HANDS: 2, HAND_COLOR: 0xF0, MINUTE_COLOR: 0xC3, SECOND_COLOR: 0xCC, SECONDS: 1 }, platform: 'emery' });
svg = screen();
assert.ok(svg.indexOf('fill="#ffffff" stroke="#ff0000" stroke-width="2"/>') > 0 && svg.indexOf('fill="#ffffff" stroke="#0000ff" stroke-width="2"/>') > 0, 'outline hour and minute hands, a color each, the background inside');
assert.ok(svg.indexOf('stroke="#00ff00" stroke-width="2"') > 0 && svg.indexOf('r="5" fill="#00ff00"') > 0, 'seconds hand and pin');
assert.strictEqual(svg.split('stroke="#00ff00" stroke-width="3"').length - 1, 4, '12/3/6/9 notches in the seconds color');
assert.ok(els.hands.innerHTML.indexOf('data-hands="2" aria-pressed="true"') > 0);
var scheme = el(); scheme.dataset.scheme = '2';
clicks[0]({ target: { closest: function () { return scheme; } } });
assert.ok(screen().indexOf('fill="#000000" stroke="#ffffff" stroke-width="2"/>') > 0, 'scheme resets the hand colors, keeps the style');
clicks[0]({ target: { closest: function () { return save; } } });
sent = JSON.parse(decodeURIComponent(ctx.location.href.split('#')[1]));
assert.deepStrictEqual([sent.SCHEME, sent.HANDS, sent.HAND_COLOR, sent.MINUTE_COLOR, sent.SECOND_COLOR], [2, 2, 0, 0, 0]);
// Picking an accent takes the seconds color back
ctx = load({ settings: { SCHEME: 4, SECOND_COLOR: 0xCC, HAND_COLOR: 0xF0 }, platform: 'emery' });
var cell = el(); cell.dataset.argb = String(0xC3); cell.parentNode = { dataset: { color: 'ACCENT_COLOR' } };
clicks[0]({ target: { closest: function () { return cell; } } });
clicks[0]({ target: { closest: function () { return save; } } });
sent = JSON.parse(decodeURIComponent(ctx.location.href.split('#')[1]));
assert.deepStrictEqual([sent.ACCENT_COLOR, sent.SECOND_COLOR, sent.HAND_COLOR], [0xC3, 0, 0xF0]);

// Pointer and sword hands: a polygon each for hour and minute. Dauphine: two, one shaded
[3, 4, 5].forEach(function (style) {
  ctx = load({ settings: { HANDS: style, HAND_COLOR: 0xF0 }, platform: 'emery' });
  svg = screen();
  assert.ok(svg.split('<polygon').length - 1 === (style === 5 ? 4 : 2) && (style !== 5 || /<polygon[^>]*fill="#aa0000"/.test(svg)) && /<polygon[^>]*fill="#ff0000"/.test(svg), 'polygon hands, style ' + style);
  assert.ok(els.hands.innerHTML.split('data-hands=').length - 1 === 7, 'seven hand styles');
});
// None: no hour or minute hand, and no pin; the seconds hand stays if it's on, with its pin
load({ settings: { HANDS: 6, HAND_COLOR: 0xF3, MINUTE_COLOR: 0xC3 }, platform: 'emery' });
svg = screen();
assert.ok(svg.indexOf('#ff00ff') < 0 && svg.indexOf('#0000ff') < 0 && svg.indexOf('r="5"') < 0, 'no hands, no pin');
load({ settings: { HANDS: 6, SECONDS: 1, SECOND_COLOR: 0xCC }, platform: 'emery' });
svg = screen();
assert.ok(svg.indexOf('stroke="#00ff00" stroke-width="2"') > 0 && svg.indexOf('r="5" fill="#00ff00"') > 0, 'seconds hand and pin without the others');

// Rings: minimal has ticks only and the subdials grown into the larger center; sport a white band,
// 00..55 on it and the accent ring inside, a rim around the minute hand over it
ctx = load({ settings: { RING: 1 }, platform: 'emery' });
svg = screen();
assert.ok(svg.indexOf('>12</text>') < 0 && svg.indexOf('r="94"') < 0 && svg.indexOf('scale(1.59)') > 0, 'minimal: no numerals or circles, subdials 86/54 the size');
assert.strictEqual(svg.split('stroke="#ffffff" stroke-width="3"').length - 1, 13, 'minimal: 12 hour ticks, and the minute hand');
ctx = load({ settings: { RING: 2, HANDS: 3 }, platform: 'emery' });
svg = screen();
assert.ok(svg.indexOf('r="87" fill="none" stroke="#ffffff" stroke-width="22"') > 0 && svg.indexOf('r="74" fill="none" stroke="#ffaa00" stroke-width="4"') > 0, 'sport: band and inset ring');
assert.ok(svg.indexOf('>00</text>') > 0 && svg.indexOf('>55</text>') > 0 && svg.indexOf('>60</text>') < 0, 'sport numerals');
assert.strictEqual(svg.split('<polygon').length - 1, 6, 'sport: the minute hand and its 4 rim copies, the hour hand');
assert.ok(els.rings.innerHTML.indexOf('data-ring="2" aria-pressed="true"') > 0);
load({ settings: { RING: 2, SCHEME: 2, HANDS: 3 }, platform: 'emery' });
assert.strictEqual(screen().split('stroke="#000000" stroke-width="3"').length - 1, 12, 'mono: hour ticks black on the white band');
// Chronograph: the band alone, as wide as sport's with its ring, black on white, 1..12 upright
ctx = load({ settings: { RING: 3, HANDS: 3 }, platform: 'emery' });
svg = screen();
assert.ok(svg.indexOf('r="85" fill="none" stroke="#ffffff" stroke-width="26"') > 0 && svg.indexOf('stroke-width="4"') < 0 && svg.indexOf('scale(1.33)') > 0, 'chronograph: wider band, no inset ring, subdials 72/54 the size');
assert.ok(/y="32" font-size="11.43" fill="#000000" transform="rotate\(0 [^>]*>12<\/text>/.test(svg) && svg.indexOf('>00</text>') < 0, 'chronograph: hours, upright, midway between the ticks and the inner edge');
assert.strictEqual(svg.split('stroke="#000000" stroke-width="3"').length - 1, 12, 'chronograph: hour ticks black');
assert.ok(els.rings.innerHTML.split('data-ring=').length - 1 === 7 && els.rings.innerHTML.indexOf('data-ring="3" aria-pressed="true">Chronograph') > 0, 'seven rings');
// Roman: the default dial, I..XII turned along it, the bottom half the other way; no band color
ctx = load({ settings: { RING: 4 }, platform: 'emery' });
svg = screen();
assert.ok(/rotate\(360 [^>]*>XII<\/text>/.test(svg) && /rotate\(60 [^>]*>II<\/text>/.test(svg) && /rotate\(360 [^>]*>VI<\/text>/.test(svg) && svg.indexOf('>12</text>') < 0, 'roman numerals');
assert.ok(svg.indexOf('r="94"') > 0 && svg.indexOf('r="54"') > 0 && els['band-row'].hidden, 'roman: the default dial and inner circle, no band color');

// Tachymeter: 72 seconds and hour ticks, the accent band inside them, 10..60 and six dots on it; the band takes a color
ctx = load({ settings: { RING: 5, HANDS: 6 }, platform: 'emery' });
svg = screen();
assert.ok(svg.indexOf('r="80.5" fill="none" stroke="#ffaa00" stroke-width="17"') > 0 && svg.indexOf('scale(1.33)') > 0 && els['band-row'].hidden === false, 'tachymeter: accent band, the larger center');
assert.ok(/rotate\(360 [^>]*>60<\/text>/.test(svg) && /rotate\(360 [^>]*>30<\/text>/.test(svg) && svg.indexOf('>70</text>') < 0, 'tachymeter numerals');
assert.strictEqual(svg.split('r="2" fill="#000000"').length - 1, 6, 'tachymeter: a dot between the numerals');
assert.strictEqual(svg.split('stroke-width="3"').length - 1, 12, 'tachymeter: hour ticks');
ctx = load({ settings: { RING: 5, BAND_COLOR: 0xC3 }, platform: 'emery' });
assert.ok(screen().indexOf('stroke="#0000ff" stroke-width="17"') > 0 && /fill="#ffffff"[^>]*>60<\/text>/.test(screen()), 'tachymeter: blue band, white numerals');

// Compass: chronograph's band, 72 ticks, the 30s heavy, N E S W upright and 30..330 turned; the band takes a color
ctx = load({ settings: { RING: 6, HANDS: 3 }, platform: 'emery' });
svg = screen();
assert.ok(svg.indexOf('r="85" fill="none" stroke="#ffffff" stroke-width="26"') > 0 && svg.indexOf('scale(1.33)') > 0 && els['band-row'].hidden === false, 'compass: band, the larger center');
assert.ok(/rotate\(0 [^>]*>N<\/text>/.test(svg) && /rotate\(0 [^>]*>W<\/text>/.test(svg) && /rotate\(330 [^>]*>330<\/text>/.test(svg) && /rotate\(300 [^>]*>120<\/text>/.test(svg) && svg.indexOf('>360</text>') < 0, 'compass: cardinals upright, degrees turned');
assert.strictEqual(svg.split('stroke="#000000" stroke-width="3"').length - 1, 12, 'compass: 30 degree ticks');
assert.strictEqual(svg.split('stroke="#000000" stroke-width="1"').length - 1, 60, 'compass: 5 degree ticks');

// Band color: fixed in any scheme, its ticks and numerals white on a dark one; the picker shows for banded rings only
ctx = load({ settings: { RING: 2, BAND_COLOR: 0xF0, HANDS: 3 }, platform: 'emery' });
svg = screen();
assert.ok(svg.indexOf('stroke="#ff0000" stroke-width="22"') > 0 && /fill="#ffffff"[^>]*>00<\/text>/.test(svg) && svg.indexOf('stroke="#ffffff" stroke-width="1"') > 0, 'red band, white numerals and ticks');
assert.strictEqual(els['band-row'].hidden, false);
var white = el(); white.dataset.scheme = '1';
clicks[0]({ target: { closest: function () { return white; } } });
assert.ok(screen().indexOf('stroke="#000000" stroke-width="22"') > 0, 'a scheme takes the band color back');
load({ settings: { RING: 1, BAND_COLOR: 0xF0 }, platform: 'emery' });
assert.strictEqual(els['band-row'].hidden, true);
// Sport's accent picked the same as the band: hour ticks in the band's ink
load({ settings: { RING: 2, BAND_COLOR: 0xF0, SECOND_COLOR: 0xF0, HANDS: 3 }, platform: 'emery' });
assert.strictEqual(screen().split('stroke="#ffffff" stroke-width="3"').length - 1, 12, 'hour ticks white on a red band with a red accent');

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
// A subdial's % is the smaller one, a corner's the font's own
load({ settings: { SLOT_TL: 13, CENTER_T: 13 }, weather: { HUMIDITY: 55 }, platform: 'emery' });
assert.ok(screen().indexOf('>55<tspan font-size="75%">%</tspan></text>') > 0 && screen().indexOf('>55%</text>') > 0, 'small % in the subdial only');
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
  { url: 'https://x.test/"a"', type: 2, name: 'KW', text: '{{a}}', min: '0', max: '50' }, { type: 1, title: 'Rain <b>', header: 'rain' },
  { type: 0, header: '{{unit}}' }] },
  api: [{ API_TEXT: '12.3', API_PCT: 25, API_MIN: '0', API_MAX: '50' }, null, { API_NAME: 'ppm', API_TEXT: 'Living room 1234 ppm' }], platform: 'emery' });
svg = screen();
assert.ok(svg.indexOf('>KW 12.3</text>') > 0 && svg.indexOf('>12.3</text>') > 0, 'gauge: last value, named in the corner, and subdial');
assert.strictEqual(svg.split('>50</text>').length - 1, 2, 'gauge: max label, corner and subdial');
assert.strictEqual(svg.split('>rain</text>').length - 1, 2, 'bar: its name for an icon, not the icon of that name');
assert.strictEqual(svg.split('>42</text>').length - 1, 2, 'bar: sample value');
assert.strictEqual(svg.split('>ppm</text>').length - 1, 2, 'header pattern: as last sent');
var sizes = svg.split('font-size="').map(parseFloat);  // [i + 1]: of the text after split i
assert.ok(sizes[svg.split('>Living room 1234 ppm</text>')[0].split('font-size="').length - 1] < sizes[svg.split('>KW 12.3</text>')[0].split('font-size="').length - 1], 'a long corner text shrinks');
assert.ok(els.corners.innerHTML.indexOf('<option value="15" selected>KW</option>') > 0 && els.corners.innerHTML.indexOf('>Rain &lt;b></option>') > 0 && els.centers.innerHTML.indexOf('<option value="17" selected>Custom 3</option></optgroup>') > 0, 'API options, in their group');
assert.ok(els.apis.innerHTML.indexOf('<details class="card api" data-card="0"><summary>KW \u00B7 Gauge</summary>') === 0, 'saved cards start collapsed');
assert.ok(els.apis.innerHTML.indexOf('value="https://x.test/&quot;a&quot;"') > 0 && els.apis.innerHTML.split('data-api-remove').length - 1 === 3, 'API cards');
// A card's Save sends everything, like the page's
var cardSave = el(); cardSave.dataset.apiSave = '0';
cardSave.closest = function () { return { querySelectorAll: function () { return []; } }; };
clicks[0]({ target: { closest: function () { return cardSave; } } });
assert.strictEqual(JSON.parse(decodeURIComponent(ctx.location.href.split('#')[1])).APIS.length, 3, 'card Save closes with the settings');
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

// Wind: km/h on the wire, the page converts; speed and direction in the corner, direction, speed and unit in the subdial
weather = require('../src/pkjs/weather').buildMessage({ current: { wind_speed_10m: 13.6, wind_direction_10m: 338 }, daily: { temperature_2m_min: [1],
  temperature_2m_max: [2], precipitation_probability_max: [3] } }, null);
assert.deepStrictEqual([weather.WIND, weather.WIND_DIR], [14, 338]);
ctx = load({ settings: { SLOT_TL: 25, CENTER_T: 25 }, weather: weather, platform: 'emery' });
svg = screen();
assert.ok(svg.indexOf('>14km/h N</text>') > 0 && svg.indexOf('>14</text>') > 0 && svg.indexOf('>N</text>') > 0 && svg.indexOf('>km/h</text>') > 0, 'wind corner and subdial');
ctx = load({ settings: { SLOT_TL: 25, UNITS: 1 }, weather: weather, platform: 'gabbro' });
assert.ok(screen().indexOf('>8mph N</text>') > 0, 'wind in mph');
ctx = load({ settings: { SLOT_TL: 25 }, weather: { TEMP: 1 }, platform: 'emery' });
assert.ok(screen().indexOf('>--</text>') > 0, 'weather saved before wind has none');

// Location: the city in a corner, its short code in a subdial; coordinates and the country without a name
var place = require('../src/pkjs/weather').place;
assert.deepStrictEqual(place({ city: 'Seattle' }, 47.6, -122.3), { LOCATION: 'Seattle', LOCATION_CODE: 'SEA' });
assert.deepStrictEqual(place({ city: 'São Paulo' }, -23.5, -46.6), { LOCATION: 'Sao Paulo', LOCATION_CODE: 'SP' }, 'accents off, initials');
assert.deepStrictEqual(place({ city: '', locality: 'St. Louis' }, 38.6, -90.2), { LOCATION: 'St. Louis', LOCATION_CODE: 'SL' }, 'locality when no city');
assert.deepStrictEqual(place({ city: '東京', countryCode: 'JP' }, 35.68, 139.69), { LOCATION: '35.7N 139.7E', LOCATION_CODE: 'JP' }, 'nothing the font has');
assert.deepStrictEqual(place({ city: 'Gemeente Utrecht' }, 52.09, 5.12), { LOCATION: 'Utrecht', LOCATION_CODE: 'UTR' }, 'not the municipality');
assert.deepStrictEqual(place({ city: 'Stockholm Municipality' }, 59.33, 18.07).LOCATION, 'Stockholm');
assert.deepStrictEqual(place(null, -33.87, -70.6), { LOCATION: '33.9S 70.6W', LOCATION_CODE: '' });
load({ settings: { SLOT_TL: 36, CENTER_T: 36 }, weather: { TEMP: 1, LOCATION: 'Sao Paulo', LOCATION_CODE: 'SP' }, platform: 'emery' });
assert.ok(screen().indexOf('>Sao Paulo</text>') > 0 && screen().indexOf('>SP</text>') > 0, 'location in a corner and a subdial');
load({ settings: { SLOT_TL: 36, CENTER_T: 36, CENTER_L: 0, CENTER_R: 0, CENTER_B: 0, SLOT_TR: 0, SLOT_BL: 0, SLOT_BR: 0 }, weather: { TEMP: 1 }, platform: 'emery' });
assert.strictEqual(screen().split('>--</text>').length - 1, 2, 'weather saved before it had a place');

// Plain calendar, retired: a saved one becomes the Date as weekday and day, in red, no page behind it
assert.deepStrictEqual([config.withDefaults({ CENTER_T: 26 }).CENTER_T, config.withDefaults({ CENTER_T: 26 }).DATE_T, config.withDefaults({ DATE_T: 99 }).DATE_T], [6, 1, 0]);
ctx = load({ settings: { CENTER_T: 26, CENTER_B: 0 }, platform: 'emery' });
svg = screen();
assert.ok(/fill="#ff0000"[^>]*>(SUN|MON|TUE|WED|THU|FRI|SAT)<\/text>/.test(svg) && !/Z" fill="#ff0000"/.test(svg), 'plain calendar');

// Date: a format per place, sent as the place's ZONE_; a time zone's place keeps its offset
msg = config.toMessage(config.withDefaults({ SLOT_TL: 6, DATE_TL: 3, CENTER_T: 6, DATE_T: 5, SLOT_TR: 32, ZONE_TR: 'PST', DATE_TR: 2 }));
assert.deepStrictEqual([msg.ZONE_TL, msg.ZONE_T, msg.ZONE_TR, msg.ZONE_B, msg.DATE_TL], [3, 5, -480, 0, undefined]);
ctx = load({ settings: { SLOT_TL: 6, DATE_TL: 3, SLOT_TR: 6, DATE_TR: 4, SLOT_BL: 6, DATE_BL: 5, SLOT_BR: 6, DATE_BR: 6,
  CENTER_T: 6, DATE_T: 3, CENTER_L: 6, DATE_L: 7, CENTER_R: 6, DATE_R: 2, CENTER_B: 6, UNITS: 1 }, platform: 'emery' });
svg = screen();
var today = new Date(), wd = ['SUN', 'MON', 'TUE', 'WED', 'THU', 'FRI', 'SAT'][today.getDay()], month = ['JANUARY', 'FEBRUARY', 'MARCH', 'APRIL', 'MAY', 'JUNE', 'JULY', 'AUGUST', 'SEPTEMBER', 'OCTOBER', 'NOVEMBER', 'DECEMBER'][today.getMonth()], mo = month.slice(0, 3), md = today.getDate(), yr = today.getFullYear();
assert.ok(svg.indexOf('>' + md + ' ' + month + '</text>') > 0 && svg.split('>' + wd + '</text>').length - 1 === 3, 'full date in a corner: day and month on the arc, the weekday beside it (and in two subdials)');
assert.ok(svg.indexOf('>' + (today.getMonth() + 1) + '/' + md + '/' + yr + '</text>') > 0, 'numeric date, month first with imperial units');
assert.ok(/>WEEK \d+<\/text>/.test(svg) && />DAY \d+<\/text>/.test(svg), 'week number and day of year');
assert.ok(svg.indexOf('>' + mo + '</text>') > 0 && svg.indexOf('>' + yr + '</text>') > 0 && /Z" fill="#ff0000"/.test(svg), 'date subdials: parts stacked, the year, the page');
assert.ok(els.corners.innerHTML.indexOf('<select data-date="DATE_TL" data-slot-key="SLOT_TL"') > 0 && els.corners.innerHTML.indexOf('<option value="3" selected>Full date</option>') > 0, 'a date format picker per place');
// ISO weeks and days of the year
assert.deepStrictEqual([[2026, 9, 6], [2021, 0, 3], [2024, 11, 30], [2026, 11, 31], [2027, 0, 1], [2024, 11, 31]].map(function (d) {
  d = new Date(d[0], d[1], d[2]);
  return [config.isoWeek(d), config.yearDay(d) + 1];
}), [[41, 279], [53, 3], [1, 365], [53, 365], [53, 1], [1, 366]]);

// Week and year progress: corners and subdials
ctx = load({ settings: { SLOT_TL: 37, SLOT_TR: 38, CENTER_T: 37, CENTER_L: 38, SLOT_BL: 0, SLOT_BR: 0, CENTER_R: 0, CENTER_B: 0 }, platform: 'emery' });
svg = screen();
var lit = (today.getDay() + 6) % 7 + 1, gone = Math.trunc(config.yearDay(today) * 100 / config.yearDays(yr));
assert.strictEqual(svg.split('stroke="#ffaa00" stroke-width="6"').length - 1, lit + (gone > 0 ? 1 : 0), 'week corner: a section per day so far; the year bar');
assert.strictEqual(svg.split('>' + wd + '</text>').length - 1, 2, 'week: the weekday in a corner and a subdial');
assert.ok(svg.indexOf('>' + gone + '%</text>') > 0 && svg.split('>' + yr + '</text>').length - 1 === 2, 'year progress and the year');

// Active calories: its own complication, corner and subdial
load({ settings: { SLOT_TL: 30, CENTER_T: 30 }, platform: 'emery' });
assert.ok(screen().split('>310</text>').length - 1 === 2 && screen().indexOf('1480') < 0, 'active calories');

// Digital time, second time zone, sleep, active minutes, moon phase: corners and subdials
ctx = load({ settings: { SLOT_TL: 31, SLOT_TR: 32, SLOT_BL: 33, SLOT_BR: 34, CENTER_T: 31, CENTER_L: 32, CENTER_R: 33, CENTER_B: 34, ZONE_TR: 'IST', ZONE_L: 'PST' }, platform: 'emery' });
svg = screen();
assert.strictEqual(svg.split(/>\d\d:\d\d<\/text>/).length - 1, 3, 'local time in a corner and a subdial, the zone time in its subdial');
var zone = new Date(Date.now() + 330 * 60000).toISOString().slice(11, 16), pst = new Date(Date.now() - 480 * 60000).toISOString().slice(11, 16);
assert.ok(svg.indexOf('>IST ' + zone + '</text>') > 0 && svg.indexOf('>PST</text>') > 0 && svg.indexOf('>' + pst + '</text>') > 0, 'a zone per place: its name, then its time');
assert.ok(svg.indexOf('>ZZ</text>') > 0 && svg.split('>7h32</text>').length - 1 === 2 && svg.indexOf('>SLEEP</text>') > 0, 'sleep');
assert.ok(svg.indexOf('>48 MIN</text>') > 0 && svg.indexOf('>48</text>') > 0 && svg.indexOf('>ACTIVE</text>') > 0, 'active minutes');
assert.ok(els.corners.innerHTML.indexOf('<select data-zone="ZONE_TR" data-slot-key="SLOT_TR"') > 0 && els.corners.innerHTML.indexOf('<option value="IST" selected>IST \u00B7 India \u00B7 UTC+5:30</option>') > 0, 'a zone picker per place');
assert.ok(els.corners.innerHTML.indexOf('<optgroup label="Activity"><option value="1">Steps</option>') > 0 && els.corners.innerHTML.indexOf('</optgroup><option value="0">None</option>') > 0 &&
  els.corners.innerHTML.indexOf('<optgroup label="Americas"><option value="HST">') > 0, 'pickers in groups, None outside them');
ctx = load({ settings: { SLOT_TL: 35, CENTER_T: 35 }, platform: 'emery' });
svg = screen();
assert.ok(/>[\u25B4\u25BE]\d+%<\/text>/.test(svg) && svg.split(ctx.ICONS.moon).length - 1 === 1 && /A12 12 0 0 [01] 100 95A[\d.]+ 12 0 0 [01] 100 71Z" fill="#ffffff"/.test(svg), 'moon: lit share in the corner, its shape in the subdial');

console.log('ok');

// AQI and UV as range gauges, calories: corners and subdials
load({ settings: { SLOT_TL: 27, SLOT_TR: 28, SLOT_BL: 29, CENTER_T: 27, CENTER_L: 28, CENTER_R: 29 }, weather: { AQI: 75, UV: 6 }, platform: 'emery' });
svg = screen();
assert.ok(svg.indexOf('>AQI 75</text>') < 0 && svg.split('>1480</text>').length - 1 === 2 && svg.indexOf('stroke="#ff5500" stroke-width="6"') > 0, 'gauge corner: caption and value apart; calories corner and subdial, a bar of the step goal');
assert.ok(svg.indexOf('>500</text>') < 0 && svg.split('>AQI</text>').length - 1 === 2 && svg.split('>75</text>').length - 1 === 2, 'AQI gauge: no min and max, caption and value in corner and subdial');
load({ settings: { SLOT_TL: 27 }, weather: { AQI: 75 }, platform: 'gabbro' });
assert.ok(screen().indexOf('>AQI 75</text>') > 0 && screen().indexOf('stroke="#aa0000"') > 0, 'gabbro gauge corner: the value leads the arc');
assert.ok(svg.indexOf('>75</text>') > 0 && svg.indexOf('>KCAL</text>') > 0 && svg.split(ctx.ICONS.flame).length - 1 === 2, 'gauge and calories subdials');
assert.ok(svg.indexOf('stroke="#aa0000"') > 0 && svg.indexOf('>AQI</text>') > 0, 'every band color on the gauge, its caption in the subdial');

// getWeather asks only what the face shows: nothing at all without a weather complication, the AQI only for its own
function fetched(settings) {
  var urls = [];
  global.localStorage = { getItem: function (k) { return k === config.SETTINGS_KEY ? JSON.stringify(settings) : null; }, setItem: function () {} };
  // node has a navigator of its own, read-only
  Object.defineProperty(global, 'navigator', { configurable: true, value: { geolocation: { getCurrentPosition: function (ok) { urls.push('position'); ok({ coords: { latitude: 1, longitude: 2 } }); } } } });
  global.XMLHttpRequest = function () {
    this.open = function (m, url) { urls.push(url.split('/')[2]); };
    this.send = function () { this.status = 200; this.responseText = '{"current":{},"daily":{"temperature_2m_min":[1],"temperature_2m_max":[2],"precipitation_probability_max":[3]}}'; this.onload(); };
  };
  global.Pebble = { sendAppMessage: function () {} };
  require('../src/pkjs/weather')();
  return urls.join(' ');
}
// A picked place: its coordinates, no position asked, and its own name for the Location complication
var PICKED = { name: 'Rome', label: 'Rome, Lazio, Italy', lat: 41.9, lon: 12.5 };
assert.deepStrictEqual(config.withDefaults({ WEATHER_PLACE: PICKED, WEATHER_FREQ: '60' }), Object.assign(config.withDefaults(), { WEATHER_PLACE: PICKED, WEATHER_FREQ: 60 }));
assert.deepStrictEqual([config.withDefaults({ WEATHER_PLACE: { name: 'x', lat: 'no', lon: 1 } }).WEATHER_PLACE, config.withDefaults({ WEATHER_FREQ: 7 }).WEATHER_FREQ], [null, 30], 'a place without coordinates is the phone\'s; an unknown interval is 30');
var NO_WEATHER = { SLOT_TL: 1, SLOT_TR: 3, SLOT_BL: 6, SLOT_BR: 8, CENTER_T: 14, CENTER_L: 35, CENTER_R: 31, CENTER_B: 6 };
assert.strictEqual(fetched(NO_WEATHER), '', 'no weather complication: no position, no request');
assert.strictEqual(fetched(Object.assign({}, NO_WEATHER, { SLOT_TL: 2 })), 'position api.open-meteo.com', 'temperature: the forecast alone');
assert.strictEqual(fetched(Object.assign({}, NO_WEATHER, { CENTER_T: 27, CENTER_L: 36 })), 'position api.open-meteo.com air-quality-api.open-meteo.com api.bigdatacloud.net', 'AQI and location: theirs too');
assert.strictEqual(fetched(Object.assign({}, NO_WEATHER, { CENTER_T: 27, CENTER_L: 36, WEATHER_PLACE: PICKED })), 'api.open-meteo.com air-quality-api.open-meteo.com', 'picked place: no position, no reverse geocoding');
console.log('ok weather');
