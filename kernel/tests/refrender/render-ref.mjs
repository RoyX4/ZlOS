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
 *   node render-ref.mjs --batch                    # desktop + one shot per app
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

/* A fixed clock and a seeded PRNG, so two runs of the same shot are
 * comparable. The reference drives its CPU graph, frame timings and
 * kernel-log chatter off Math.random() and Date.now(). Re-injected on every
 * page load, so each shot in a batch starts from the same seed. */
const DETERMINISM = `(() => {
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

function fail(msg) { console.error('render-ref: ' + msg); process.exit(2); }

function help() {
  console.log(`usage: node render-ref.mjs [options]

  --src PATH      reference HTML (default: ./ds.html, the vendored copy)
  --shot NAME     output basename under out/ (default: reference-1280x800, or app-<id>)
  --app ID        open only this app id, focused and centred
  --state JSON    patch merged into component state before the shot
  --batch         render the default desktop plus one shot per app id, in one browser
  --width N       viewport width  (default 1280 — the reference's own $preview)
  --height N      viewport height (default 800)
  --settle SEC    extra safety margin after animations settle (default 0.15)
  --live          do NOT freeze the clock / seed Math.random (shots stop being reproducible)
  --strict        exit non-zero if the page logged any error
  --full          full-page screenshot instead of viewport
  --quiet         only print the output path and hard failures
  --list-apps     print every app id the reference registers, one per line, and exit`);
}

function parseArgs(argv) {
  const o = { settle: 150, width: VW, height: VH };
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
    else if (a === '--batch') o.batch = true;
    else if (a === '--list-apps') o.listApps = true;
    else if (a === '-h' || a === '--help') { help(); process.exit(0); }
    else fail(`unknown argument ${a}`);
  }
  if (o.batch && (o.app || o.shot)) fail('--batch cannot be combined with --app or --shot');
  return o;
}

/* Turn a shot spec into one setState patch. `state` is the component's own
 * freshly-mounted state, so window geometry comes from the reference's APPS
 * table rather than being duplicated here. */
function buildPatch(spec, state, o) {
  const patch = { booting: false, dialog: null, toasts: [] };
  if (spec.app) {
    const pos = { ...state.pos };
    if (!pos[spec.app]) fail(`unknown app id ${JSON.stringify(spec.app)} — not present in state.pos`);
    const geo = { ...pos[spec.app] };
    geo.x = Math.max(0, Math.round((o.width - geo.w) / 2));
    geo.y = Math.max(TOP, Math.round(TOP + (o.height - TOP - DOCK - geo.h) / 2));
    pos[spec.app] = geo;
    Object.assign(patch, {
      open: { [spec.app]: true },
      min: {},
      focus: spec.app,
      pos,
      winWs: { ...state.winWs, [spec.app]: state.ws },
      zOf: { [spec.app]: 20 },
      z: 20,
      overview: false,
      showApps: false
    });
  }
  if (spec.state) {
    let extra;
    try { extra = JSON.parse(spec.state); } catch (e) { fail('--state is not valid JSON: ' + e.message); }
    Object.assign(patch, extra);
  }
  return patch;
}

/* One shot: fresh page load, patch state, wait, screenshot.
 * The page is reloaded per shot on purpose — it re-runs the init script, so
 * the PRNG and clock reset and shots do not depend on batch order. */
async function shoot(page, url, spec, o) {
  const consoleMsgs = [], pageErrors = [], failedReqs = [];
  const onConsole = m => consoleMsgs.push([m.type(), m.text()]);
  const onPageError = e => pageErrors.push(String(e && e.stack || e));
  const onFailed = r => failedReqs.push(`${r.url()} ${(r.failure() || {}).errorText || ''}`);
  page.on('console', onConsole);
  page.on('pageerror', onPageError);
  page.on('requestfailed', onFailed);
  try {
    await page.goto(url, { waitUntil: 'load', timeout: 60000 });

    // The shim sets data-dc-ready once React has mounted the template.
    try {
      await page.waitForSelector("html[data-dc-ready='1']", { timeout: 30000 });
    } catch {
      const why = await page.getAttribute('html', 'data-dc-failed');
      fail('the shim never mounted' + (why ? ': ' + why : ' (no data-dc-failed set either)'));
    }

    const fonts = await page.evaluate(async () => {
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

    if (o.listApps) return { apps: Object.keys(state.pos) };

    await page.evaluate(p => window.__DC__.setState(p), buildPatch(spec, state, o));

    /* Wait on a condition, never on a wall clock: this box runs these shots
     * next to other work, and a fixed sleep would make the output depend on
     * host load. Finite entry animations (zwin .16s, zpop .1s, zov .16s) are
     * waited out; the infinite ones (zpulse on unfocused dock dots) can never
     * finish, so they are pinned to their first keyframe instead — which also
     * makes them reproducible rather than sampled at a random phase. */
    await page.evaluate(async () => {
      const infinite = a => {
        try { return a.effect.getComputedTiming().iterations === Infinity; } catch { return false; }
      };
      // A counter, not Date.now(): the clock is frozen by the init script.
      for (let round = 0; round < 60; round++) {
        const running = document.getAnimations().filter(a => a.playState === 'running' && !infinite(a));
        if (!running.length) break;
        await Promise.race([
          Promise.all(running.map(a => a.finished.catch(() => {}))),
          new Promise(r => setTimeout(r, 100))
        ]);
      }
      for (const a of document.getAnimations()) if (infinite(a)) { a.currentTime = 0; a.pause(); }
      await new Promise(r => requestAnimationFrame(() => requestAnimationFrame(r)));
    });
    if (o.settle) await page.waitForTimeout(o.settle);

    const shim = await page.evaluate(() => ({
      errors: window.__DC__.errors.slice(),
      warnings: window.__DC__.warnings.slice(),
      nodes: window.__DC__.templateNodes,
      domNodes: document.querySelectorAll('#dc-root *').length,
      // Nothing should reach the live DOM still holding an uninterpolated
      // mustache; if it does, a template feature was silently skipped.
      leaked: (document.getElementById('dc-root').innerHTML.match(/\{\{/g) || []).length
    }));

    const png = path.join(OUT, spec.name + '.png');
    await page.screenshot({ path: png, fullPage: !!o.full });
    return { png, fonts, shim, consoleMsgs, pageErrors, failedReqs };
  } finally {
    page.off('console', onConsole);
    page.off('pageerror', onPageError);
    page.off('requestfailed', onFailed);
  }
}

function report(r, o) {
  console.log('wrote ' + r.png);
  if (!o.quiet) {
    console.log(`template nodes compiled: ${r.shim.nodes}, live DOM nodes: ${r.shim.domNodes}, uninterpolated {{ left: ${r.shim.leaked}`);
    for (const w of r.shim.warnings.slice(0, 40)) console.log('shim warning: ' + w);
    for (const e of r.shim.errors.slice(0, 40)) console.log('shim error: ' + e.split('\n')[0]);
    for (const e of r.pageErrors.slice(0, 40)) console.log('page error: ' + e.split('\n')[0]);
    for (const [t, m] of r.consoleMsgs) if (t === 'error' || t === 'warning') console.log(`console.${t}: ${m.slice(0, 300)}`);
    for (const f of r.failedReqs) console.log('request failed: ' + f);
  }
}

/* --------------------------------------------------------------------- */

const o = parseArgs(process.argv.slice(2));
const src = path.resolve(o.src || path.join(HERE, 'ds.html'));
if (!fs.existsSync(src)) fail('no such file: ' + src);
if (!fs.existsSync(CHROME)) fail('no chromium at ' + CHROME + ' (set CHROMIUM_PATH)');
await fsp.mkdir(OUT, { recursive: true });

const [server, port] = await serve(path.dirname(src));
const url = `http://127.0.0.1:${port}/${path.basename(src)}`;

let status = 0;
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

  if (o.listApps) {
    const { apps } = await shoot(page, url, { name: 'probe' }, o);
    console.log(apps.join('\n'));
  } else {
    const specs = [];
    if (o.batch) {
      const { apps } = await shoot(page, url, { name: 'probe' }, { ...o, listApps: true });
      specs.push({ name: `reference-${o.width}x${o.height}` });
      for (const id of apps) specs.push({ name: 'app-' + id, app: id });
    } else {
      specs.push({
        name: o.shot || (o.app ? 'app-' + o.app : `reference-${o.width}x${o.height}`),
        app: o.app,
        state: o.state
      });
    }

    let badFonts = 0, errored = 0;
    for (const spec of specs) {
      const r = await shoot(page, url, spec, o);
      if (o.batch && o.quiet) console.log('wrote ' + r.png);
      else report(r, o);

      if (!(r.fonts.publicSans && r.fonts.robotoMono)) {
        badFonts++;
        console.error(`\n*** WEB FONTS DID NOT LOAD for ${spec.name} ***`);
        console.error(`Public Sans: ${r.fonts.publicSans}   Roboto Mono: ${r.fonts.robotoMono}`);
        console.error('loaded families: ' + JSON.stringify(r.fonts.families));
        console.error('Text metrics differ from the reference. Do NOT diff against this PNG.');
      } else if (!o.quiet) {
        console.log('fonts ok: ' + r.fonts.families.sort().join(', '));
      }
      if (r.shim.errors.length || r.pageErrors.length) errored++;
      if (r.shim.leaked) {
        console.error(`*** ${spec.name}: ${r.shim.leaked} uninterpolated {{ }} reached the DOM ***`);
        errored++;
      }
    }
    if (badFonts) status = 3;
    else if (o.strict && errored) status = 4;
    if (o.batch) console.log(`\n${specs.length} shots, ${badFonts} with missing fonts, ${errored} with errors`);
  }
} finally {
  await browser.close();
  server.close();
}
process.exit(status);
