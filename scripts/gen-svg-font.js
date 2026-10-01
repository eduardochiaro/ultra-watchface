#!/usr/bin/env node
// Generate an SVG <font> from a TTF, suitable for pebble-fctx-compiler.
//
// SVG fonts use a y-up coordinate system with the baseline at y=0, whereas
// opentype.js emits y-down screen coordinates, so we negate y. Coordinates are
// kept in raw font units (em = unitsPerEm) which the fctx compiler rescales.
//
// Icons from resources/icons/<name>.svg (24x24, one path) are added at U+E000
// on in ICONS order: their box scaled to the cap height, so an icon drawn at
// cap height N is N px square. Keep ICONS in sync with ICON_* in src/c/draw.h.
//
// Usage: node scripts/gen-svg-font.js <in.ttf> <out.svg> <font-id> [chars]
const fs = require('fs');
const path = require('path');
const opentype = require('opentype.js');
const parsePath = require('svg-path-parser');

const ICONS = ['heart', 'runner', 'bolt', 'umbrella', 'arrow'];

const [, , inPath, outPath, fontId, charsArg] = process.argv;
if (!inPath || !outPath || !fontId) {
  console.error('usage: gen-svg-font.js <in.ttf> <out.svg> <font-id> [chars]');
  process.exit(1);
}

// Default glyph set: digits + upper/lowercase letters (covers day/month/DoW
// names and the AM/PM marker). A space glyph is included for advance widths.
const chars = charsArg ||
  ' 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz';

// Per-glyph advance overrides (in font units), e.g. { '.': 250 } to tighten a
// glyph's cell. Nunito is proportional, so none are needed.
const ADV_OVERRIDE = {};

const font = opentype.parse(fs.readFileSync(inPath));
const em = font.unitsPerEm;
const os2 = font.tables.os2 || {};
const capHeight = os2.sCapHeight || Math.round(em * 0.7);

function pathData(glyph, dx = 0) {
  // getPath at fontSize == em keeps coordinates in font units (scale 1). dx
  // shifts every point horizontally (used to recenter a narrowed glyph).
  const p = glyph.getPath(0, 0, em);
  let d = '';
  for (const c of p.commands) {
    const ny = (v) => -Math.round(v);
    const nx = (v) => Math.round(v) + dx;
    switch (c.type) {
      case 'M': d += `M${nx(c.x)} ${ny(c.y)}`; break;
      case 'L': d += `L${nx(c.x)} ${ny(c.y)}`; break;
      case 'C': d += `C${nx(c.x1)} ${ny(c.y1)} ${nx(c.x2)} ${ny(c.y2)} ${nx(c.x)} ${ny(c.y)}`; break;
      case 'Q': d += `Q${nx(c.x1)} ${ny(c.y1)} ${nx(c.x)} ${ny(c.y)}`; break;
      case 'Z': d += 'Z'; break;
    }
  }
  return d;
}

function esc(s) {
  return s.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;')
          .replace(/"/g, '&quot;');
}

const glyphs = [];
for (const ch of chars) {
  const g = font.charToGlyph(ch);
  if (!g || g.index === 0) continue;            // skip .notdef
  const adv0 = Math.round(g.advanceWidth || em / 2);
  const adv = ADV_OVERRIDE[ch] != null ? ADV_OVERRIDE[ch] : adv0;
  // Recenter the ink in the resized cell so a narrowed glyph stays centered
  // (mono glyphs are drawn centered on adv/2).
  const dx = Math.round((adv - adv0) / 2);
  const d = pathData(g, dx);
  glyphs.push(`    <glyph unicode="${esc(ch)}" horiz-adv-x="${adv}" d="${d}"/>`);
}

// 24-unit y-down icon path -> font units, y-up, box on the baseline. The y flip
// mirrors arcs, so their rotation and sweep flip too.
function iconPath(d) {
  const s = capHeight / 24;
  const X = (v) => Math.round(v * s), Y = (v) => Math.round((24 - v) * s);
  const at = { x: 0, y: 0 }, start = { x: 0, y: 0 };
  return parsePath(d).map((c) => {
    if (c.relative) {  // to absolute, against the pen
      for (const k of ['x', 'x1', 'x2']) if (k in c) c[k] += at.x;
      for (const k of ['y', 'y1', 'y2']) if (k in c) c[k] += at.y;
    }
    c.code = c.code.toUpperCase();
    if (c.code === 'Z') Object.assign(at, start);
    else Object.assign(at, { x: 'x' in c ? c.x : at.x, y: 'y' in c ? c.y : at.y });
    if (c.code === 'M') Object.assign(start, at);
    switch (c.code) {
      case 'M': case 'L': case 'T': return `${c.code}${X(c.x)} ${Y(c.y)}`;
      case 'H': return `H${X(c.x)}`;
      case 'V': return `V${Y(c.y)}`;
      case 'C': return `C${X(c.x1)} ${Y(c.y1)} ${X(c.x2)} ${Y(c.y2)} ${X(c.x)} ${Y(c.y)}`;
      case 'S': return `S${X(c.x2)} ${Y(c.y2)} ${X(c.x)} ${Y(c.y)}`;
      case 'Q': return `Q${X(c.x1)} ${Y(c.y1)} ${X(c.x)} ${Y(c.y)}`;
      case 'A': return `A${Math.round(c.rx * s)} ${Math.round(c.ry * s)} ${-c.xAxisRotation} ${c.largeArc ? 1 : 0} ${c.sweep ? 0 : 1} ${X(c.x)} ${Y(c.y)}`;
      case 'Z': return 'Z';
    }
    throw new Error(`unsupported path command ${c.code}`);
  }).join('');
}

const iconDir = path.join(__dirname, '../resources/icons');
ICONS.forEach((name, i) => {
  const d = fs.readFileSync(path.join(iconDir, `${name}.svg`), 'utf8').match(/ d="([^"]+)"/)[1];
  glyphs.push(`    <glyph unicode="&#x${(0xE000 + i).toString(16)};" glyph-name="${name}" horiz-adv-x="${capHeight}" d="${iconPath(d)}"/>`);
});

const svg = `<?xml version="1.0" standalone="no"?>
<svg xmlns="http://www.w3.org/2000/svg">
<defs>
  <font id="${fontId}" horiz-adv-x="${Math.round(em / 2)}">
    <font-face units-per-em="${em}" ascent="${font.ascender}" descent="${font.descender}" cap-height="${capHeight}"/>
${glyphs.join('\n')}
  </font>
</defs>
</svg>
`;

fs.writeFileSync(outPath, svg);
console.log(`wrote ${outPath}: ${glyphs.length} glyphs, em=${em}, cap=${capHeight}`);
