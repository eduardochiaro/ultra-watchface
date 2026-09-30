// Run: npm test. Runs the built settings page against a stub DOM and checks
// the preview draws and the message the watch gets.
var assert = require('assert');
var vm = require('vm');
var execSync = require('child_process').execSync;
var config = require('../src/pkjs/config');

// Message: ints, and UNITS stays on the phone
var msg = config.toMessage(config.withDefaults({ SLOT_TL: '4', SCHEME: 3, UNITS: 1 }));
assert.deepStrictEqual(msg, { SLOT_TL: 4, SLOT_TR: 2, SLOT_BL: 3, SLOT_BR: 4, SCHEME: 3, STEP_GOAL: 10000, SECONDS: 0 });

execSync('node scripts/inline-config.mjs');
delete require.cache[require.resolve('../src/pkjs/page')];
var page = require('../src/pkjs/page');
var state = { settings: { SLOT_TL: 1, SLOT_TR: 2, SLOT_BL: 3, SLOT_BR: 0, SCHEME: 1, SECONDS: 1 }, platform: 'gabbro' };
page = page.replace('var STATE = null; //$$STATE$$', 'var STATE = ' + JSON.stringify(state) + ';');

function el() {
  return { innerHTML: '', textContent: '', value: '', attrs: {}, dataset: {}, classList: { add: function () {} },
    children: [], setAttribute: function (k, v) { this.attrs[k] = String(v); }, addEventListener: function () {} };
}
var els = {};
var clicks = [];
var document = {
  getElementById: function (id) {
    if (!els[id]) { els[id] = el(); if (id === 'schemes' || id === 'units') els[id].children = [el(), el(), el(), el()].slice(0, id === 'units' ? 2 : 4); }
    return els[id];
  },
  addEventListener: function (type, fn) { if (type === 'click') clicks.push(fn); }
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
assert.ok(svg.indexOf('>30%</text>') < 0, 'rain slot is None');
assert.ok(svg.indexOf('stroke="#ffaa00" stroke-width="2"') > 0, 'seconds hand');
assert.strictEqual(els.schemes.children[1].attrs['aria-pressed'], 'true');
assert.strictEqual(els.seconds.attrs['aria-checked'], 'true');

// Save navigates with the settings as payload
var save = el(); save.id = 'save';
clicks[0]({ target: { closest: function () { return save; } } });
var sent = JSON.parse(decodeURIComponent(ctx.location.href.split('#')[1]));
assert.strictEqual(sent.SCHEME, 1);
assert.strictEqual(sent.SLOT_BR, 0);

console.log('ok');
