#!/usr/bin/env node
/* Render ds.html — the zlOS desktop reference — to a PNG.
 *
 * ds.html is written for an `x-dc` template runtime that ships as ./support.js
 * and was never delivered. support.js in this directory is a reimplementation.
 * This script serves the directory over HTTP (so localStorage and dynamic
 * import behave like a real origin rather than file://), loads the reference at
 * exactly the size declared in its own $preview (1280x800), skips the boot
 * screen, waits for the web fonts, and screenshots.
 *
 *   node render-ref.mjs                            # default desktop
 *   node render-ref.mjs --app term                 # only Terminal, focused, centred
 *   node render-ref.mjs --state '{"locked":true}'  # arbitrary state patch
 *   node render-ref.mjs --shot mine --app files
 *
 * Exit status is non-zero if the web fonts did not load, or (with --strict) if
 * the page logged an error — either one silently changes text metrics and would
 * make every later pixel diff wrong.
 *
 * Why node and not python: the python3-playwright deb on this box has no node
 * driver (it looks for /usr/share/nodejs/playwright/cli.js, and the packaged
 * node-playwright is 1.38 against a 1.55 python binding). playwright-core from
 * npm, driving the system /usr/bin/chromium, is the working combination.
 */

import http from 'node:http';
import fs from 'node:fs';
import fsp from 'node:fs/promises';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { chromium } from 'playwright-core';

const HERE = path.dirname(fileURLToPath(import.meta.url));
const OUT = path.join(HERE, 'out');

// ds.html:974 — the reference's own $preview size.
const VW = 1280, VH = 800;
// ds.html:1214 — `const TOP = 48, DOCK = 62;`. Used only for centring.
const TOP = 48, DOCK = 62;

const FROZEN_EPOCH = 1755000000000; // fixed instant, so two runs match

const CHROME = process.env.CHROMIUM_PATH || '/usr/bin/chromium';

const MIME = {
  '.html': 'text/html; charset=utf-8',
  '.js': 'text/javascript; charset=utf-8',
  '.mjs': 'text/javascript; charset=utf-8',
  '.css': 'text/css; charset=utf-8',
  '.json': 'application/json; charset=utf-8',
  '.png': 'image/png', '.svg': 'image/svg+xml', '.woff2': 'font/woff2'
};

function serve(root) {
  const server = http.createServer((req, res) => {
    const rel = decodeURIComponent(new URL(req.url, 'http://x').pathname).replace(/^\/+/, '');
    const file = path.resolve(root, rel);
    if (!file.startsWith(path.resolve(root))) { res.writeHead(403).end(); return; }
    fs.readFile(file, (err, buf) => {
      if (err) { res.writeHead(404, { 'content-type': 'text/plain' }).end('not found'); return; }
      res.writeHead(200, { 'content-type': MIME[path.extname(file)] || 'application/octet-stream' });
      res.end(buf);
    });
  });
  return new Promise(r => server.listen(0, '127.0.0.1', () => r([server, server.address().port])));
}

const DETERMINISM = `(() => {
  // A fixed clock and a seeded PRNG, so two runs of the same shot are
  // comparable. The reference drives its CPU graph, frame timings and
  // kernel-log chatter off Math.random() and Date.now().
  const EPOCH = ${FROZEN_EPOCH};
  let s = 0x9e3779b9 >>> 0;
  Math.random = () => { s ^= s << 13; s >>>= 0; s ^= s >>> 17; s ^= s << 5; s >>>= 0; return s / 4294967296; };
  const R = Date;
  function D(...a) { if (!(this instanceof D)) return new R(EPOCH).toString(); return a.length ? new R(...a) : new R(EPOCH); }
  D.prototype = R.prototype;
  D.now = () => EPOCH; D.parse = R.parse; D.UTC = R.UTC;
  Object.setPrototypeOf(D, R);
  window.Date = D;
  performance.now = () => 0;
})();`;

function parseArgs(argv) {
  const o = { settle: 900, width: VW, height: VH };
  for (let i = 0; i < argv.length; i++) {
    const a = argv[i];
    const val = () => { const v = argv[++i]; if (v === undefined) fail(`${a} needs a value`); return v; };
    if (a === '--src') o.src = val();
    else if (a === '--shot') o.shot = val();
    else if (a === '--app') o.app = val();
    else if (a === '--state') o.state = val();
    else if (a === '--width') o.width = +val();
    else if (a === '--height') o.height = +val();
    else if (a === '--settle') o.settle = Math.round(parseFloat(val()) * 1000);
    else if (a === '--live') o.live = true;
    else if (a === '--strict') o.strict = true;
    else if (a === '--full') o.full = true;
    else if (a === '--quiet') o.quiet = true;
    else if (a === '-h' || a === '--help') { help(); process.exit(0); }
    else fail(`unknown argument ${a}`);
  }
  return o;
}

function help() {
  console.log(`usage: node render-ref.mjs [options]

  --src PATH      reference HTML (default: ./ds.html, the vendored copy)
  --shot NAME     output basename under out/ (default: reference-1280x800, or app-<id>)
  --app ID        open only this app id, focused and centred
  --state JSON    patch merged into component state before the shot
  --width N       viewport width  (default 1280 — the reference's own $preview)
  --height N      viewport height (default 800)
  --settle SEC    seconds to wait after fonts are ready (default 0.9)
  --live          do NOT freeze the clock / seed Math.random (shots stop being reproducible)
  --strict        exit non-zero if the page logged any error
  --full          full-page screenshot instead of viewport
  --quiet         only print the output path and hard failures`);
}

function fail(msg) { console.error('render-ref: ' + msg); process.exit(2); }

