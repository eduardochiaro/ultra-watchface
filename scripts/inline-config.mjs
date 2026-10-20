// The settings page travels inside the app: pkjs hands the phone the whole page
// as a data: URL rather than a link to one. This inlines src/pkjs/config.js
// and src/pkjs/i18n.js into src/pkjs/config.html and turns the result into src/pkjs/page.js, the
// module index.js requires. Run by every build script. The icons in
// resources/icons ride along as ICONS { name: path } for the preview.
import { readFileSync, readdirSync, writeFileSync } from "node:fs";

const read = (name) => readFileSync(new URL(`../src/pkjs/${name}`, import.meta.url), "utf8");
const tag = '<script src="config.js"></script>', i18n = '<script src="i18n.js"></script>';
const html = read("config.html");
for (const t of [tag, i18n]) {
  if (!html.includes(t)) {
    throw new Error(`config.html lost its ${t}`);
  }
}

const iconDir = new URL("../resources/icons/", import.meta.url);
const icons = Object.fromEntries(readdirSync(iconDir).filter((f) => f.endsWith(".svg")).map((f) =>
	[f.slice(0, -4), readFileSync(new URL(f, iconDir), "utf8").match(/ d="([^"]+)"/)[1]]));

// Through a function: config.js may hold "$" sequences a string replacement would read
const page = html.replace(tag, () => `<script>\nvar ICONS = ${JSON.stringify(icons)};\n${read("config.js")}</script>`)
	.replace(i18n, () => `<script>\n${read("i18n.js")}</script>`);
writeFileSync(new URL("../src/pkjs/page.js", import.meta.url),
	`// Generated from config.html + config.js + i18n.js by scripts/inline-config.mjs. Do not edit.\nmodule.exports = ${JSON.stringify(page)};\n`);
