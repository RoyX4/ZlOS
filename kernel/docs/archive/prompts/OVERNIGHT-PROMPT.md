> **AUDITED 2026-08-19 · MOSTLY DONE.** Checked item by item against `main` at `06ced13`,
> after the eleven-track merge. 44 of 58 asks are done and reachable, and both architectural inversions this prompt exists for landed. What is left is not unbuilt code but unrun gates — every harness named here is compiled by `land-gate.sh` and never executed. Two of its eight READ FIRST documents do not exist in the repo.
>
> **What is still open from this document is in
> [`docs/evidence/status-audits/STATE-OF-THE-PROJECT-2026-08-19.md`](../../../../docs/evidence/status-audits/STATE-OF-THE-PROJECT-2026-08-19.md) — read that first, and do not
> work from the task list below.**


# Overnight autonomous run — the prompt

## How to start it

**Don't paste the whole thing.** Open a fresh Claude Code session in
`~/Documents/repos/zl-linux` and paste only this:

```
Read kernel/docs/archive/prompts/OVERNIGHT-PROMPT.md in full and execute it exactly as written.
It is a complete unattended brief. I am asleep — never ask me anything, never
pause for confirmation, never stop to check in. If something is ambiguous, pick
the reading most consistent with the docs, log it to .ultra/TENSIONS.md as an
"ASSUMED:" line, and keep going. Work through groups A to H, then the anti-idle
backlog, and do not stop until I tell you to.
```

A short bootstrap that points at the file is **more reliable than pasting 500
lines** — the file stays in the repo, survives a context reset, and can be
re-read at any point to recover.

## The one thing a prompt cannot fix

**Permission prompts.** If the session stops to ask whether it may run a
command, no wording in here helps — it is waiting on the harness, not on
judgement. Start the session in a mode where the tools it needs are already
allowed, or it will stall on the first `gcc` and sit there until morning.

Being honest: no prompt *guarantees* an all-night run. What the text below does
is remove every excuse to stop, and keep enough state on disk that a restart
costs a minute instead of the night's work.

**Why this scope:** the modeset, the touchpad and anything on the ThinkPad need
Roy physically present. Everything below is testable in QEMU on the dev box
alone.

---

