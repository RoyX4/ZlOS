# BROWSER-NEXT-PROMPT — continue the browser, post-merge

Written 2026-08-19, for a fresh session. Read this, then
`kernel/docs/browser-status.md` (18 KB, the honest account of what was built and
what is defective), then `kernel/docs/BROWSER-PROMPT.md` (the original brief -
items 0..7, all of which are DONE).

## Where it actually is

**It works.** `desktop/browser` landed on `main` on 2026-08-19 as part of an
eleven-track merge. zlOS fetches `http://example.com/` **by name, off the real
internet**, and renders it. ~2,617 lines of new code against a ~3,050 budget.

Every piece is gated and every gate passes on `main` right now:

```
htmltest    101 checks, 0 failed      nettest    152 checks, 0 failed
browsertest  58 checks, 0 failed      httptest    91 checks, 0 failed
dnstest      97 checks, 0 failed      tcptest    110 checks, 0 failed
browsershot  renders a page to PPM
```

All eight sources - `browser.c html.c layout.c http.c tcp.c net.c dns.c
virtio_net.c` - are in `kernel/SOURCES`, so all four build targets compile them.
Verified, not assumed.

The app is `APP_BROWSER = 5` in `kernel.zl` (**renumbered during the merge** - it
was 3, which collided with `APP_SNAKE`; if any doc still says 3, it is stale).

---

## 1. Damage I did to it during the merge — fix this first

`desktop/browser` shipped ~124 lines in `fb.c` giving itself sized, bold and
oblique text: `fb_text_rich()`, `fb_text_rich_w()`, `fb_prop_em()`, over its own
`rich_adv()` and `blend_cov_shear()`.

**I deleted that implementation and replaced it with a 71-line shim onto
`fb_text_role()`.** The reason was sound - browser's version indexed
`font16x32_prop` with `PROP16_W == 30`, and this tree has neither (its atlas is
`prop16`/`prop16b`/`prop24`/`prop24b` with `PROP16_W == 16`), so it could not
have compiled. Browser synthesised bold by double-striking because, in its own
words, "there is one weight and one slope in the atlas"; this tree ships a real
bold. The API is unchanged and all 15 call sites in `browser.c` still work.

But it cost two things, and both are visible in a rendered page:

1. **Italic is gone.** `FBT_ITAL` renders REGULAR. `browser.c:51` maps
   `LS_ITALIC -> FBT_ITAL` and `layout.c` emits it, so `<em>` and `<i>` now look
   identical to their surrounding text. Browser's sheared oblique was a real
   feature and I removed it.
   The fix is to re-synthesise the shear on top of THIS atlas: `blend_cov`
   needs a source stride that is not the glyph width (a proportional glyph's ink
   is narrower than its cell) and a per-row horizontal offset. Browser's old
   `blend_cov_shear()` is the reference - `git show desktop/browser:kernel/fb.c`
   and find it. It is ~35 lines and the maths is already worked out.

2. **Sizes quantise to three steps.** `role_for_size()` snaps a requested pixel
   size to the nearest of `TEXT_CAPTION`/`TEXT_BODY`/`TEXT_TITLE`, so `h1`
   through `h6` land on three sizes rather than six. Browser's version scaled
   the 16x32 atlas continuously. Either generate more atlases, or scale one -
   `fb_text_scaled` already exists - but decide deliberately and say so in the
   comment.

Both are marked in `fb.c` under "HONEST LIMITS" around line 2885. When you fix
one, delete the corresponding sentence.

---

## 2. The correction the merge was supposed to trigger

`browser-status.md` says this explicitly, and it has not been done:

> `feature-catalogue.md` marks a web browser ❌ and `OVERNIGHT-PROMPT.md` says
> "Unbounded. Chromium is 6.38M lines, Ladybird 314K, all of zlOS is 11,374."
> **Every number there is correct and the conclusion does not follow.** ... Those
> two files live on other branches. **When this branch merges, both rows should
> be corrected.**

Both files are now on `main` - they were orphaned in `refs/wip` snapshots until
2026-08-19 and have been rescued. So the blocker is gone and the correction is
owed. `browser-status.md` §"The corrected row" already has the replacement text.

This matters beyond tidiness: the project has already had to publicly correct a
"95% achievable" that was really 20% (`DECISIONS.md` #26). The same standard
applies in the optimistic direction - a capability that was called impossible
and then built must have its row corrected, or the catalogue teaches the wrong
lesson about what "unbounded" means.

---

## 3. Known defects, already characterised — do not re-diagnose

`browser-status.md` has the full evidence for both. Summary only:

- **virtio-net receive: one stale descriptor.** The device reported the correct
  length for a frame that really arrived while leaving the buffer holding its
  previous contents. Measured: 45 inbound frames, 45 trace entries, exactly one
  mismatch, at index 32 descriptor 0. Clearing the buffer before re-posting
  turned the stale ARP frame into all-zeros, which is how "the device did not
  write it" became established rather than assumed. The driver now detects and
  drops it, and frame counts match the wire. **Understand why before changing
  it.**
- **An ICMP echo reply that the peer sent and the guest never saw.** Capture
  shows 42 requests, 42 replies, the lost one with 3 µs wire latency; the guest
  spun ~500 ms and gave up. Ring never overrun, no runts, identical at 16 and 32
  descriptors. Still open.

---

## 4. What to build next, in the order the original brief implies

Items 0-7 of `BROWSER-PROMPT.md` are done. What it left unfinished:

- **HTTPS** - §5 of the original brief. Read it before starting; the honest
  framing there is the point, not the code. `kernel/crypto.c` exists (21 KB, with
  `cryptotest.c`) but is **on no branch** - it is only in `refs/wip/*` snapshots.
  `git fetch origin 'refs/wip/*:refs/wip/*'` then look at
  `refs/wip/zl-linux:kernel/crypto.c`. Decide whether it lands before this work
  depends on it.
- **The URL bar, Back and history** are marked done and gated. Exercise them by
  hand in `./try.sh` before believing it - `probe-*.py` drives the pointer in
  single large jumps, which hides interaction bugs (see `POINTER-PROMPT.md`).
- More of HTML/CSS: `layout.c` is a box model plus inline flow at 402 lines.
  Whatever you add, the rule from the original brief holds - it must be gated by
  a host test with no kernel and no boot, the way `htmltest` and `layouttest`
  already are.

---

## 5. Rules

- **The pointer is currently broken** (`kernel/docs/POINTER-PROMPT.md`): two
  drainers on one xHCI event ring. If you are testing clicking on links, that is
  why it feels wrong, and it is not yours to fix unless you want it.
- Gates run backgrounded and sequentially, never beside an agent fan-out.
  `CLAUDE.md` has the measured OOM.
- `docs/MERGE-EVIDENCE.md` is the account of the merge, including the eight
  silent app-id collisions and the memory map that would have corrupted every AP
  stack. Read §2 before assuming a merge artefact is impossible.
- Write findings into the repo, not into a chat reply.
