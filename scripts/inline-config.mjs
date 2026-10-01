// The settings page travels inside the app: pkjs hands the phone the whole page
// as a data: URL rather than a link to one. This inlines src/pkjs/config.js
// into src/pkjs/config.html and turns the result into src/pkjs/page.js, the
// module index.js requires. Run by every build script. The icons in
// resources/icons ride along as ICONS { name: path } for the preview.
import { readFileSync, readdirSync, writeFileSync } from "node:fs";

const read = (name) => readFileSync(new URL(`../src/pkjs/${name}`, import.meta.url), "utf8");
const tag = '<script src="config.js"></script>';
const html = read("config.html");
if (!html.includes(tag)) throw new Error(`config.html lost its ${tag}`);

const iconDir = new URL("../resources/icons/", import.meta.url);
const icons = Object.fromEntries(readdirSync(iconDir).filter((f) => f.endsWith(".svg")).map((f) =>
	[f.slice(0, -4), readFileSync(new URL(f, iconDir), "utf8").match(/ d="([^"]+)"/)[1]]));

// Through a function: config.js may hold "$" sequences a string replacement would read
const page = html.replace(tag, () => `<script>\nvar ICONS = ${JSON.stringify(icons)};\n${read("config.js")}</script>`);
writeFileSync(new URL("../src/pkjs/page.js", import.meta.url),
	`// Generated from config.html + config.js by scripts/inline-config.mjs. Do not edit.\nmodule.exports = ${JSON.stringify(page)};\n`);