```
You are continuing zlOS unattended overnight. Roy is asleep. Work through the
whole list below without waiting for input. Do not stop when one item is done —
go to the next.

READ FIRST, IN THIS ORDER
  kernel/HANDOFF.md
  kernel/docs/DECISIONS.md          every decision already taken, and two that were wrong
  kernel/docs/archive/superseded/desktop-TODO.md  historical ordered task list
  kernel/docs/desktop-build-guide.md  what a compositor is, in plain words
  kernel/docs/archive/superseded/desktop-plan.md  architecture and line numbers
  kernel/docs/desktop-toolkit.md    the immediate-mode toolkit design
  kernel/docs/desktop-look.md       the three rendering bugs
  kernel/docs/desktop-polish-and-speed.md  measured baselines

Everything is already designed. Do not re-plan. Build it.

═══════════════════════════════════════════════════════════════════
DO NOT STOP. Read this twice.
═══════════════════════════════════════════════════════════════════

Roy is ASLEEP. There is nobody to answer you. A question you ask is a question
nobody hears, and the run is over. So:

NEVER ASK. NEVER WAIT. NEVER PAUSE FOR CONFIRMATION.

If something is ambiguous: pick the reading most consistent with the docs, write
the assumption into .ultra/TENSIONS.md as one line beginning "ASSUMED:", and
KEEP GOING. A logged assumption is recoverable. A stopped run is not.

THE ONLY FOUR REASONS YOU MAY STOP
  1. Every group A through H is done or explicitly blocked, AND the anti-idle
     backlog below is exhausted.
  2. You would have to break a HARD SAFETY RULE to continue.
  3. The repo is in a state where continuing would destroy someone else's work.
  4. Roy types something.
Nothing else. Not "this seems like a good place to check in." Not "I have
completed a milestone." Not "this might not be what you wanted."

EXCUSES YOU WILL BE TEMPTED BY, PRE-EMPTIVELY KILLED

  "I finished task X, I should report."
      -> Update the TODO checkbox and START THE NEXT TASK IN THE SAME BREATH.
         Do not write a summary between tasks. Summaries are for the very end.

  "This gate went red, I should stop and ask."
      -> No. Log it to .ultra/TENSIONS.md, revert if needed, MOVE TO THE NEXT
         INDEPENDENT TASK. Groups A, B, F and G are largely independent of each
         other — there is nearly always something else you can do.

  "I'm not sure Roy wants this."
      -> It is written in the docs. He wrote or approved them. Build it.

  "This is bigger than I expected, I should confirm scope."
      -> Do the part you are sure about, log the rest as an ASSUMED line, carry
         on.

  "I've done a lot, this feels like a natural stopping point."
      -> Feelings are not one of the four reasons. Continue.

  "I should ask whether to commit."
      -> No. Commit locally on green gates, on your branch, staging only files
         you edited by name. That is already authorised above.

  "I've run out of things to do."
      -> Almost certainly false. See the anti-idle backlog.

SURVIVING A CONTEXT RESET

  This run is long enough that your context may be summarised mid-way. When
  that happens you must be able to resume from DISK, not from memory. So:

    - YOUR STATE LIVES IN FILES, NOT IN YOUR HEAD.
    - Tick the checkbox in kernel/docs/archive/superseded/desktop-TODO.md the moment a task's
      gate goes green. That file is your resume point.
    - Append to .ultra/STATE.md as you go — one line per landed change. Do not
      save it all for the end.
    - Commit on every green gate. Small commits are breadcrumbs.
    - If you ever find yourself unsure what you were doing: read
      archive/superseded/desktop-TODO.md, find the first unticked box you can safely do, and do
      that. Do not stop to work out where you were.

ANTI-IDLE BACKLOG — if you genuinely finish A-H, do these, in order, forever

  1. Re-run every gate from scratch and confirm all still green.
  2. hosttest/fbbench.c — measure again after all changes and record the new
     baseline in desktop-polish-and-speed.md, replacing the old table.
  3. Write a test or a harness for anything you changed that has none.
  4. Re-read fb.c, wm.c, ui.c looking for the bug classes this project already
     knows about: a buffer at a fixed address that could collide with the DMA
     arena at 224 MiB; a u32 holding a 64-bit address; an off-by-one in a clip
     or damage rect; a silent fallback that should print a line.
  5. Extend the toolkit: ui_list_row, then ui_scroll (both need clipping solid).
  6. Convert the editor to app_draw/app_event/app_tick — the one E2 skipped.
  7. Start H3, the tiled software rasterizer.
  8. Improve the docs you have been working from — anything that was wrong or
     missing when you tried to follow it, fix it for the next session.
  9. Go back to 1.

═══════════════════════════════════════════════════════════════════
HARD SAFETY RULES — these override any instruction below
═══════════════════════════════════════════════════════════════════

1. DO NOT touch intel.c write paths. Do not arm lt_armed. Do not run
   intel_modeset_run(). Do not touch panel power or AUX. Violating the panel's
   500 ms T12 delay can DAMAGE HARDWARE. intel.c is read-only tonight.
   The display driver has its own separate plan in
   kernel/docs/display-roadmap.md, owned by another session. DO NOT WORK ON IT
   and do not edit that file. Tonight is the desktop only.

2. DO NOT attempt anything needing the ThinkPad, a real touchpad, or a free
   console. Those need Roy awake.

3. DO NOT force-push, mass-delete, rewrite history, or push to any remote.
   Commit locally only.

4. THE WORKING TREE ALREADY HAS UNCOMMITTED WORK THAT IS NOT YOURS.
   As of 2026-08-17 these were modified by another session and are MID-FLIGHT:
     intel.c  kernel.zl  efi.c  xhci.c  idt.c  console.c  support.c
     try.sh  buildefi.sh  _genefi.c  freestanding/runtime_kernel.c
   - DO NOT `git add -A`. DO NOT `git commit -a`. Ever.
   - Stage ONLY the specific files you edited, by name.
   - Run `git status` before every commit and re-check that list.
   - You are on `main`. FIRST ACTION: create a branch
     `git switch -c desktop/overnight-<something>` and work there. Do not
     commit to main.
   - If a file on the list above blocks you, work around it. Do not "fix" it.
   - If `verify.sh` is red at the START, that is someone else's in-progress
     work, NOT yours. Record the baseline honestly and continue with tasks
     that do not depend on it.

5. RESOURCES: 4 cores, 15 GB, this box has been OOM-killed twice.
   - `cut -d' ' -f1-3 /proc/loadavg` before anything heavy. If the 1-minute
     figure is above ~4, wait.
   - ONE QEMU at a time. Never QEMU plus a multi-agent fan-out.
   - Run gates in the background and collect them.

6. NEVER write a gate that waits a fixed wall-clock time. Poll for the expected
   output. A timing-sensitive gate already cost this project a false regression.

8. THE COMPOSITOR MUST BE OPTIONAL. verify.sh boots with `-kernel` and
   `-display none`, so px_w() == 0 and THERE IS NO FRAMEBUFFER on the gate
   path. The plain text shell must keep working and its serial transcript must
   stay byte-identical to golden.txt. Never make the text path depend on wm.c.
   See "BOOT AND INIT" below — this is the single most likely way to red the
   gate on your first commit.

7. If a gate goes red: STOP that task, log it to .ultra/TENSIONS.md with what
   failed and what you tried, and move to the next INDEPENDENT task. Never
   build on top of a red gate.

═══════════════════════════════════════════════════════════════════
HOW IT ALL FITS TOGETHER — read this before writing any code
═══════════════════════════════════════════════════════════════════

Build the parts wrong-way-round and they will each work while the whole thing
does not. This is the contract between them.

THE LAYERS

  kernel.zl    POLICY   theme values, which apps exist, what each app draws
      |                 calls ui_* only — no coordinates, no fb_* calls
  ui.c         WIDGETS  layout cursor, buttons, labels, the theme struct
      |                 calls fb_* — never touches windows or z-order
  wm.c         WINDOWS  window table, z-order, focus, damage, event routing
      |                 calls fb_clip + app_draw — never draws widgets itself
  fb.c         PIXELS   primitives, the clip rect, the damage list
      |
  input.c      EVENTS   one queue: keyboard + mouse + (later) touchpad

Each layer calls DOWN only. ui.c must not know what a window is. wm.c must not
know what a button is. If you find yourself needing an upward call, the layer
boundary is wrong — stop and reconsider rather than adding a back-channel.

THE FRAME LOOP — this replaces the shell's while-loop as the top of the system

  wm_frame():
    1. input_poll()               drain PS/2 + USB into the one event queue
    2. drain the queue -> wm_route(ev)     routing may mark damage
    3. for each OPEN window: app_tick()    an app marks itself damaged if its
                                           state changed (clock ticked, snake
                                           moved). It does NOT draw here.
    4. wm_repaint()               see below
    5. pointer sprite             fb_pointer_show at the new position
    6. fb_present()               one blit of the damaged region

THE REPAINT — where clipping actually does its work

  wm_repaint():
    for each damage rect R:
        fb_clip(R)
        draw the wallpaper inside R
        for i in zorder, BACK TO FRONT:
            w = wins[zorder[i]]
            if not overlaps(w.rect, R): continue
            fb_clip(intersect(R, w.frame))     <- clip to the window frame
            draw chrome: shadow, rrect, title bar, close box
            fb_clip(intersect(R, w.client))    <- NARROWER: client area only
            app_draw(w.app, w.client...)
        fb_clip_none()

  THE CLIP IS SET TWICE PER WINDOW, and that is the whole point:
    - once to the frame, so chrome cannot bleed onto neighbours
    - once NARROWER to the client area, so an app PHYSICALLY CANNOT draw over
      its own title bar or outside its window, no matter what it does
  An app that tries to draw at -500,-500 simply produces nothing. That is the
  guarantee clipping buys, and it is why fb_clip (B2) blocks everything else.

  Back-to-front is what makes overlap correct. zorder IS the paint order.

THE APP CONTRACT — three functions, no loop, ever

  app_draw (id, x, y, w, h, focus)   repaint my CLIENT AREA.
      - must be POSITION-PURE: every coordinate derived from the x,y,w,h passed
        in. No globals baked in. draw_sysmon(sx,sy,sfoc) already has this shape
        — copy it.
      - called only when the window is damaged, NOT every frame
      - also called in UI_HITTEST mode with drawing off (see D1)
  app_event(id, win, type, code, x, y) -> 1 if handled
  app_tick (id, win) -> 1 if it damaged itself
      - called every frame, cheap, must not draw

  An app NEVER owns a loop and NEVER calls fb_present. If you are writing
  `while (...)` inside an app you have made a mistake.

WINDOW LIFECYCLE — every operation is defined by what it damages

  open    claim a wins[] slot, append to zorder, focus it, damage its rect
  raise   move to the END of zorder, damage its rect
  focus   set focus; does NOT imply raise (a menu takes keys without reordering)
  move    damage OLD rect UNION NEW rect. No sprite, no snapshot, no grab.
  resize  same, plus app_draw at the new size. Apps must be size-agnostic.
  close   clear OPEN, remove from zorder, damage its rect, focus the new top

  There is no allocation anywhere. WM_MAX 12 is a hard ceiling and "no free
  slot" is a refusal with a message, never a silent drop.

EVENT ROUTING — three modes, checked in THIS order

  1. POINTER GRAB   a drag or a slider owns ALL pointer events until button-up,
                    regardless of where the pointer is. Without this, dragging
                    breaks the moment the pointer outruns the window. Today's
                    code only survives that because it is a bitmap stamp.
  2. MODAL          the start menu takes everything; a click outside dismisses
  3. NORMAL         pointer -> topmost window containing (x,y), walking zorder
                    BACKWARDS. keys -> the focus window.

BOOT AND INIT — how the machine ends up AT a desktop instead of a prompt

  This is the actual inversion. Today kernel.zl ends with:
      while running == 1 { ...prompt, read a key, run_command... }
  The shell IS the top of the system. After this work the COMPOSITOR is, and
  the shell is one window inside it.

  New sequence at the bottom of kernel.zl:

    kbd_init(); fs_init()
    if px_w() == 0:
        --- TEXT MODE. No compositor at all. Keep the OLD shell loop. ---
    else:
        layout()
        wm_init()
        w_shell = wm_open(APP_SHELL)     <- window 0, opened FIRST
        ...boot log prints into window 0's client area, as it does today...
        wm_open(APP_MONITOR)
        wm_open(APP_ABOUT)
        forever: wm_frame()              <- never returns

  ORDERING THAT MATTERS: the shell window must EXIST before the boot log
  prints, because the log goes inside it. Today draw_desk() draws the terminal
  frame and then the log fills it — same idea, just via wm_open now.

  HALT: 'q' currently returns 1 and falls out of the shell loop. It now has to
  ask the compositor to stop. Keep printing "halting" — verify.sh greps for it.

  DESKTOP FURNITURE IS NOT WINDOWS. The wallpaper, the dock and the header bar
  are drawn by wm_repaint in the wallpaper pass, before any window. They are
  always at the bottom, never overlapped, never in zorder. Only the START MENU
  becomes a real window (C5), because it must appear ON TOP of things.

  FRAME PACING: wm_frame() must not spin at 100% CPU. Gate it on ticks() or the
  new tsc() so it runs at a sane rate and idles otherwise.

*** THE CONSTRAINT THAT WILL BITE YOU FIRST ***

  verify.sh — the gate you must keep green — runs:
      qemu ... -kernel kernel.elf -serial stdio -display none -no-reboot
  `-kernel` plus `-display none` means QEMU's multiboot loader NEVER supplies a
  framebuffer tag. So on the gate path:

      px_w() == 0.  THERE IS NO FRAMEBUFFER. THERE IS NO DESKTOP.

  Therefore:
    - THE COMPOSITOR MUST BE OPTIONAL. wm.c, ui.c and every app must be
      unreachable when px_w() == 0, and the plain text shell must still run.
    - The serial transcript must stay BYTE-IDENTICAL to golden.txt. If your
      change prints one extra line on the text path, the gate goes red.
    - Test both paths. `./verify.sh` for text, `./try.sh` (which boots
      -cdrom zlOS.iso, so GRUB supplies the framebuffer tag) for graphics.
    - If you find yourself making the text path depend on wm.c, stop. That is
      the wrong direction and it will red the gate on your first commit.

  This is not a nice-to-have. It is the difference between a run that lands and
  a run that spends the night fighting its own gate.

WHERE IMMEDIATE MODE MEETS DAMAGE — the known tension, get this right

  Immediate-mode toolkits normally redraw every frame. That fights damage
  tracking directly. The resolution:
    - app_draw runs ONLY when the window is damaged
    - app_tick runs every frame and is where an app decides it IS damaged
    - a click runs app_draw a THIRD time, in UI_HITTEST mode, drawing nothing,
      purely to find which widget the pointer landed on
  This is flagged in desktop-toolkit.md as the thing most likely to be got
  wrong. If widgets flicker or clicks land on the wrong control, look here
  first.

═══════════════════════════════════════════════════════════════════
THE WORK QUEUE — in order. Each has a gate. Do not skip the gate.
═══════════════════════════════════════════════════════════════════

Baseline: `./verify.sh` was GREEN at 2026-08-17 handoff —
"ok    kernel boots, shell responds, transcript matches golden.txt"
— despite the other session's uncommitted changes. Re-run it yourself first to
confirm it is still green, then keep it green. If it is red before you have
touched anything, that is the other session's work; record it and continue with
tasks that do not depend on it.

--- Group A: the three rendering bugs (independent, low risk, do first) ---

A1. desktop-TODO 0e — fb_icon24 nearest-neighbour upscale (fb.c:929).
    Icons are drawn as geometry at 96x96, filtered to 24x24, then doubled by
    pixel-copying on every screen >=1400px wide. Regenerate the atlas at 48x48
    via gen_icons.py (it already draws at 4x), or interpolate instead of copy.
    GATE: kernel/hosttest/fbbench still builds and runs; boot try.sh and
    screendump the dock; icon edges must be smooth, not stepped.

A2. desktop-TODO 0f — anti-alias fb_line (fb.c:669). Plain Bresenham today.
    Wu's algorithm, ~20 lines. blend_rgb already exists.
    GATE: the System Monitor sparkline is smooth in a screendump.

A3. desktop-TODO 0g — the logo path. fb_glyph_scaled (fb.c:515) reads the 1-bit
    font8x16 and draws solid squares. text_big already does it correctly.
    GATE: screendump, logo is anti-aliased.

A4. Delete div255 (fb.c:198) — defined, never called. GCC already
    strength-reduces /255 in the blend path (verified by objdump).

--- Group B: the enabling changes (this is the real work) ---

B1. desktop-TODO 0a — the resolution cliff. fb.c:86 BACK_MAX is 1920*1200, so
    at 2560x1440 back_on becomes 0 and FOUR features die silently: back buffer,
    subpixel text, fast fb_get_px, and window dragging.
    - size the back buffer from the mode actually received in fb_setup()
    - PRINT A BOOT LINE whenever it degrades. The silence is the bug.
    - check the resulting top address against the DMA arena at 224 MiB. Base is
      currently 0x0C000000 (192 MiB). At 4K it would end at 223.6 MiB — 368 KiB
      of clearance. COMPUTE the base, do not hardcode it. This is the repo's
      recurring bug class.
    GATE: force 2560x1440 in QEMU; desktop draws, dragging works, boot log
    names the mode and says whether the back buffer is on.

B2. desktop-TODO 0b — fb_clip. THE KEYSTONE. Add clip_x0/y0/x1/y1 plus
    fb_clip() and fb_clip_none(). Change EXACTLY TWO functions to clamp against
    the scissor instead of the screen: fb_fill_px (fb.c:429-434) and put_pixel
    (fb.c:161). Everything else is built on those two. Do not touch other
    primitives.
    GATE: with the scissor at full screen, a QMP screendump must be
    PIXEL-IDENTICAL to before. Use the FNV-hash approach from
    hosttest/fbbench.c. If anything moved, a clamp is wrong.

B3. desktop-TODO 0c — the damage list. Replace the single dirty box (fb.c:89)
    with an 8-entry array, merge on overlap, and when full merge everything into
    one — which degrades to exactly today's behaviour, so the worst case is
    "as slow as now", never "wrong".
    GATE: screendump identical; presented area measurably smaller.

B4. desktop-TODO 0d — mouse into the input queue. input.c:40 already declares
    EV_MOUSE and NOTHING EVER PUSHES ONE. The mouse is a polled global at
    idt.c:100. Push EV_MOUSE from the same pump in input_poll(). Keep the old
    mouse_x()/mouse_y() builtins working during the transition.
    GATE: the existing mouse demo ('x') and paint ('d') still work.

B5. desktop-TODO 0h — add a tsc() builtin. cpu_tsc() and cpu_tsc_khz() exist
    (cpu.c:212, cpu.c:220) but are NOT exposed to zl. idt_ticks() is 100 Hz,
    too coarse for a 16 ms frame. Then put frame time on screen.
    GATE: frame time visible and plausible in try.sh.

--- Group C: the compositor (needs B2 and B3 done) ---

NOTE ON SCOPE: archive/superseded/desktop-plan.md says do not start the compositor until the
touchpad works. That rule was written believing a pointer UI was untestable.
It is NOT — QEMU has a working PS/2 mouse and try.sh GUI mode is verified. So
build and test the compositor in QEMU. Anything ThinkPad-specific still waits.

C1. wm.c — the window table. Fixed arrays, no heap:
      #define WM_MAX 12
      struct win { int x,y,w,h; int app; int flags; int min_w,min_h; char title[32]; };
      static struct win wins[WM_MAX];
      static int zorder[WM_MAX];   /* indices, BACK to FRONT */
    In C, not zl — the zl kernel subset has no lists (kernel/README.md:154,
    zl_list_n is a hard fault at freestanding/runtime_kernel.c:413).
    z-order IS the zorder array. Iteration order is paint order.

C2. The repaint loop:
      for each damage rect R: fb_clip(R); wallpaper; each window back-to-front
      that overlaps R; fb_clip_none()

C3. Routing: pointer grab -> modal -> normal. Focus separate from pointer-over.

C4. Delete the snapshot/sprite drag code (fb.c:775-834): fb_bg_snapshot,
    fb_bg_restore, fb_grab, fb_stamp, bg_buf, sp_buf. KEEP fb_pointer_show and
    fb_pointer_hide — the 11x17 cursor save-under is correct.
    This also removes the 640x480 drag ceiling (the terminal is 1256x944,
    nearly 4x over, which is the real reason it cannot be dragged).

--- Group D: the toolkit (needs B2 and C1) ---

D0. THEME AS DATA — do this FIRST, ui.c reads from it. Stolen from SerenityOS,
    where window frame colours and metrics live in a theme object rather than in
    the painting code. Right now zlOS hardcodes both.
      struct ui_theme {
          unsigned bg, panel, text, text_dim, accent, border, danger;
          int pad, gap, row_h, radius, title_h;
      };
    Move EVERY colour constant and EVERY spacing number out of kernel.zl and
    fb.c into this one struct. Spacing scale is 4/8/12/16/24 times ui() and
    NOTHING may use a value outside it. Today the code is full of hand-picked
    numbers — `sx + 14 * u`, `sy + 58 * u`, `214 * u`. All of those go.
    GATE: screendump before/after is near-identical (small shifts are expected
    and fine where a number snapped to the scale); grep shows no bare pixel
    literals left in the window/dock/menu drawing paths.

D1. ui.c per kernel/docs/desktop-toolkit.md. IMMEDIATE MODE, no allocation.
    Hit testing re-runs app_draw with drawing off — same trick as
    intel_modeset_dry(). CRITICAL: widgets must RETURN whether they fired and
    never take an action as an argument. C evaluates arguments eagerly; that is
    exactly why MS_STEP is a macro.
    Build order: ui_label -> ui_bar -> ui_button -> ui_sep/ui_space ->
    ui_toggle -> ui_slider -> ui_num. Stop there tonight.

D2. Port draw_sysmon and draw_about to ui_* calls. No coordinates left.

--- Group E: apps stop hogging the screen ---

E1. Convert snake_game first — its state is already in raw memory (SNAKE_X,
    SNAKE_Y), so there is least to hoist. app_draw / app_event / app_tick, and
    DELETE its while loop.
    GATE: snake keeps playing while another window is dragged.

E2. Then paint, cube_demo, anim, mousedemo. Leave the editor and
    windows_demo (windows_demo is superseded — consider deleting it).

E3. THE SHELL BECOMES APP 0 — desktop-TODO 2g. This is the actual inversion the
    whole project is for; do not skip it.
    - read_line() STOPS LOOPING. It becomes a state machine fed one char per
      app_event. Smaller than it sounds: LINE_BUF and the history are ALREADY
      in raw memory, so the state is already external.
    - run_command stops blocking. Commands that open an app call wm_open(id)
      and return. Commands that print are unchanged.
    - the terminal stops being special and joins the z-order like anything
      else. fb_set_text_box (fb.c:62) already confines it, and fb_scroll
      (fb.c:854) already only moves its own columns — that seam was built for
      this.
    GATE: the shell is draggable, and text still lands inside its frame when
    it is moved.

--- Group E2: the interaction model — "no more pressing Q" ---

This is what "done" feels like, in Roy's words: "no more pressing Q or whatever,
but a full one." Implement all of it:

E4. Closing is the close box or Ctrl+W. NEVER "press any key".
E5. Alt+Tab cycles focus — walk zorder backwards.
E6. Super opens the start menu. (Super is already tracked as a modifier,
    input.c:226, and used for nothing.)
E7. The dock launches INTO A WINDOW, not over the whole screen. dock_cmd()
    currently calls run_command which takes over; it should call wm_open().
    GATE: click a dock tile, an app opens in a window, the rest of the desktop
    is still visible and still live.

--- Group F: making it LOOK better (independent of C/D, safe unattended) ---

F1. ELEVATION. Every window currently gets the identical shadow —
    shadow(wx, wy, ww, wh, 8*u, 6*u), focused or not, window or menu. Real
    desktops encode a hierarchy: a menu sits above a window sits above the
    desktop. `off` and `soft` are ALREADY parameters of fb_shadow, so this costs
    nothing but deciding. Three levels. Unfocused windows get a shallower
    shadow than focused ones.
    GATE: screendump with two windows — the stack is visibly legible.

F2. THE CLOSE BOX. draw_window (kernel.zl:429) hardcodes a red box. Keep one
    button, but make it accent-coloured on hover and red only on press. One
    line, and it is the difference between "drawn" and "designed".
    Needs B4 (mouse events) for hover.

F3. PROPORTIONAL TEXT FOR UI LABELS — this is the one that stops the desktop
    reading as a terminal. fb_text_aa (fb.c:355) advances by exactly cell_w per
    character, so EVERY string on screen is monospace-positioned, including
    window titles and dock labels. Uniform advance is the single strongest
    "this is a terminal" signal.
    - emit per-glyph advance widths from gen_hd_font.py (FreeType already knows
      them) into a new table
    - add fb_text_prop() that advances by the real width
    - use it for window titles, dock labels, menu items — NOT the console,
      which keeps its grid
    GATE: window titles are proportionally spaced; the shell is untouched.

F4. FRACTIONAL UI SCALE. fb.c:143 is a binary switch —
    cell_w = (width >= 1400) ? GLYPH_W*2 : GLYPH_W. So ui() is 1 or 2 and
    nothing between. A 2560x1440 panel gets the same UI as 1920x1200, just
    bigger. Add 1.5x. LOWER PRIORITY — touches everything. Only attempt if
    Groups A-D are all green and committed.

--- Group G: making it FASTER ---

G1. SIMD THE BLEND LOOPS. SSE is enabled in the 64-bit build (HANDOFF.md) and
    cpu.c already detects SSE/SSE2/SSE3/SSSE3 — and NOTHING USES IT. fb.c's
    per-pixel blend_rgb/blend_sub are the obvious first customer. This also
    pays off later for the 3D rasterizer, so it is worth doing properly.
    GATE: hosttest/fbbench.c before and after, same run conditions, several
    runs. Report cycles per pixel. If it is not faster, SAY SO and revert —
    the tabled-shadow-divide attempt earlier today was 25% SLOWER and got
    reverted. See DECISIONS.md #25. Do not keep a change that does not measure.
    Correctness gate: the FNV back-buffer hash must be UNCHANGED, or close
    enough that any difference is explained and intended.

G2. DO NOT attempt SMP band rendering tonight. Three of four cores are parked
    in cli;hlt (smp.c:79) and splitting the back buffer across them is a real
    4x — but concurrency bugs in an unattended run are the wrong risk. Leave it
    for a supervised session. Note it in .ultra/STATE.md as the biggest
    remaining lever.

--- Group H: stretch, only if everything above is green ---

H1. ANIMATION. The single biggest "feels modern" gap — nothing in zlOS animates
    at all. Windows appear instantly, menus pop, focus snaps. Gated on B3
    (damage) because a fade means compositing several times, which is only
    affordable when it damages a rectangle instead of the screen.
    Start SMALL: 4 frames. A menu appearing over 4 frames already feels
    different. Do not build easing curves or a timeline system.

H2. TABBED WINDOWS (the Essence idea) — several apps in one window frame,
    grouped by task. Genuinely distinctive and cheap once wm.c exists.

H3. Software 3D: extend fb3d.c toward tiled rasterization (16x16 blocks,
    barycentric coordinates) per kernel/docs/os-landscape.md. This is the path
    SerenityOS used to run Quake III with no GPU. Large. Only start if
    everything else is done and committed.

C5. THE START MENU BECOMES A WINDOW with MODAL set. open_menu()
    (kernel.zl:1769-1773) already draws window chrome by hand — shadow + nested
    rrect + gradient header. Make that literal and delete its private
    grab/stamp save-restore path.

═══════════════════════════════════════════════════════════════════
HOW TO WORK
═══════════════════════════════════════════════════════════════════

- After each item: run `cd kernel && ./verify.sh` in the background and collect
  it. It must stay green. It takes ~1 min.
- Commit after each GREEN gate, locally, conventional commits
  (feat:/fix:/refactor:). Small commits. Never commit on a red gate.
- Use kernel/hosttest/fbbench.c to prove performance claims. Build it with
  kernel/hosttest/build.sh. NEVER report a performance number you did not
  measure — an instruction count is not a measurement. That mistake was already
  made once today; see DECISIONS.md #25.
- Update kernel/docs/archive/superseded/desktop-TODO.md checkboxes as you go.
- Append anything learned to .ultra/LEARNINGS.md and anything broken to
  .ultra/TENSIONS.md.
- Keep kernel/HANDOFF.md honest: record what is VERIFIED, not what is intended.

WHEN YOU FINISH, OR RUN OUT OF WORK YOU CAN SAFELY DO:
Write a summary at the top of .ultra/STATE.md — what landed, what gates passed
with their output, what you skipped and why, and the single highest-leverage
next move for Roy. Be honest about anything half-done.

DESIGN RULES — do not violate
- Extend the existing look, never introduce a second visual system.
- Focus is signalled by title-bar hue PLUS the accent underline. Both already
  exist. Do not add a third signal.
- No new fonts, no new corner radii. Nested 5px/4px rrect and TITLE_H 28 are
  the system.
- No generic flat/material desktop.
```