function buildPatch(o, state) {
  const patch = { booting: false, dialog: null, toasts: [] };
  if (o.app) {
    const pos = { ...state.pos };
    if (!pos[o.app]) fail(`unknown app id ${JSON.stringify(o.app)} — not present in state.pos`);
    const geo = { ...pos[o.app] };
    geo.x = Math.max(0, Math.round((o.width - geo.w) / 2));
    geo.y = Math.max(TOP, Math.round(TOP + (o.height - TOP - DOCK - geo.h) / 2));
    pos[o.app] = geo;
    Object.assign(patch, {
      open: { [o.app]: true },
      min: {},
      focus: o.app,
      pos,
      winWs: { ...state.winWs, [o.app]: state.ws },
      zOf: { [o.app]: 20 },
      z: 20,
      overview: false,
      showApps: false
    });
  }
  if (o.state) {
    let extra;
    try { extra = JSON.parse(o.state); } catch (e) { fail('--state is not valid JSON: ' + e.message); }
    Object.assign(patch, extra);
  }
  return patch;
}

const o = parseArgs(process.argv.slice(2));
const src = path.resolve(o.src || path.join(HERE, 'ds.html'));
if (!fs.existsSync(src)) fail('no such file: ' + src);
if (!fs.existsSync(CHROME)) fail('no chromium at ' + CHROME + ' (set CHROMIUM_PATH)');
const root = path.dirname(src);
const name = o.shot || (o.app ? 'app-' + o.app : `reference-${o.width}x${o.height}`);
await fsp.mkdir(OUT, { recursive: true });
const png = path.join(OUT, name + '.png');

const [server, port] = await serve(root);
const url = `http://127.0.0.1:${port}/${path.basename(src)}`;

const consoleMsgs = [], pageErrors = [], failedReqs = [];
let fonts, shim;

const browser = await chromium.launch({
  executablePath: CHROME,
  args: ['--font-render-hinting=none', '--disable-lcd-text', '--force-color-profile=srgb']
});
try {
  const ctx = await browser.newContext({
    viewport: { width: o.width, height: o.height },
    deviceScaleFactor: 1,
    colorScheme: 'dark'
  });
  if (!o.live) await ctx.addInitScript(DETERMINISM);
  const page = await ctx.newPage();
  page.on('console', m => consoleMsgs.push([m.type(), m.text()]));
  page.on('pageerror', e => pageErrors.push(String(e && e.stack || e)));
  page.on('requestfailed', r => failedReqs.push(`${r.url()} ${(r.failure() || {}).errorText || ''}`));

  await page.goto(url, { waitUntil: 'load', timeout: 60000 });

  // The shim sets data-dc-ready once React has mounted the template.
  await page.waitForSelector("html[data-dc-ready='1']", { timeout: 30000 }).catch(async () => {
    const why = await page.getAttribute('html', 'data-dc-failed');
    fail('the shim never mounted' + (why ? ': ' + why : ' (no data-dc-failed set either)'));
  });

  fonts = await page.evaluate(async () => {
    await document.fonts.ready;
    const fams = new Set();
    document.fonts.forEach(f => { if (f.status === 'loaded') fams.add(f.family); });
    return {
      families: [...fams],
      publicSans: document.fonts.check('700 15px "Public Sans"'),
      robotoMono: document.fonts.check('400 12px "Roboto Mono"')
    };
  });

  const state = await page.evaluate(() => JSON.parse(JSON.stringify({
    pos: window.__DC__.instance.state.pos,
    winWs: window.__DC__.instance.state.winWs,
    ws: window.__DC__.instance.state.ws
  })));

  await page.evaluate(p => window.__DC__.setState(p), buildPatch(o, state));

  // two frames, then a settle window for the CSS entry animations
  // (zwin 0.16s, zpop 0.1s) and any deferred canvas paint
  await page.evaluate(() => new Promise(r => requestAnimationFrame(() => requestAnimationFrame(r))));
  await page.waitForTimeout(o.settle);

  shim = await page.evaluate(() => ({
    errors: window.__DC__.errors.slice(),
    warnings: window.__DC__.warnings.slice(),
    nodes: window.__DC__.templateNodes,
    domNodes: document.querySelectorAll('#dc-root *').length
  }));

  await page.screenshot({ path: png, fullPage: !!o.full });
} finally {
  await browser.close();
  server.close();
}

console.log('wrote ' + png);
if (!o.quiet) {
  console.log(`template nodes compiled: ${shim.nodes}, live DOM nodes: ${shim.domNodes}`);
  for (const w of shim.warnings.slice(0, 40)) console.log('shim warning: ' + w);
  for (const e of shim.errors.slice(0, 40)) console.log('shim error: ' + e.split('\n')[0]);
  for (const e of pageErrors.slice(0, 40)) console.log('page error: ' + e.split('\n')[0]);
  for (const [t, m] of consoleMsgs) if (t === 'error' || t === 'warning') console.log(`console.${t}: ${m.slice(0, 300)}`);
  for (const f of failedReqs) console.log('request failed: ' + f);
}

const badFont = !(fonts.publicSans && fonts.robotoMono);
if (badFont) {
  console.error('\n*** WEB FONTS DID NOT LOAD ***');
  console.error(`Public Sans: ${fonts.publicSans}   Roboto Mono: ${fonts.robotoMono}`);
  console.error('loaded families: ' + JSON.stringify(fonts.families));
  console.error('Text metrics differ from the reference. Do NOT diff against this PNG.');
  process.exit(3);
}
if (!o.quiet) console.log('fonts ok: ' + fonts.families.sort().join(', '));
if (o.strict && (shim.errors.length || pageErrors.length)) process.exit(4);
