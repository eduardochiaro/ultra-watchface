#!/usr/bin/env node
// Generate an SVG <font> from a TTF, suitable for pebble-fctx-compiler.
//
// SVG fonts use a y-up coordinate system with the baseline at y=0, whereas
// opentype.js emits y-down screen coordinates, so we negate y. Coordinates are
// kept in raw font units (em = unitsPerEm) which the fctx compiler rescales.
//
// Usage: node scripts/gen-svg-font.js <in.ttf> <out.svg> <font-id> [chars]
const fs = require('fs');
const opentype = require('opentype.js');

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