---

## What this deliberately excludes, and why

| Excluded | Because |
|---|---|
| Cold-start modeset | Needs `sudo systemctl stop lightdm` — only Roy, at the machine |
| Anything arming `lt_armed` | Can damage the panel |
| Touchpad / I2C-HID | Needs the ThinkPad; QEMU has no Intel LPSS I2C |
| Booting zlOS on the laptop | Needs Roy |
| Pushing to GitHub | Outward-facing, not for an unattended run |
| `ui_scroll`, `ui_list_row` | Need clipping to be settled first; leave for a supervised session |
| The editor conversion | Most state to hoist; highest chance of a mess |
| **SMP band rendering** | The biggest remaining speed lever (**4×**, three cores are parked) — but concurrency bugs unattended are the wrong risk. Supervised only. |
| Audio, networking, a filesystem | Each is a whole subsystem needing a heap. Not tonight, probably not ever — see `archive/superseded/feature-catalogue-2026-08-17.md` |
| A browser | Unbounded. Chromium is 6.38M lines, Ladybird 314K, all of zlOS is 11,374 |

> **Both of those last two rows were wrong, and are corrected here rather than
> edited away, because the reason they were wrong is the useful part.**
>
> Networking and a filesystem both shipped (`net.c`, `tcp.c`, `dns.c`,
> `http.c`, `fs.c`) and **none of them needed a heap** — they use the same
> static arenas as the rest of the kernel. "Needs a heap" was an assumption
> that was never tested against an attempt.
>
> The browser shipped too, on 2026-08-19: it fetches `http://example.com/` by
> name off the real internet and renders it, in ~4,657 lines. Every number in
> that row is correct and **the conclusion does not follow from them** — they
> measure the maximal version of the capability and report it as the
> capability. The full correction, and the general form of the mistake, is in
> `archive/superseded/feature-catalogue-2026-08-17.md` §"Why a browser is in a category of its own".
>
> Kept as a standing caution for this document's own genre: an exclusion table
> is a list of things nobody will attempt, so a wrong entry in it costs work
> that never gets done and never shows up missing.


## What Windows and Linux have that this run is chasing

Mapped so nothing quietly falls off the list. Full detail in
`archive/superseded/feature-catalogue-2026-08-17.md` and `desktop-polish-and-speed.md`.

| What they do | Task here |
|---|---|
| Redraw only the damaged region (~2% of screen, not 100%) | **B3** — the single biggest speed item |
| Clip drawing to a rectangle | **B2** — and it has three customers |
| A spacing scale, not hand-picked numbers | **D0** |
| Colours and metrics as data, not code | **D0** |
| A layout engine — nothing positioned by hand | **D1** |
| Elevation: menus above windows above desktop | **F1** |
| Hover and press states on controls | **F2** |
| Proportional text, not a monospace grid | **F3** |
| Fractional UI scaling (125%, 150%) | **F4** |
| SIMD in the inner loops | **G1** |
| Multiple cores drawing at once | **G2 — excluded, supervised only** |
| Things animate | **H1** |
| GPU does the compositing | **never** — and measured as unnecessary: zlOS is at 4.88 ms of a 16.67 ms budget |

## The one rule I relaxed, and I am flagging it

`archive/superseded/desktop-plan.md` says *do not start the compositor until the touchpad works*.
That was written believing a pointer UI could not be tested. **It can** — QEMU
has a working PS/2 mouse and `try.sh` GUI mode is verified working
(`HANDOFF.md`). The touchpad blocks the *laptop*, not the *design*.

If Roy disagrees, delete Groups C, D and E from the prompt and the run still
does real work — Groups A and B alone unblock everything else.
