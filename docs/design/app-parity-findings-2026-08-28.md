# 72 confirmed parity findings, by app

2026-08-28. Fourteen read-only agents audited every app body against
`docs/design/presswork-prototype.html`; every finding was then handed to a
separate agent told to REFUTE it, defaulting to false if it could not
reproduce the claim from source. 98 agents, 0 errors. What survived is below.

The verifiers did not merely rubber-stamp. On the Files finding the verifier
corrected the reporter's line number by 227 lines and raised the cause count
from seven to ten, making the finding worse than reported. Treat every line
here as a claim to reproduce, not as a fact.

**INVENTED FIGURE** marks the category that matters most here: a number or
claim drawn as if measured that nothing measures. 29 of the 72 are that.

Status is as of this file's commit. Items fixed the same day are struck.

## calc (6)

- **INVENTED FIGURE** — The pixel-clock note is a written literal, and the native that reads it already exists
  `/home/roy/Documents/repos/zl-linux-presswork/kernel/apps/apps_system.zl:348` · backing **READ**
  ca_draw prints the string literal "241690 kHz is the measured pixel clock" unconditionally. Nothing in the drawing path measures it. The zl native `gpu_clk()` is registered at /home/roy/Documents/repos/zl-linux-presswork/freestanding/runtime_kernel.c:1595 and calls intel_pixel_clock_khz() (kernel/src/drivers/display/intel.c:789), which returns 0 when intel_present() is false or the transcoder is not eDP (intel.c:759-784) - i.e. 0 under QEMU, the primary target. kernel.zl already calls gpu_clk() twice (kernel/src/kernel.zl:3911, :10535), so this is a read the file declined to make. The same file argues this exact point 197 lines earlier: apps_system.zl:151-157 says the Intel block "reads ZERO

  *What to do:* `gpu_clk()` (runtime_kernel.c:1595 -> intel_pixel_clock_khz). Draw the figure with label_num only when gpu_clk() != 0; when it is 0 print a true alternative - e.g. label "no eDP link on this transcoder" - rather than the reference's number. Precedent for the conditional shape is si_draw's apic_on() branch at apps_system.zl:201-205.

- Mouse hit-test for the key grid is off by 70*u; the bottom row is unclickable
  `/home/roy/Documents/repos/zl-linux-presswork/kernel/apps/apps_system.zl:363` · backing **READ**
  ca_draw's grid origin is gy = ay + 116*u: ky starts at ay+42u (:308), advances +16u three times (:311,:314,:317) to ay+90u, +18u to the rule at ay+108u (:320), then gy = ky + 8*u (:323). ca_event subtracts only 46*u (:363), and the comment above it (:356) asserts "ca_draw's grid starts at (ax + 8u, ay + 46u)" - which the code contradicts. Measured mapping at u=1 (derived by script from the same offsets): real rows occupy client-y 116-159, 160-203, 204-247, 248-291; the handler maps them to r=1..2, r=2..3, r=3..4, r=4..5. Consequences: row 3 (C, 0, =, +) maps entirely to r>=4 and is rejected by the `r > 3` guard - the equals, zero and clear keys cannot be clicked at all; clicking the HEX/OCTA

  *What to do:* No figure involved - the grid origin is already a value in ca_draw (`gy`). Hoist the row-stack arithmetic into a `fn ca_gridy()` used by both ca_draw and ca_event so the two cannot drift, instead of repeating a constant. This is exactly the duplicated-constant class recorded in the box's own learnings (tests-that-restate-constants).

- BINARY row silently drops every bit above 15; the three bases contradict each other and the display
  `/home/roy/Documents/repos/zl-linux-presswork/kernel/apps/apps_system.zl:316` · backing **BACKABLE**
  ac_draw_base(x, y, val, digits, base, color) (kernel/apps/apps_common.zl:82-95) prints exactly `digits` digits at places base^(digits-1)..base^0; anything above base^digits is dropped with no indication. ca_draw passes digits=8 base=16 (:310), digits=11 base=8 (:313), digits=16 base=2 (:316) - three different silent ceilings: 65535, 4294967295, 8589934591. ca_digit (:248) has no cap, so 99999 is five keypresses away: the display well shows 99999, HEX shows 0001869F (correct), and BINARY shows 1000011010011111, which is 34463 - the low 16 bits. Verified by script: true binary of 99999 is 11000011010011111 (17 bits). The file's own comment at :293-295 states "THE THREE BASES ARE THE POINT ...

  *What to do:* Bounded change in kernel/apps/apps_common.zl:82: compute the digit count needed (the same shape as s3_digits at apps_sys3.zl:220-226, generalised to a base) and draw that many, or have ac_draw_base return the count it could not fit so the caller can show an overflow marker. No new native needed.

- Negative values draw non-digit glyphs in all three base rows
  `/home/roy/Documents/repos/zl-linux-presswork/kernel/apps/apps_system.zl:310` · backing **BACKABLE**
  ca_val() can be negative: ca_apply's op 2 is a real subtraction (:242), so 5 - 9 = leaves ca_acc = -4 and ca_val() returns it (:297). zl's % is C semantics - zl_binop casts both operands to long long and returns x % y (freestanding/runtime_kernel.c:1264,1270) - so for a negative val, ac_draw_base's `dv = (val / place) % base` (apps_common.zl:91) is negative, and ac_hexchar(dv) returns 48 + dv (apps_common.zl:77), a character code below '0'. For val = -1 the low hex digit is ac_hexchar(-1) = 47 = '/'. ac_draw_base's own contract note says the opposite of what happens here: apps_common.zl:81 states "`val` is treated as an unsigned magnitude - callers pass band()'d values" - ca_draw passes ca_v

  *What to do:* Two bounded options, both at the call site: band(ca_val(), 0xFFFFFFFF) to honour ac_draw_base's stated unsigned contract (matching the prototype's `v >>> 0` at prototype:1796-1798), or draw a '-' with label() and pass the magnitude. `band` is a live native (src/runtime/runtime.c:1165).

- **INVENTED FIGURE** — "int64" is a claim the runtime does not deliver - a zl number is a double
  `/home/roy/Documents/repos/zl-linux-presswork/kernel/apps/apps_system.zl:319` · backing **CANNOT**
  The WIDTH row prints "int64, no float in the drawing path". The second half is true (zl_binop does all arithmetic in long long, runtime_kernel.c:1264-1270). The first half is not: the kernel is built with the boxed C backend (kernel/build.sh:19,25 -> ../compile), whose N_NUMBER emits zl_num() (src/backends/c/compile.c:296) into a Value whose payload is `double num;` (src/runtime/runtime.h:73). Each operation is done in i64 and the result is stored back as a double, so the exact range is 2^53 = 9007199254740992, not 2^64. ca_digit (:248) has no digit cap, so a 17-digit entry crosses it, and past 2^63 the (long long) cast in zl_binop is undefined. The prototype's own value is capped at 1e9 (pr

  *What to do:* Nothing in the OS measures its own numeric width at runtime. Print the half that is true and verifiable instead: "no float in the drawing path" alone, or "integer ops, exact to 2^53" - and cap ca_digit so the drawn value stays inside whatever the row claims.

- Right-alignment of the display value measures the wrong font
  `/home/roy/Documents/repos/zl-linux-presswork/kernel/apps/apps_system.zl:304` · backing **READ**
  cvw = s3_numw(ca_val()) (:304) is s3_digits(n) * cell_w() (kernel/apps/apps_sys3.zl:227), the fixed mono console cell. Its own doc says what it is for: apps_sys3.zl:216-218, "num_aa has no width function and its advance is cell_w()". But the value is drawn with label_num at T_TITLE/W_BOLD (:305), and label_num passes no MONO flag (runtime_kernel.c:1434), so it renders through the proportional path - fb_text_rich_w at ui_text_h(LG) = 21.0px (uikit.c:280, design.h:543), not the mono cell (uikit.c:279). The two width functions are different code paths and cannot agree except by coincidence. With the 8x16 atlas (cell_w = 8) the 21px proportional run is wider than the reserved box, so the number

  *What to do:* `ui_tnw(n, size, flags)` is a registered native that returns the exact width of a number at a given size and flags (runtime_kernel.c:1890), already used in kernel/src/kernel.zl:5320. Replace s3_numw(ca_val()) with ui_tnw(ca_val(), T_TITLE, 2) (2 = ZL_UI_F_BOLD, runtime_kernel.c:664).

## clock (6)

- **INVENTED FIGURE** — The well asserts NTP corrects the RTC mid-session. This kernel has no NTP and never observes drift.
  `kernel/apps/apps_system.zl:115` · backing **CANNOT**
  Lines 115-116 draw "this rtc runs slow and ntp corrects it mid session, so a / timestamp from before the correction and one from after are / not comparable." Both clauses are claims about the DEVELOPER'S Linux host, copied from the prototype's well text (presswork-prototype.html:1850), presented on screen as facts about the machine zlOS is running on. Verified: `grep -rn 'ntp\|NTP' kernel/src kernel/apps` returns exactly two hits, this label and its own justifying comment at apps_system.zl:27 — there is no NTP client, no SNTP, no time-discipline code anywhere in the tree. And "runs slow" is a comparison against a reference clock; nothing in zlOS ever compares the RTC to anything. The file co

  *What to do:* Nothing measures RTC drift or NTP. Third line (:117) is already true and can stay. Honest replacement for the first two: "the rtc is read once and never differenced. nothing here disciplines it, so two readings taken apart are not comparable." If a drift figure is genuinely wanted later, the only readable version is rtc_epoch() sampled against ticks() over a long interval — that measures RTC-vs-PIT skew, not "behind UTC", and must be labelled as such.

- **INVENTED FIGURE** — "invariant" is printed as an unconditional claim while the native that reads it goes unused
  `kernel/apps/apps_system.zl:95` · backing **READ**
  Line 95: clk_kv(..., "TSC", cpu_khz(), "kHz, invariant"). The figure is read (cpu_tsc_khz, calibrated against the PIT, cpu.c:364), but the word "invariant" is a hardware property — CPUID leaf 0x80000007 EDX bit 8 — and this call site never checks it. On a CPU without an invariant TSC the pane states the opposite of the truth, and cpu.c:444 explicitly acknowledges that hardware exists.

  *What to do:* cpu_inv — bound at freestanding/runtime_kernel.c:2259 to cpu_tsc_invariant() (kernel/src/arch/x86/cpu.c:244-249, which reads 0x80000007 EDX bit 8). Two literal unit strings under `if cpu_inv() == 1` / `== 0` ("kHz, invariant" / "kHz, not invariant") — string VALUES are not needed, only two literals, so it is legal zl.

- **INVENTED FIGURE** — TICK prints the literal 100 and calls it "the PIT divisor", which it is not
  `kernel/apps/apps_system.zl:91` · backing **BACKABLE**
  Line 91: clk_kv(..., "TICK", 100, "Hz, the PIT divisor this kernel programs"). Two separate problems. (a) 100 is a written constant, not a read — nothing in the drawing path asks the timer what rate it was programmed at, so if pit_init() ever changes, this pane keeps saying 100 and the MONOTONIC/SINCE BOOT cards (which divide ticks() by their own literal 100 at line 65) go wrong silently and together. (b) The unit noun is factually wrong: the divisor actually written is 1193182/100 = 11931 (kernel/src/arch/x86/idt.c:494), and the resulting rate is 1193182/11931 = 100.0068 Hz, not 100. The label names a figure the code does not draw.

  *What to do:* idt.c owns the divisor it wrote but exposes only idt_ticks(). Bounded change: keep the divisor in a static in pit_init() and add `unsigned idt_pit_hz(void)` returning 1193182/divisor (or the divisor itself), then one binding line beside "ticks" at freestanding/runtime_kernel.c:2304. Until then the truthful unit noun is "Hz, the rate this kernel programs the PIT to" — dropping the word "divisor" costs nothing and stops the line being false.

- The hour field silently wraps to 00 at 100 h of uptime
  `kernel/apps/apps_system.zl:43` · backing **READ**
  clk_card draws hours with ac_draw_base(vx, vy, clk_hms_h(secs), 2, 10, ...) at line 43 — a fixed 2-digit field. ac_draw_base (kernel/apps/apps_common.zl:82-93) computes dv = (val / place) % base per digit, so a 3-digit value loses its leading digit with no overflow marker. Re-ran that exact arithmetic: 4 -> "04", 99 -> "99", 100 -> "00", 123 -> "23". Division here is integer (freestanding/runtime_kernel.c:1268 casts both operands to long long), so this is not a float artefact. zlOS targets a real laptop; 100 h is 4 days 4 h of uptime. The prototype's own f() prints Math.floor(s/3600) unbounded and never wraps (presswork-prototype.html:1835). A duration instrument that resets to zero without

  *What to do:* label_num already prints an unbounded integer and is used three lines away at :87 for the TICKS card. Draw hours with label_num and keep ac_draw_base's 2-digit pad only for minutes and seconds, where 00-59 is the true range; then advance vx by s3_numw(clk_hms_h(secs)) instead of 2*cell_w().

- The pane repaints off the RTC — the one clock its own well says cannot be trusted — and freezes entirely without one
  `kernel/apps/apps_system.zl:122` · backing **READ**
  clk_tick (lines 122-127) returns 1 only when rtc_s() changes, and line 123 returns 0 outright when rtc_ok() == 0. Every figure the pane actually shows comes from ticks(). So: on a machine with no RTC, or after kernel.zl's three-strike give-up latches (kernel.zl:1486-1493, RTC_GIVE_UP = 3, which is what stops rtc_up() being called and therefore what freezes the cached rtc_s()), the MONOTONIC / SINCE BOOT / TICKS readouts stop advancing while the counter behind them keeps running — and the well underneath still says "every duration above is read from ticks()". The cached seconds only move because kernel.zl:9040 calls rail_clock_tick() once a frame; this app has no tick source of its own despit

  *What to do:* ticks() — bound at freestanding/runtime_kernel.c:2304. Replace the body with `s = idiv(ticks(), 100)` and delete the rtc_ok() guard; clk_last_s already exists and is already private to this app. One line, no new native, and it removes the dependency on the subsystem this pane exists to distrust.

- The head's status dot reports the RTC while its label names the PIT
  `kernel/apps/apps_system.zl:70` · backing **BACKABLE**
  Lines 68-74: the label reads "SOURCE  PIT, 100 Hz on IRQ0", and the status dot immediately to its right is coloured TH_OK/TH_WARN off rtc_ok() — a different subsystem entirely. If the PIT stopped, the dot stays green; if the RTC is missing, the dot goes amber against a PIT that is running perfectly. The word beside it is "monotonic"/"no rtc", neither of which is a state word — the prototype's sw() convention (presswork-prototype.html:1583-1594) exists so colour never carries a state alone, and "monotonic" restates the source kind rather than its state. The prototype prints sw('DRIFT') here, a warn state about the source itself.

  *What to do:* The honest state for a PIT source line is "is idt_ticks() advancing". clk_tick already runs once a frame and would already be holding the previous ticks() value after the fix above — latch a second module-level variable (clk_alive) set to 1 when the counter moved between ticks and 0 when it did not, and key the dot and a two-literal word ("running"/"stalled") off that. No native needed. Do not delete the RTC indication; move it onto the WALL row where it belongs.

## disk (5)

- "cow, journal then superblock swap" — zlfs has no journal and never swaps the superblock
  `kernel/apps/apps_sys3.zl:345` · backing **READ**
  apps_sys3.zl:345 prints it and the comment at :335-339 asserts it is "true of this filesystem rather than borrowed: zlfs journals then swaps the superblock". Both halves are false against the code. fs.c:12 lists the three ideas and a journal is not one of them ("no fragmentation, no free list, no extents"). The superblock is written at exactly two sites, fs.c:514 (invalidate) and fs.c:519 (write), BOTH inside fs_mkfs_impl — format only. The real write path is dir_commit (fs.c:383-397): write the INACTIVE directory generation's data (dir_write_slot, fs.c:308-343), flush, write the checksummed header, flush, then flip dir_active. That is copy-on-write with a header-last publish, not a journal,

  *What to do:* No figure needed — the code is the source. Truthful string that fits the same width: "cow directory, header written last". Cite fs.c:308-343.

- The state WORD is dropped — colour carries the state alone
  `kernel/apps/apps_sys3.zl:321` · backing **READ**
  apps_sys3.zl:321 and :326 draw only `ui_dot(...)` in TH_OK / TH_WARN. The text next to it is a file count (:322-323) or lowercase `no volume` (:328) — neither is a state word. The prototype's sw() helper exists specifically for this and says so at presswork-prototype.html:1583-1584: "THE STATE WORD BESIDE THE STATE COLOUR. Free rider, all four parents do it, and it is why colour never carries a state alone here." R.disk calls sw('OK') at :1861.

  *What to do:* The state is already read — fs_ok() (fs.c:648, fs_mounted). The word only needs drawing: ui_badge is bound (runtime_kernel.c:1915) and ui_caps is bound; either renders 'OK' / 'WARN' beside the existing dot. Zero new natives.

- Both instruments fill with the overprint, which design.h explicitly forbids
  `kernel/apps/apps_sys3.zl:292` · backing **READ**
  apps_sys3.zl:292 sets the capacity segbar fill to theme(TH_ACCENT), and :378 passes theme(TH_ACCENT) to ui_meter for the largest-files bars. design.h:659 reads "meters and bars. These are INSTRUMENTS, so their fill is ZD_STEEL and never the overprint", and design.h:801 is `#define ZD_ACCENT ZD_VERM` — TH_ACCENT resolves to the vermilion overprint. This is defect #1 in HANDOFF-APP-PARITY.md, recorded as fixed in the System Monitor; the Disk app was never swept.

  *What to do:* TH_STEEL = 39 is live and reachable through ui_color (kernel.zl:110, ui.c:219). One-token change at :292 and :378.

- A read-only mount renders in the FAIL colour
  `kernel/apps/apps_sys3.zl:331` · backing **READ**
  apps_sys3.zl:331 passes ok=0 for the read-only volume, so s3du_mount:288-289 paints the flag text theme(TH_DANGER) and :292-293 paints the whole bar TH_DANGER. Read-only is not a fault. The prototype's own state map disagrees explicitly — presswork-prototype.html:1587 puts `RO: 'warn'`, in the WARN group with BOUND and DRIFT, not in the `bad` group with FAIL/DOWN. The parameter is even named `ok`, so the call site is asserting the volume is unhealthy.

  *What to do:* TH_WARN = 23 (kernel.zl:81). If the fabricated rd0 row is deleted per finding 1 this disappears with it; if a read-only row survives, it is WARN.

- **INVENTED FIGURE** — /tmp draws a 100%-full healthy bar on a volume the filesystem refused to mount
  `kernel/apps/apps_sys3.zl:333` · backing **READ**
  fs_capacity() (fs.c:1012) returns sb_data_blocks with NO `mounted` guard, while fs_free_blocks() (fs.c:1021-1023) returns 0 when unmounted. sb_data_blocks is assigned at fs.c:590, before the geometry refusal at fs.c:602-608 and the directory-entry refusal at fs.c:627-632, both of which return 0 leaving mounted=0 and sb_data_blocks set. So s3du_tmp_used() (:257) = (cap - 0) * bs = the whole volume. apps_sys3.zl:333 then hardcodes ok=1, so the bar is drawn in the healthy colour at 100% — while the head at :326-328 has correctly branched to `rd0, unformatted` / WARN. Same window, two answers, and the one drawn as a measurement is derived from stale state.

  *What to do:* Gate the row on fs_ok() the way the head at :319 already does, or add the `mounted` guard to fs_capacity() (fs.c:1012) so it matches fs_free_blocks(). Root fix is the one-line guard in fs.c.

## edit (2)

- **INVENTED FIGURE** — Window subtitle is a hardcoded "fb.c" — there is no fb.c on the volume
  `kernel/src/kernel.zl:804` · backing **BACKABLE**
  rail_sub(3) returns the literal "fb.c" (kernel/src/kernel.zl:804), copied straight from the prototype's APPS table (presswork-prototype.html:1549). label_windows() pushes it into the editor window's chrome once per frame via wm_set_label (kernel.zl:842), so the title bar states which file is being edited — and states a file that does not exist. The editor's default buffer is /user/notes.txt (editor_open_default, kernel.zl:3496-3507) and open_disk_editor(slot) opens any zlfs slot (kernel.zl:8962-8979). zlfs only ever seeds hello.zl (kernel/src/fs/fs.c:1095-1102). So the one cell in the whole editor chrome that names a file names a file that is not there, and it does not change when you open a

  *What to do:* The name is already read from the volume one byte at a time — fs_ch(slot, i), bound to fs_name_byte (freestanding/runtime_kernel.c:2470), used at kernel.zl:2812 and kernel.zl:6487. wm_set_label takes a const char*, which zl cannot construct (string literals, no string values), so the bounded change is one new native — wm_label_name(win, reg, slot) — that copies the zlfs name in C, exactly the seam fs_ch already is. Until then, print nothing rather than "fb.c": wm.c renders an empty subtitle deli

- No viewport and no scroll — the editor goes blind past one screen
  `kernel/src/kernel.zl:2803` · backing **READ**
  The prototype's body is `.well.scroll` (presswork-prototype.html:1706). editor_draw always starts at ed_i = 0 (kernel.zl:2792) and aborts the draw the moment ed_y passes ay + ah - ed_ch*2 (kernel.zl:2803). There is no scroll offset anywhere in the editor's state (ed_slot, ed_len, ed_dirty — kernel.zl:3479-3482). EDIT_MAX is 8000 bytes (kernel.zl:2469) against roughly (ah / cell_h) - 2 visible rows, so once the buffer is taller than the window every further keystroke is appended to EDIT_BUF (kernel.zl:3592-3594) and drawn nowhere. This is not cosmetic parity — the missing well is the reason the editor stops showing what you type.

  *What to do:* ui_scroll / ui_scroll_content / ui_scroll_end are already bound natives (freestanding/runtime_kernel.c ui_* table), and ui_mono(x,y,w,h,style) is the well itself — kl_ui uses it at kernel/apps/apps_sys2.zl:313. The scroll extent is a real read: the LF count from the peek8 loop editor_draw already runs at kernel.zl:2794.

## files (5)

- **INVENTED FIGURE** — INVENTED CLAIM: the empty state asserts a mount-failure cause it never reads, and offers to format on it
  `kernel/src/kernel.zl:6540` · backing **BACKABLE**
  kernel.zl:6540 prints the flat assertion "the disk is present and unformatted" whenever fs_ok() == 0, and kernel.zl:6543-6545 then offers SHIFT F to format. fs_ok() is fs_mounted() (fs.c:648), and fs_mount_impl (fs.c:539-635) returns 0 from SEVEN distinct causes: probe_device failed ("no disk - NVMe controller did not come ready", fs.c:449), superblock unreadable (fs.c:544), magic mismatch = genuinely unformatted (fs.c:552), on-disk version refused (fs.c:561), superblock checksum bad, where fs.c:568-569 explicitly prints "the volume is damaged, not empty. Nothing was read.", geometry does not fit the disk (fs.c:629), and a directory entry out of range (fs.c:653). The app collapses all seven

  *What to do:* Half of it is already READ and unused: nv_ready() (runtime_kernel.c:2224 -> nvme_ready) and nv_ok() (runtime_kernel.c:2222 -> nvme_present) are live natives already called from zl at apps_sys3.zl:457 and apps_sys3.zl:445 — files_draw calls neither, so `if nv_ready() == 0` gives a truthful "no disk" arm for zero new natives. The rest is a bounded change: fs.c already computes the reason at each return; store it in one static, add `int fs_why(void)`, bind one native line beside fs_ok at runtime_ke

- Click and keyboard row geometry disagree with the draw geometry — three copies, none reading ui_colhead_h()
  `kernel/src/kernel.zl:6620` · backing **READ**
  files_draw puts the first row at files_y0 = ay + 62*u + ui_colhead_h() (kernel.zl:6576) and fits files_rows = (ah - 104*u - ui_colhead_h()) / (24*u) (kernel.zl:6577). The other two copies of that arithmetic never learned about the column head: files_mouse_slot uses files_my0 = wm_cy(win) + 72*u (kernel.zl:6620) and files_mrows = (wm_ch(win) - 104*u) / (24*u) (kernel.zl:6623); files_event's scroll clamp uses the same headless count at kernel.zl:6645. wm_cy(win) is the same client_of() rectangle app_draw is handed (wm.c:1684-1689, wm.c:3510-3522), so the origins are directly comparable. ui_colhead_h() is DP(ZD_COLHEAD_H) = DP(20) (uikit.c:1255, design.h:633), which is exactly 20*ui at every in

  *What to do:* ui_colhead_h() is already bound as a native (runtime_kernel.c:1823) and already called in the draw path. Fix once by extracting a files_row_geom-style helper the draw, the mouse and the key handler all call, rather than patching 6620/6623/6645 separately.

- Failure scenario for the row geometry: a click near the bottom of a row selects the row below it
  `kernel/src/kernel.zl:6623` · backing **READ**
  Concrete, at ui scale 1: row 0's drawn band is [ay+82, ay+106). files_mouse_slot computes files_mrow = (ey - (ay+72)) / 24. A click at ey = ay+100 — visibly inside row 0, 6px above its bottom edge — yields (100-72)/24 = 1, and files_slot_at_order(files_scroll + 1) selects the SECOND file. Because files_event ety==4 with band(ecode,256) also opens on the same event (kernel.zl:6684), a double-click aimed at one file opens a different one in the editor. The mis-selection covers the lower 10 of every 24 scaled pixels, i.e. ~42% of every row, at every scale.

  *What to do:* Same helper as above. No new native.

- MISSING: the entire detail well — rule, selected-file row, and four kv rows
  `kernel/src/kernel.zl:6606` · backing **READ**
  The prototype's skeleton after the table (presswork-prototype.html:1675-1681) is `.r2` rule, then a `selected` row carrying name + size + sw('OK'), then kv('full path'), kv('mode'), kv('extent'), kv('crc32'). files_draw has NONE of it: after the row loop (kernel.zl:6606) it goes straight to the empty-volume label and the delete-warning / bytes-free footer at kernel.zl:6608-6614. The repo's own HANDOFF-APP-PARITY.md:36 names the shared skeleton as "head line -> table or cards -> rule -> detail well -> band"; Files has head and table and stops. The four kv rows classify differently and are listed separately below — this entry is the missing structure (rule + selected row) itself.

  *What to do:* The selected row's two figures are pure reads already in the file: the name via files_draw_name (kernel.zl:6487) and the size via fs_bytes (runtime_kernel.c:2466 -> fs_size, fs.c:1006). The sw('OK') state word has nothing behind it — see the extent finding for what could honestly stand there.

- kv('extent') — two of its three figures are readable today and the natives sit unused
  `kernel/src/kernel.zl:6606` · backing **READ**
  presswork-prototype.html:1680 prints "generation 3, 1 extent, contiguous". zlOS prints nothing. "1 extent, contiguous" is TRUE OF ZLFS BY CONSTRUCTION — fs.c:11-16 states the design as "files as CONTIGUOUS RUNS - no fragmentation, no free list, no extents" — so the reference happens to be describing this filesystem accurately, and the run is directly readable: fs_lba (runtime_kernel.c:2467 -> fs_start, fs.c:1007) and fs_run (runtime_kernel.c:2468 -> fs_runlen, fs.c:1008). Neither native is called from any .zl file in the tree (verified by grep across kernel/**/*.zl: fs_lba and fs_run appear only at kernel.zl:4505, inside a console command, never in an app). So an honest extent line — start L

  *What to do:* fs_lba(slot) and fs_run(slot), already bound at runtime_kernel.c:2467-2468. "contiguous" is a structural fact of fs.c, not a measurement, and may be stated. "generation 3" is the one part that is NOT readable — see the next finding.

## hex (5)

- Only 8 of the 16 coded rows can ever draw at the shipped window size
  `kernel/apps/apps_sys2.zl:535` · backing **READ**
  The row loop is `while r < 16` guarded by `if by + lh <= py + ph` (apps_sys2.zl:535-536). Derived from the tree's own constants at 1920px wide (scale_q8=256, so UI_DP(n)==n, ui.h:120): title_h=DP(ZD_TITLE_H)=28 (design.h:360, ui.c:256), band_h=DP(ZD_STATUS_H)+1=21 (wm.c:1622-1628), so client ah = 340-28-2-21 = 289 (wm.c:1655-1662). Then ph = ah - ui_tb_h()(30, design.h:600) - ui_sb_h()(20, design.h:603) - ui_colhead_h()(20, design.h:633) = 219. lh = fb_cell_h()*3/2 = 24 (uikit.c:1743-1748). by starts at py+7 and steps 24, so rows land at 7,31,55,79,103,127,151,175 and the 9th at 199 fails 199+24<=219. Eight rows = 128 bytes. Prototype R.hex draws 20 rows (presswork-prototype.html:1766). Reco

  *What to do:* Nothing new is needed to READ the count — the loop already knows it. The fix is either to let the loop fill the pane (drop the hard 16) or to size the window so 16 rows fit; at the prototype's own 620x420 geometry 12 rows fit, still not 16.

- **INVENTED FIGURE** — "256 B/page" is a figure the pane does not deliver
  `kernel/apps/apps_sys2.zl:511` · backing **READ**
  apps_sys2.zl:511 prints the literal "256 B/page" in the header well. It is true of the chip stride and false of the pane: 8 rows x 16 bytes = 128 B are shown at the shipped 566x340 window, 176 B at 2560 wide. It is drawn in the slot where ds-reference.html:272 puts `{{ hexSize }}` — a read value — so it reads as measured. This is the file's own overriding rule broken in a literal.

  *What to do:* The drawn-row count is already computed inside hx_ui by the loop at :535-554. Count the rows that pass the fit test into a local and draw `count * 16` with the existing `ac_draw_base(x, y, val, digits, 10, ink)` (apps_common.zl:82), then a literal " B/page". One extra local, one line, no new native.

- **INVENTED FIGURE** — "/dev/kmem" names a path that does not exist on this system
  `kernel/apps/apps_sys2.zl:510` · backing **READ**
  apps_sys2.zl:510 draws the literal "/dev/kmem" as the well's identity, in the exact slot ds-reference.html:271 fills with `{{ hexPath }}` — a real path. zlOS has no device namespace: zlfs is a flat name store with no directories (fs.c superblock is SB_MAGIC/VERSION/BSIZE/BLOCKS/DIRLBA/DIRBLK/DATALBA/DATABLK/MAXFILES/CSUM, fs.c:216-225; kernel.zl:6570 records it as "a flat store with NO MODES"), and no fs_* native resolves a path (runtime_kernel.c:2460-2496 are all index-addressed). The app's own status bar 53 lines later says the true thing — "kernel image - live memory" (:563) — so the pane contradicts itself.

  *What to do:* The true answer is already a literal in this function. Change the well's literal to "kernel image" (or "physical memory"). No native, no measurement.

- The ascii column header is anchored to the opposite edge from the ascii data
  `kernel/apps/apps_sys2.zl:522` · backing **READ**
  The head is laid out by the grid (`ui_grid("74|*|150")` :522, `ui_colhead` :523). ui_grid_span walks tracks from x+DP(ZD_COLHEAD_PL)=+4 and the last fixed track ends flush at x+w-DP(ZD_COLHEAD_PR)=aw-15 (uikit.c:1227-1252), so column 2 is RIGHT-anchored to the window. The data is LEFT-anchored by mono accumulation: as0 = 9u + 8cw + 13u + 35cw + 13u = ax+379 at u=1/cw=8 (:530-532), width 18 cells = 144px, right edge ax+523. The head's text right edge is cx+cw-DP(ZD_CELL_PX) = ax+aw-21. At the shipped aw=560 (566-(1+ZD_FOCUS_BAR=3)-2, wm.c:1655-1658) that is ax+539 — a 16px gap. Windows resize (wm.c:2184 min_w, in_resize_grip :3739), and the gap grows 1:1: +100px of width gives a 116px gap, -1

  *What to do:* Both numbers are already in the function. Either derive the grid from the same cw arithmetic the rows use (the track widths are the only invented part of :522), or draw the three heads at bx / hx0 / as0 directly.

- Two of the three column heads come out right-aligned; the prototype left-aligns all three
  `kernel/src/graphics/ui/uikit.c:1303` · backing **READ**
  ui_colhead right-aligns any fixed, non-star track: `int tx = (G.fixed[i] && i != G.star) ? cx + cw - tw - cp : cx + cp;` (uikit.c:1303). With "74|*|150" that right-aligns "offset" and "ascii" and left-aligns only "bytes". The prototype's `th` is `text-align: left` (presswork-prototype.html:779) and only `th.r` flips it (:788) — R.hex passes `r` on none of its three columns (:1781-1782), so all three heads are left-aligned there. This is the mechanism behind the ascii drift above.

  *What to do:* No measurement involved. Either give hx_ui a grid whose fixed tracks match the drawn columns, or add a left-align flag to ui_colhead. Note the widget is shared, so changing :1303 is a 53-app blast radius — fix it at the caller.

## log (5)

- **INVENTED FIGURE** — "ring 14 records - 0 dropped" asserts zero loss the OS measures and never reads
  `kernel/apps/apps_sys2.zl:265` · backing **READ**
  The head's right-aligned run is a string literal, drawn twice (once to measure width, once to draw). The comment above it at apps_sys2.zl:262-263 defends it as "Both true - the log is fourteen fixed rows, so nothing can have been dropped" — true of the app's own static table, false of the machine. This kernel HAS a real log ring: `static u8 ram_records[4096][64]` with `ram_head, ram_count, ram_dropped` (kernel/src/core/zllog.c:66-67), and `ring_append` increments `ram_dropped` on every admission refusal (zllog.c:256). 4096 x 64 = 262144 = 256 KiB, not the prototype's 64 KiB and not 14 records. Drawing "0 dropped" in the slot the prototype reserves for the ring's loss figure is the exact fail

  *What to do:* `diag_drop()` -> zllog_dropped() -> ram_dropped, registered at freestanding/runtime_kernel.c:2202 and already called from kernel.zl:3737. Ring capacity: ZLLOG_RAM_RECORDS(4096) x ZLLOG_RECORD_BYTES(64) at zllog.c:45,51 — needs one trivial native to reach zl, or print "ring 4096 records" once exposed. Honest text today with zero new natives: drop the ring clause and print only what diag_drop() returns.

- **INVENTED FIGURE** — The 14 boot timestamps are invented; a real per-record TSC exists
  `kernel/apps/apps_sys2.zl:134` · backing **BACKABLE**
  kl_time is a 14-branch literal ladder returning 2, 11, 17, 34, 48, 63, 95, 140, 182, 214, 260, 298, 331, 372, drawn into the right-aligned "time" column at apps_sys2.zl:290. Nothing produced these numbers and they carry no unit anywhere on screen — the prototype's column is seconds to 6 dp ("0.000041"), so a reader cannot even tell what 372 means. The header comment at apps_sys2.zl:128-129 concedes it ("nothing records a per-subsystem boot time") but the figures are drawn anyway, in a numeric right-aligned column that reads as measured.

  *What to do:* Every zllog record already carries a real timestamp: make_record writes cpu_tsc() to bytes 8..15 (zllog.c:223), and cpu_tsc_khz() is declared at zllog.c:16 — so ms are derivable with integer division, no float. Needs one accessor native over ram_records (e.g. klog_ms(i)); the record fields are all scalars, so it is one line and well under 8 args. `ticks()` (idt_ticks) is ALREADY a live native at runtime_kernel.c:2304 if a coarser figure is acceptable. Honest text with zero new natives: delete th

- The band says "14 of 14 messages" while only 7 rows are ever drawn
  `kernel/apps/apps_sys2.zl:323` · backing **READ**
  The row loop clips silently: `if ty2 + rowh <= limit` (apps_sys2.zl:286) skips any row that will not fit, with no scrollbar, no ellipsis and no count of what was cut — the prototype puts the table inside `<div class="scroll">` so all 32 rows are reachable. The band then reports kl_count() of KL_N, and kl_count() counts rows PASSING THE FILTER (apps_sys2.zl:225-230), not rows drawn, so it always says 14 of 14. Verified arithmetic at ui=1: window 320 (sys2_h, apps_sys2.zl:816); client h = 320 - title 28 (ZD_TITLE_H, design.h:360) - border 2 - wm band 21 (band_h_of, wm.c:1622-1628) = 269; limit = 269 - sbh 20 (ZD_STATUS_H, design.h:603) - wellh 46 - 8 = 195; first row at hdh 22 + colhead 20 = 4

  *What to do:* `shown` is already incremented per drawn row at apps_sys2.zl:304 and is in scope at line 323 — printing `shown` instead of `n` makes the band true for the cost of one identifier. The real fix is a scroll viewport, but the truthful readout costs nothing.

- Row selection is dead: kl_sel_row is written nowhere in the tree
  `kernel/apps/apps_sys2.zl:345` · backing **READ**
  The prototype's rows are pick targets — every TR carries `class="pick" data-act="log" data-arg="i"` (presswork-prototype.html:1750) and the handler sets S.logSel and redraws (presswork-prototype.html:2776), which is what makes the detail well below the table mean anything. In zlOS the selection variable is initialised once and never assigned: grep across every .zl/.c/.h in the repo returns only its declaration (apps_sys2.zl:345) and its two readers (346, 347). The well at apps_sys2.zl:314-316 is therefore permanently frozen on record 0 ("multiboot" / "entry, 32-bit protected mode") and no click can move it.

  *What to do:* ui_grid_row ALREADY hit-tests: `int fired = ui_fire(x, y, w, h);` at kernel/src/graphics/ui/uikit.c:1379-1381, and returns it. kl_ui calls it as a bare statement at apps_sys2.zl:287 and throws the return away. Capturing it (and passing `i`, not `shown`, as the identity) wires selection with the machinery already in place.

- The severity filter is unreachable AND actively reset on every click
  `kernel/apps/apps_sys2.zl:349` · backing **READ**
  kl_filter / kl_shown / kl_count / kl_apply (apps_sys2.zl:132, 217-231, 349) implement a three-way all/warnings/errors filter. Nothing draws a control for it: `ui_seg` is used twice in this very file — rd_ui at apps_sys2.zl:421 and ft_ui at 742 — but never inside kl_ui. Worse, it cannot even hold a value: kl_ui's only return is `return 0` at apps_sys2.zl:331, so in sys2_event `g` is always 0, the guard `if g < 0 { return 0 }` (apps_sys2.zl:903) does not fire, and `kl_apply(0)` runs on every click (apps_sys2.zl:904), pinning kl_filter to 0 forever. Note the prototype's R.log has NO filter at all — this whole subsystem is imported from ds-reference.html, which the header comment at apps_sys2.zl

  *What to do:* ui_seg/ui_seg_w/ui_seg_h are live natives used elsewhere in this same file. Either wire it (return the seg index from kl_ui instead of 0) or delete it to match the prototype. Do not leave it half-present.

## mon (6)

- **INVENTED FIGURE** — "ioapic lines 16 routable" is a written figure and a read one exists
  `kernel/src/kernel.zl:5389` · backing **READ**
  pw_sect prints the literal 16 as the section count and the row loop is bounded by the same literal. The IOAPIC's pin count is real and readable: apic.c:409 computes it as ((IOAPIC VER >> 16) & 0xFF) + 1, apic.c:432 exports apic_ioapic_pins(), and runtime_kernel.c:1570 already binds it as the zl native apic_pins. On any IOAPIC with 24 pins (the common case) the pane asserts 16 lines are 'routable' when the hardware says otherwise. Separately, apic_redtbl itself refuses irq > 15 (apic.c:457) and apic_gsi the same (apic.c:453), so the ROW loop genuinely cannot go past 16 today — but the section COUNT can and must be read. Honest form: print apic_pins() as the count and say the table shows the 1

  *What to do:* apic_pins native, freestanding/runtime_kernel.c:1570 -> apic_ioapic_pins() (kernel/src/arch/x86/apic.c:432), value latched from IOAPIC VER at apic.c:409

- **INVENTED FIGURE** — The per-core "wakes" column prints one global sum on every row
  `kernel/src/kernel.zl:5306` · backing **BACKABLE**
  sm_core_row draws smp_jobs(i) in a column headed 'wakes' inside a table headed 'core'. The native DISCARDS the index: runtime_kernel.c:2216 is `smp_jobs -> smp_band_wakes()`, and smp.c:242-246 sums ap_slots[1..ap_slots_live].wakes. Two consequences, both verified in the C: (a) every core row prints the identical number, so the column reads as per-core and is not; (b) the sum starts at slot 1, so the boot core's row prints a total that by construction contains none of its own work. The comment directly above at kernel.zl:5295-5298 asserts 'Every column is read: ... smp_jobs is band wakes completed' — that comment is wrong about which core's wakes.

  *What to do:* The per-slot counter already exists — struct ap_slot.wakes (kernel/src/arch/x86/smp.c:118) incremented at smp.c:195. Needs `u32 smp_slot_wakes(int i)` beside smp_band_wakes at smp.c:242 plus one line in runtime_kernel.c. Note slot index is assignment order (`int slot = ap_online` at smp.c:164), not MADT index, so the accessor must be keyed on APIC id or the row must be keyed on slot.

- Per-core UP/DOWN is a count comparison, not a per-core read
  `kernel/src/kernel.zl:5302` · backing **READ**
  `on = 1` iff `i < smp_n()`. smp_n is smp_online() = ap_online + 1 (smp.c:240) — a COUNT of how many answered, with no identity attached. smp_start (smp.c:305-321) walks MADT entries and skips any whose IPI fails, so the set that answered is not necessarily the first N entries. If MADT index 1 fails and index 2 answers, ap_online==1, smp_n==2, and row 1 prints UP next to smp_id(1) = apic_cpu_id(1) (apic.c:436) — the APIC id of the core that did NOT come up. The pane's own comment (kernel.zl:5295-5298) claims 'state is whether this core came up at all'; it is whether the ONLINE COUNT reached this row's index.

  *What to do:* ap_mask is already a per-APIC-id bitmask — smp.c:159 `if (id < 32) ap_mask |= (1u << id)`, exported as smp_mask() at smp.c:249 and bound as the zl native smp_mask at runtime_kernel.c:2208. With band (runtime_kernel.c:2539) and shl (runtime_kernel.c:2543), both already used across kernel/apps/*.zl, the read is one line: `if band(smp_mask(), shl(1, smp_id(i))) != 0 { on = 1 }`. Valid for APIC ids < 32, which is the same ceiling ap_mask has.

- min / mean / budget are drawn with no unit, beside the same budget in a different unit
  `kernel/src/kernel.zl:5404` · backing **READ**
  sm_fig (kernel.zl:5229-5242) draws only a caps label and a number — there is no unit argument and none is drawn. The three figures in that row are min (dp=1), mean (dp=1) and budget (dp=3), all encoding milliseconds, and none says so. Immediately above them, kernel.zl:5400 prints the SAME budget value as '16667 us budget'. So the pane shows '16667 us budget' and then 'BUDGET 16.667' four pixels below, and the only unit word on screen is the wrong one for the lower reading. The reference writes 'ms' on all three (prototype:1731-1733). This is the exact hazard the file's own pw_dec comment is written about — a number in a column that means something else.

  *What to do:* No new data needed. sm_fig takes 6 args, so a 7th unit-string arg still fits under the 8-arg native ceiling (sm_fig is a zl fn, not a native, so it is unconstrained), and ui_txt/ui_tw already draw and measure the suffix the way sm_meter does at kernel.zl:5137-5140.

- min and mean cover a different sample window than the chart they sit under
  `kernel/src/kernel.zl:5199` · backing **READ**
  sm_ring_min (5199) and sm_ring_mean (5214) both loop `while i < wm_sn()` — every sample the ring holds, up to WM_SAMPLE_N = 256 (wm.c:4229). sm_spark (5156) draws only the last SPARK_MAX = 64 (kernel.zl:2219, matching design.h:739). So the figures describe up to 256 frames while the chart above them shows 64, and a spike that has already scrolled off the chart still holds the 'min' underneath it. The head at 5365 prints wm_sn() itself — up to 256 — so it describes neither the chart nor a rate. The reference ties all three to one window ('composite time, 64 s' / 64 bars / min+mean beneath).

  *What to do:* Same ring, same natives (wm_sn at runtime_kernel.c:1717, wm_sf at 1718). Clamp both loops to the last SPARK_MAX samples exactly as sm_spark already does at kernel.zl:5162-5164, or draw all 256 in the chart. Either is internal; nothing new is needed.

- The "cores" section count and the row count come from two different sources
  `kernel/src/kernel.zl:5376` · backing **READ**
  The section head count is smp_total() = apic_cpus() = cpu_count from the MADT (apic.c:434), labelled ' usable'. The row loop is bounded by cpu_thr() = cpu_threads(), which is CPUID leaf 0x0B's core-level logical-processor count (cpu.c:177-190). These are different questions with different answers — firmware enumeration versus silicon topology — and the pane never says which is which. Where CPUID reports more logical processors than the MADT lists (QEMU -smp 1 on a multi-thread host is the routine case), the extra rows print DOWN, which reads as 'this core failed to start' when the truth is 'firmware never listed it'. ' usable' is also the wrong word for a firmware-listed count.

  *What to do:* Both natives already exist and disagree honestly: smp_total (runtime_kernel.c:2206) and cpu_thr (runtime_kernel.c:2241). The fix is labelling, not measurement — bound the rows by smp_total() and print cpu_thr() as a separate 'threads' reading, or keep both rows and say which source each column answers to.

## net (6)

- **INVENTED FIGURE** — Driver name is typed, not read — "virtio-net" hardcoded while net_kind() exists and is already used elsewhere in this repo
  `kernel/apps/apps_sys3.zl:1109` · backing **READ**
  s3nw_drv(i) returns the string literal "virtio-net" for every non-loopback interface (apps_sys3.zl:1109). It is drawn twice: flush-right in the head (apps_sys3.zl:1237) and as the `driver` kv value (apps_sys3.zl:1267). netdev.c:31-43 selects among CDC-ECM, e1000 and virtio at runtime, preferring CDC-ECM (netdev.c:34-37 states the ThinkPad's onboard I219 has no carrier without its dongle), so on real hardware this pane will confidently print "virtio-net" for an e1000 or a USB CDC-ECM link. The comment directly above it (apps_sys3.zl:1224-1233) asserts this slot is "THE DRIVER THIS INTERFACE ACTUALLY HAS, read rather than written" and recounts catching exactly this defect once before — the fix

  *What to do:* net_kind() → netdev_kind() (freestanding/runtime_kernel.c:2084; enum in kernel/src/drivers/network/netdev.h:3-4: NONE=0, VIRTIO=1, E1000=2, CDC_ECM=3). Three literals selected by an int comparison — no string values needed. Pattern already in the tree at kernel/src/kernel.zl:7064-7066. Pair with net_devid() → e1000_device() (runtime_kernel.c:2085, e1000.c:247) for the Intel device id, as kernel.zl:7048 already does.

- Link state gated on virtio_net_ready() for every driver — pane shows DOWN while its own counters climb
  `kernel/apps/apps_sys3.zl:1105` · backing **READ**
  s3nw_up(1) returns net_ok() (apps_sys3.zl:1105), and net_ok is bound to virtio_net_ready() (runtime_kernel.c:2079 → virtio_net.c:696), which is virtio-specific. Every counter on the same pane comes from the driver-agnostic netdev_* layer: net_rx → netdev_rx_count(), net_tx → netdev_tx_count(), net_drop → netdev_rx_drops() (runtime_kernel.c:2086-2088). On an e1000 or CDC-ECM link virtio_net_ready() is 0, so the head prints DOWN (apps_sys3.zl:1223), the state row prints DOWN in danger red (apps_sys3.zl:1259), the sidebar prints "down" instead of the address (apps_sys3.zl:1158) and the toggle offers "ifup eth0" (apps_sys3.zl:1177) — all while rx/tx packets on rows 5 and 6 increase. The correct

  *What to do:* net_link() → netdev_link_up() (runtime_kernel.c:2083, netdev.c:57-58). Already called at kernel/src/kernel.zl:1322 and kernel.zl:7068. One-line change: `return net_link()`.

- **INVENTED FIGURE** — errors row prints two typed zeroes beside one measured figure, in the same face and colour
  `kernel/apps/apps_sys3.zl:1273` · backing **READ**
  The `errors` kv row draws the literal "0  dropped " (apps_sys3.zl:1273), then the real net_drop() value, then the literal "  overruns 0" (apps_sys3.zl:1276). Two of the three numbers on that row are typed constants rendered in theme(TH_TEXT) identically to the measured one, so a reader cannot tell which is which. net.c actually counts six error classes — c_badsum, c_short, c_ver, c_ihl, c_frag, c_notours (net.c:598-609) — and every one of them is already a zl native. The row therefore does not merely fail to measure errors; it prints 0 while non-zero measurements sit unread. kernel.zl:7159 already prints ip_badsum() and ip_notours() in the boot console.

  *What to do:* ip_badsum, ip_notours, ip_short, ip_badver, ip_badihl, ip_frag (freestanding/runtime_kernel.c:2069-2076) → net_rx_bad_csum/not_ours/short/badver/badihl/frag (net.c:598-609). Note these are IP-layer errors, not link errors: label the row "ip errors" rather than "errors", and drop "overruns" entirely — nothing in netdev.c counts an overrun distinct from netdev_rx_drops().

- **INVENTED FIGURE** — address and ping target are compile-time QEMU literals while DHCP runs every frame and dhcp_ip()/dhcp_gw() exist
  `kernel/apps/apps_sys3.zl:1110` · backing **READ**
  s3nw_addr(1) returns NET_ME (apps_sys3.zl:1110), drawn in the sidebar (apps_sys3.zl:1156) and the `address` row (apps_sys3.zl:1262). The ping line prints NET_GW (apps_sys3.zl:1295) and pings it (apps_sys3.zl:1370). Both are literals in kernel.zl:6938-6939 (167772687 = 10.0.2.15, 167772674 = 10.0.2.2) — QEMU user-mode defaults. Meanwhile the boot path calls ip_auto() (kernel.zl:7011) and every WM loop pumps dhcp_work() (kernel.zl:10618, 10985, 11000), so on any DHCP link the machine's real address is dhcp_ip() and this pane shows 10.0.2.15 regardless, and the ping button probes a gateway that is not this network's. The file header (apps_sys3.zl:1090-1091) claims the addresses are "DERIVED fro

  *What to do:* dhcp_ip / dhcp_gw / dhcp_mask natives at freestanding/runtime_kernel.c:1989-1991 → dhcp_address()/dhcp_gateway()/dhcp_mask(), guarded by dhcp_state()==3 (runtime_kernel.c:1988). Fall back to NET_ME only when dhcp_state()!=3, and say so on the row (e.g. draw the value dim, or add a `static` marker) so the asserted case is visibly different from the leased one.

- The "ifdown" pill cannot take an interface down — the label promises an action the handler does not perform
  `kernel/apps/apps_sys3.zl:1176` · backing **CANNOT**
  s3nw_toggle_label() returns "ifdown eth0" whenever the link is up (apps_sys3.zl:1176) and the pill is drawn with that text (apps_sys3.zl:1188). The click handler only ever brings the interface UP: `if s3nw_sel == 1 { if net_ok() == 0 { net_up() } }` (apps_sys3.zl:1363). The comment at apps_sys3.zl:1359-1362 states this plainly — "there is no ifdown, so the toggle only ever brings an interface UP" — so the defect is documented and shipped. Pressing a button labelled ifdown repaints and does nothing. Same for lo, where the click is a total no-op.

  *What to do:* Nothing in netdev.c, virtio_net.c or e1000.c exposes a teardown — netdev_init() is the only bring-up and it is idempotent. Honest fix is the label, not the driver: when the link is up, draw the pill disabled with "link up" (or omit it) rather than naming an operation that does not exist. Only ever offer "ifup <name>" as a live control.

- **INVENTED FIGURE** — state row prints "UP RUNNING" — RUNNING is asserted while the carrier read sits unused
  `kernel/apps/apps_sys3.zl:1257` · backing **READ**
  apps_sys3.zl:1257 draws the literal "UP RUNNING". In ifconfig semantics UP is the administrative flag and RUNNING means carrier present; here both words come from one value, net_ok() = virtio_net_ready() (runtime_kernel.c:2079), which reports driver init, not carrier. The actual carrier read exists and this pane never calls it.

  *What to do:* net_link() → netdev_link_up() (runtime_kernel.c:2083). Four literals cover it: "UP RUNNING" (init ok + carrier), "UP NO-CARRIER" (init ok, no carrier), "DOWN" , and the lo case. Selecting a literal by int comparison is legal zl; concatenation is not, which is why four literals rather than two.

## regs (6)

- **INVENTED FIGURE** — Window chrome says "pipe A" on a pane that reads no pipe
  `kernel/src/kernel.zl:863` · backing **READ**
  rail_sub(12) returns the literal "pipe A" (kernel.zl:863) and label_windows() stamps it onto whatever window rail_app(12) resolves to (kernel.zl:890, wm_set_label(lw, ls+1, rail_sub(ls))). rail_app(12) is 32 = System Info (kernel.zl:1684). wm_set_label stores it in wins[].sub (wm.c:1835-1848) and chrome_title draws it after the uppercased title (wm.c:2777-2783). So the titlebar reads `13  SYSTEM INFO  pipe A` while si_draw's own head, six lines of code away, reads `BLOCK  cpuid + local apic` (apps_system.zl:198-199). Nothing in the System Info window touches TRANS_EDP, PIPE_A, or any display register — si_val() (apps_system.zl:180-190) calls only cpu_*/smp_n/apic_on. This is a caption copied

  *What to do:* intel_ok() (intel_supported(), bound at freestanding/runtime_kernel.c:2277) already answers whether an Intel pipe exists, and apps_sys2.zl:380 already branches on it. Either make rail_sub(12) say what the pane reads ("cpuid" / "apic"), or point slot 12 at a real display-register pane and let "pipe A" become true.

- **INVENTED FIGURE** — si_reg names two CPUID fields the code does not read
  `kernel/apps/apps_system.zl:160` · backing **READ**
  si_reg(0)="CPUID.1:EBX[23:16]" and si_reg(1)="CPUID.1:EBX[31:24]" (apps_system.zl:160-161) are printed in a column headed `register` beside values that come from somewhere else. si_val(1)=cpu_thr()=cpu_threads(), and cpu.c:174-176 states in its own words that leaf-1 EBX[23:16] "is a maximum, not a count, and lies on most parts" — cpu_threads() (cpu.c:177-191) reads CPUID leaf 0x0B core-level EBX[15:0] and only falls back to EBX[23:16]. si_val(0)=cpu_cores() (cpu.c:202-206) is not a register field at all: it is cpu_threads()/cpu_threads_per_core(). And EBX[31:24] is the initial APIC ID — cpu_apic_id() (cpu.c:209-214) reads exactly that field, and it is nothing like a thread count. So two of n

  *What to do:* Nothing new needed. Row 1's honest name is "CPUID.0B:EBX[15:0]" (core level) with decode "logical processors the topology leaf enumerates"; row 0 is derived, not read — name it "derived: threads / threads-per-core" so the register column stops asserting an address.

- **INVENTED FIGURE** — si_reg(8) "IA32_APIC_BASE[11]" is a software flag, and the MSR can disagree with it
  `kernel/apps/apps_system.zl:168` · backing **READ**
  si_val(8)=apic_on() (apps_system.zl:189) binds to apic_active() (runtime_kernel.c:1566), which is `int apic_active(void) { return apic_on; }` (apic.c:429) — a static int set at apic.c:419 after init completes. It never reads IA32_APIC_BASE. Worse, the two can be actively opposite: apic_init() writes the enable bit into the MSR at apic.c:372 and then returns 0 at apic.c:406 when madt_found is 0 — the OVMF path apic.c:388-399 documents as MEASURED and real. On that path IA32_APIC_BASE[11] is 1 while this row prints "no". The value is honest ("is the local APIC in use"); the register name attached to it is not.

  *What to do:* apic_active() already IS the reading. Rename the register cell to "apic_active()" or "LAPIC SVR[8] + PIC off" and keep the decode "local APIC enabled, not the 8259". If the raw bit is genuinely wanted, that is BACKABLE: one rdmsr(IA32_APIC_BASE) accessor in apic.c plus one streq line in runtime_kernel.c.

- **INVENTED FIGURE** — "processors that answered INIT/STARTUP" prints 1 when no IPI was ever sent
  `kernel/apps/apps_system.zl:173` · backing **READ**
  si_val(2)=smp_n() (apps_system.zl:183) is smp_online() = `ap_online + 1` (smp.c:240) — the +1 is the boot core, which SENT the INIT/STARTUP IPIs rather than answering them, so the decode at apps_system.zl:173 is wrong by construction. The larger problem: smp_start() has exactly one caller in the whole tree — the `smp_go` native (runtime_kernel.c:2204), whose only zl caller is kernel.zl:4484 inside the `if cmd == 42` shell branch (kernel.zl:4472). It is never on the boot path. So on an ordinary desktop boot ap_online is 0, this row prints 1, and the decode claims one processor answered a startup sequence that never ran.

  *What to do:* smp_total() (apic_cpus(), the MADT count, bound at runtime_kernel.c:2206) and smp_n() are both already zl-visible. Honest decode: "cores online: boot core plus APs started" — and since APs are only started by the `*` shell command, the row should read `1 of N` using smp_total() as the denominator, which is what kernel.zl:4486 already prints in the shell.

- The subject is substituted: no display-register table at all, on a premise that is stale
  `kernel/apps/apps_system.zl:153` · backing **READ**
  R.regs prints fourteen Intel display registers (presswork-prototype.html:1477-1491, rendered at :1879-1885). si_draw prints nine cpuid/APIC rows instead. The comment at apps_system.zl:149-157 justifies this: "Printing the reference's 0x0080D9E4 on a machine with no Intel display would be inventing a reading, so the table shows the registers this machine DOES answer." The first half is right and the second half is stale — it conflates "QEMU has no Intel pipe" with "nothing measures it". This tree reads that block extensively. Six of the prototype's fourteen rows are readable from zl TODAY with no new code, and six more need one binding line each. I verified every one against intel.c rather th

  *What to do:* Already bound and callable from zl now: HTOTAL_A -> gpu_ha/gpu_ht, VTOTAL_A -> gpu_va/gpu_vt (runtime_kernel.c:1590-1593, from intel_htotal/hactive/vtotal/vactive intel.c:672-675); PIPE_A_SRCSZ -> intel_w/intel_h (runtime_kernel.c:2284-2285, from intel.c:514-520); PIPE_A_CONF -> intel_pipe/intel_plane (runtime_kernel.c:2292-2293); PP_STATUS 'panel on' -> panel_on (runtime_kernel.c:1610). BACKABLE, one streq line each against an existing intel.c function: PIPE_A_LINK_M/N -> intel_link_m1_reg/inte

- Both buttons missing, and the pane takes no input at all
  `kernel/apps/apps_registry.zl:515` · backing **READ**
  R.regs draws a row with a primary button "re-read block" (data-act=toast, arg=modeset) and a secondary "intel.c" (data-act=open, arg=edit) — presswork-prototype.html:1889-1890. si_draw draws neither. Verified there is no interaction path whatsoever: reg_event (apps_registry.zl:515-541) has no APP_SYSINFO arm and reg_tick (apps_registry.zl:542-551) has none either, so no click or key ever reaches this app.

  *What to do:* "re-read block" has nothing to trigger: si_val() calls its natives inside si_draw, so every repaint is already a fresh read — the honest replacement is a caption saying the table is live, not a button that does nothing. "intel.c" is straightforward: reg_open() (apps_registry.zl:451) already opens windows by id, and the editor is APP_EDIT.

## set (5)

- **INVENTED FIGURE** — "THIS PANE, LAST DRAW" prints the whole compositor frame, not this pane
  `kernel/src/kernel.zl:8618` · backing **READ**
  kernel.zl:8618 does `setpb_v = wm_us()`. `wm_us` binds to `wm_frame_us()` (freestanding/runtime_kernel.c:1707) — the entire frame's body time across every open window. The label above it (kernel.zl:8617) claims a per-window figure, so the number is drawn as a measurement of something nothing measured at that call site. Three things make this the worst item in the pane: (1) the per-window native EXISTS and is bound — `wm_appus` → `wm_win_us(win)` → `wins[win].app_us` (runtime_kernel.c:1729, wm.c:4255); (2) the System Monitor already uses it correctly (`wm_appus(w)` in sm_win_row, kernel.zl:5324ff); (3) the settings window's OWN foot band, drawn a few pixels below this row, prints `W->band_us`

  *What to do:* wm_appus(win) — bound at freestanding/runtime_kernel.c:1729 to wm_win_us(), wm.c:4255, which returns wins[win].app_us

- HANDOFF says this was already fixed; the export landed, the call site did not
  `HANDOFF-APP-PARITY.md:110` · backing **READ**
  HANDOFF-APP-PARITY.md:110 states "`wm_win_us(win)` now exports it" in a section titled "one measurement that existed but could not be read", and kernel.zl:5322 says in the past tense "...so the settings pane printed the whole compositor frame under a per-window label instead". Both read as closed. The native was exported and the System Monitor was wired to it; the settings pane at kernel.zl:8618 was not touched. Anyone reading either note will skip the defect. Verified: `grep -n wm_appus kernel/src/kernel.zl` matches only inside sm_win_row, never in the set_* block.

  *What to do:* same native as above; nothing new needed to make the doc true

- The control count in the handoff is wrong in both numbers: 13 controls, 11 absent
  `HANDOFF-APP-PARITY.md:71` · backing **READ**
  HANDOFF-APP-PARITY.md:71 says "8 of the reference's 12 controls are absent". Counted mechanically over the extracted R.set body (prototype lines 2013-2220): tog/seg/slider call sites total 13 — LADDER 2 (seg 'surface ladder' :2036, slider 'ui scale' :2037), FOCUS 2 (tog 'the knockout' :2083, slider 'focus bar' :2084), BOUNDARY 1 (tog 'occlusion edge' :2144), PRESS 8 (togs :2181-2186, segs :2187-2188). zlOS implements exactly 2 — set_focus_ctl rows 1 and 2 (kernel.zl:7948ff), the only ones with a hit test (settings_event :8765-8772, guarded `if set_tab == SET_FOCUS`) or a key binding (:8797-8799). So 11 of 13 are absent, not 8 of 12. The handoff's own prose in the same sentence enumerates 11

  *What to do:* script over the prototype's R.set body counting tog(/seg(/slider( call sites; and grep for ui_knock/ui_fbar/set_hit_row in settings_event

- LADDER table is 18 of the prototype's 21 rows — three rows are three lines
  `kernel/src/kernel.zl:8396` · backing **READ**
  Diffed mechanically. Present: CUT, WELL, VOID, BASE, RAISE, FLOAT, LITSOFT, LIT, EDGE_OVER, KNOCK, KO_EDGE, TEXT_0..TEXT_3, TEXT_INERT, STEEL, VERM (kernel.zl:8379-8396). Missing against prototype:2041-2049: ZD_KNOCK_INK ('reversed out'), ZD_GRID ('the module grid'), ZD_INK_ON ('on the overprint'). All three already have theme slots declared in this same file — TH_KNOCK_INK = 35 (kernel.zl:106), TH_GRID = 38 (:109), TH_INK_ON = 41 (:112) — and TH_KNOCK_INK is already read by set_focus_kv while TH_GRID is already read by draw_grid (:2044). Three `set_rung_row` lines, no new native, no C change. Cheapest single fix in the pane.

  *What to do:* theme(TH_KNOCK_INK) / theme(TH_GRID) / theme(TH_INK_ON) — all three slots already declared at kernel.zl:106,109,112

- LADDER: the 'smallest step' summary row is absent
  `kernel/src/kernel.zl:8412` · backing **READ**
  prototype:2064-2067 draws a bold summary row — the minimum of the five step ratios, plus '(minStep-1)/0.0728 x the magnitude' against graphite. zlOS's step table (kernel.zl:8398-8412) draws the five steps and then jumps straight to the 'CUT TO BASE, THE WHOLE DOWNWARD BUDGET' row; the smallest-step row is simply not there. The min is the number the entire widening argument is about, and it is free — it is the minimum of the same five ui_ratio() values set_step_row already computes. The 'Nx the magnitude' half is against graphite's 1.0728, which this file deliberately does not quote (see the better-than-reference finding), so print the min alone.

  *What to do:* min of the five ui_ratio(a,b) values already evaluated inside set_step_row

## shell (4)

- The transcript is one ink where the reference is three — and it is the wrong rung
  `kernel/src/graphics/windowing/term.c:695` · backing **BACKABLE**
  term.c:695 draws EVERY scrollback row with `dim`, and kernel.zl:8631 passes theme(TH_TEXT_DIM) for that parameter, which ui.c:157 assigns as ZD_TEXT_3. The prototype uses three: ink1/ZD_TEXT_1 for the command echo lines, ink3/ZD_TEXT_3 for output, k-ok/ZD_OK for the four `pass`/`survived` lines (prototype:1654). Separately from parity, this is a design.h violation in its own right: design.h:282 names ZD_TEXT_1 "BODY - the desktop's root ink" and design.h:284 names ZD_TEXT_3 "labels, column heads, captions". The shell draws its entire body at the caption rung. The typed line is the only text at ZD_TEXT_0 (term.c:730) and it drops to ZD_TEXT_3 the instant Enter commits it, so the echo dims as

  *What to do:* Two of the three inks are free: term_draw already receives fg, dim and accent, and term.c already knows which rows are echoes — term_key writes the literal prefix 'z','l','>',' ' at term.c:232 before echoing. Tag that row (one flag byte per scrollback row, or test the first four chars) and draw echoes in fg, output in dim. The THIRD ink is the part that must not be copied: nothing in this kernel measures whether an output line is a pass, so a green line would be a written claim. The honest third

- **INVENTED FIGURE** — INVENTED FIGURE: `exit 0 · 1.42 s` — neither half is retained anywhere
  `kernel/src/graphics/windowing/term.c:209` · backing **BACKABLE**
  The prototype prints an exit code and an elapsed time (prototype:1662). zlOS retains neither. term_complete (term.c:209-213) takes `result` and does nothing with it but emit a zlt_event; there is no static, no accessor, no native. term_submit (term.c:203-207) records no timestamp. So both numbers would have to be written. `1.42 s` is additionally impossible as a constant: it is a per-command figure and the prototype's is fixed.

  *What to do:* Two statics in term.c plus two accessors plus two lines in runtime_kernel.c. `run_command`'s return value already reaches term_complete (kernel.zl:8749-8753), and `ticks()` is a bound native (idt_ticks, PIT at 100 Hz) — stamp it in term_submit, subtract in term_complete. For sub-tick resolution `cpu_tsc32`/`cpu_khz` are also bound. Until that lands the row should print the exit code alone, or nothing.

- **INVENTED FIGURE** — INVENTED FIGURE: `text 218432  data 30720  bss 1441792`
  `kernel/boot/link.ld:29` · backing **BACKABLE**
  The prototype's LD line prints three section sizes (prototype:1633). No native exposes them and no linker symbol makes them reachable: link.ld (kernel/boot/link.ld) defines exactly one symbol, __kernel_end at :29, and no per-section boundaries. Copying these three numbers would be writing three figures about a binary this kernel cannot inspect.

  *What to do:* link-raw.ld ALREADY defines __bss_start and __bss_end (link-raw.ld:28 and :31) — the raw-boot entry uses them to zero BSS by hand. Add the same pair plus _etext/_edata to link.ld, declare them extern, and bind three natives. Until then the honest LD line is the one figure that IS bound: `arena_cap`/`arena_used`/`heap_used` are all natives, and __kernel_end is what the two ASSERTs at link.ld:30-32 already police.

- **INVENTED FIGURE** — `pixel clock 241690 kHz -> 59.998 Hz` — the kHz is READ, the Hz reads zero on an idle panel
  `kernel/src/drivers/display/intel.c:802` · backing **BACKABLE**
  Half of this line is the best figure in the whole renderer and half is a trap. `gpu_clk` is bound to intel_pixel_clock_khz() (runtime_kernel.c:1595), which derives the pixel clock from the transcoder's LINK_M/N ratio and returns 241690 kHz on this panel — intel.c:743-748 records it verified against the panel's own EDID descriptor to the kHz. The refresh side is the trap: `gpu_hz` is bound to intel_refresh_mhz() (runtime_kernel.c:1594), the frame-COUNTER measurement, and intel.c:722-731 states outright that firmware leaves PSR on on this laptop so it returns 0 with an idle screen and a correct 60.0 Hz with a terminal scrolling. A shell that prints gpu_hz will print 0.000 Hz most of the time a

  *What to do:* intel_refresh_mhz_derived() at intel.c:802 is the PSR-proof answer and is NOT bound to zl — one line in runtime_kernel.c. Or compute it in zl from three already-bound natives: gpu_clk, gpu_ht, gpu_vt, as (clk_khz * 1000000) / (ht * vt) in milli-hertz, which is integer arithmetic and needs no floats. Also note both return 0 on QEMU/BGA/virtio-gpu, so the line needs a not-measured branch, not a fallback constant.

## sys (5)

- **INVENTED FIGURE** — "logical, hyperthreaded" is a written claim; cpu_tpc() is bound and measures it
  `/home/roy/Documents/repos/zl-linux-presswork/kernel/src/kernel.zl:5540` · backing **READ**
  kernel.zl:5540 draws the CPU card as sys_card(..., "cpu", cpu_thr(), 0, "logical, hyperthreaded"). The VALUE is read (cpu_thr -> cpu_threads(), CPUID leaf 0x0B core level, kernel/src/arch/x86/cpu.c:177). The CAPTION is a string literal that asserts SMT unconditionally. On any non-SMT part cpu_threads_per_core() returns 1 (cpu.c:192-200) and the card states a hardware feature the machine does not have, on a pane whose whole thesis is that its figures are read. The prototype's caption at presswork-prototype.html:1940 is "logical, 4 physical" — a second FIGURE, not an adjective. Both numbers behind it are already bound and already used elsewhere in zl: kernel.zl:4523-4525 prints cpu_cores()/cpu

  *What to do:* cpu_tpc -> cpu_threads_per_core(), bound at freestanding/runtime_kernel.c:2242, defined kernel/src/arch/x86/cpu.c:192. cpu_cores -> cpu_cores(), bound at runtime_kernel.c:2240, cpu.c:202. Minimal honest fix (one-line calls, no new native): `if cpu_tpc() > 1 { sys_card(cx, cy, sd_w, sd_h, "cpu", cpu_thr(), 0, "logical, hyperthreaded") }` / `if cpu_tpc() == 1 { sys_card(cx, cy, sd_w, sd_h, "cpu", cpu_thr(), 0, "logical, one per core") }` — sys_card is a zl fn with 8 params, so the 8-arg NATIVE cap

- The display row prints a boolean where the pipe identity belongs — "pipe 1"
  `/home/roy/Documents/repos/zl-linux-presswork/kernel/src/kernel.zl:5627` · backing **BACKABLE**
  kernel.zl:5626-5627 draws `"intel gen9, pipe "` then sys_vn(..., intel_pipe(), ...). The native `intel_pipe` is bound to intel_pipe_enabled() (freestanding/runtime_kernel.c:2292), which is a 0/1 predicate — intel.c:548-552 reads bit 31 of TRANS_CONF and returns `? 1 : 0`. So the pane prints "intel gen9, pipe 1" meaning "the transcoder is enabled", and "intel gen9, pipe 0" when it is not — which reads as a real pipe name. The prototype says "pipe A" (presswork-prototype.html:1945). The correct reader exists and was missed because of the name collision: intel_pipe_current() at kernel/src/drivers/display/intel.c:133 returns cur_pipe (PIPE_A=0/PIPE_B=1/PIPE_C=2) and is NOT bound. Verified the dr

  *What to do:* Bind intel_pipe_current() (intel.c:133) as e.g. `intel_pipecur` — one line beside runtime_kernel.c:2292. zl has no string values, so the letter is three one-line branches: `if intel_pipecur() == 0 { sv_x = sys_vt(sv_x, vy, "pipe A, ", dr) }` and the same for B/C. Keep intel_pipe() but move it to what it actually says — it is the honest source for a separate "transcoder enabled/stopped" reading.

- PASS/FAIL grades the rounded printout, not the measurement
  `/home/roy/Documents/repos/zl-linux-presswork/kernel/src/kernel.zl:5740` · backing **READ**
  The verdict test is `(sg_q + 50) / 100 >= sg_fl` (kernel.zl:5740) and the same expression drives the head badge (kernel.zl:5495). `/` in the kernel runtime is integer division — zl_binop casts both operands to long long at freestanding/runtime_kernel.c:1264-1269 (note kernel.zl:34-54's comment saying `/` is float division describes ./interp, the HOST interpreter, not the kernel build). So (q4+50)/100 is exactly the two-decimal value pw_rat prints (kernel.zl:5346-5351), and the verdict is computed from the DISPLAY value rather than the ratio. A pair measuring 4.4951:1 has q4=44951, rounds to 450, and is marked PASS against a 450 floor — WCAG AA requires >= 4.5 and the prototype's `v >= t[3]`

  *What to do:* ui_ratio already returns the unrounded q4 (ui_ratio_q4, bound runtime_kernel.c:1771). Compare at full precision: `if sg_q >= sg_fl * 100` at kernel.zl:5740 and `if sg_q < sf_fl * 100` at kernel.zl:5495. Keep printing the rounded value — a row reading "4.50:1 FAIL" is the honest output and is exactly what the prototype draws.

- **INVENTED FIGURE** — The framebuffer stride is computed as width*4, never read from the pitch the compositor writes with
  `/home/roy/Documents/repos/zl-linux-presswork/kernel/src/kernel.zl:5645` · backing **BACKABLE**
  kernel.zl:5645 draws the stride as sys_vn(..., px_w() * 4, ...) under the label "stride ... B". The comment above it (kernel.zl:5641-5642) claims "both figures derived from the mode rather than from a constant" — derived from the WIDTH, which is not the stride. The real scanline pitch is fb_pitch (fb.c:102, set from the caller at fb.c:819) and the UEFI path takes it from GOP's pixels_per_scan_line (kernel/boot/efi.c:767, 783), whose validation at efi.c:772 rejects only `pitch_pixels < horizontal_resolution` — a padded scanline is explicitly legal and common on real firmware. On any such machine this row prints a number that is not the stride. The same assumption is in ruler_fb() (kernel.zl:2

  *What to do:* fb_pitch_bytes() already exists as a pure reader at kernel/src/graphics/framebuffer/fb.c:136 and is not in the native table (checked all 773 streq(name,...) bindings in freestanding/runtime_kernel.c). Bind it, then use it at kernel.zl:5645 and inside ruler_fb(). For bpp, add a matching reader for fb_bpp; until then "x 32" should not be drawn as if measured.

- The ladder row reports the knockout switch under the ladder key
  `/home/roy/Documents/repos/zl-linux-presswork/kernel/src/kernel.zl:5691` · backing **READ**
  presswork-prototype.html:1950-1952 switches this row on S.cfg.ladder: "presswork, widened. smallest surface step N:1" versus "RAKING, the comparison ladder. smallest step N:1". kernel.zl:5691-5692 switches on ui_knock_on() instead and prints "presswork, widened, knockout on" / "presswork, KNOCKOUT OFF" under the key "ladder" (kernel.zl:5609). Two different controls: the prototype's ladder segmented control is a separate thing from its knockout toggle, and the zl SETTINGS pane has the knockout but not the ladder control. So "presswork, widened" is a literal no read gates — true today only because the raking ladder does not exist in this image, and a lie the moment somebody lands the segmented

  *What to do:* ui_knock_on -> ui_knockout_get() is bound (freestanding/runtime_kernel.c:1778) and is the right read — it is under the wrong key. Move the knockout state to its own row or to the SETTINGS pane, and let the LADDER row carry only what is measured: the smallest surface step. Do not add a "presswork"/"raking" word until a ladder selector exists and a native reports which one resolved; nothing does today.

## type (6)

- ROLE column reads the abandoned role ladder, so the pane marks its own LG face '-'
  `/home/roy/Documents/repos/zl-linux-presswork/kernel/src/kernel.zl:6202` · backing **BACKABLE**
  ty_roles asks atlas_role(r,0) = ui_atlas_for_role (ui.c:772), whose wanted height is fb_text_role_h(role) (ui.c:774) = prop_cell (fb.c:3358) = role_base{8,12,16} scaled and floored at 12, i.e. 12/12/16 at ui 1. Nearest linked sans cell for all three is the 16px prop16 (ui.c table row 3). But uikit.c stopped asking for roles — 'So the roles are not asked for any more' (uikit.c:234) — and ui_text/ui_text_tracked pass ui_text_h(size) (uikit.c:266) = ZD_T_*/2 = 11/13/21 into fb_text_rich, which resolves prop_atlas_cell(want) (fb.c:3368: <=20 -> 16, <=28 -> 24, else 32) to 16/16/24. So LG genuinely draws out of prop24/prop24b (table rows 5/6) while the ROLE column puts SM, MD and LG all on the pr

  *What to do:* One line: ui.c:774 `int want = fb_text_role_h(role);` -> `int want = ui_text_h(role);`. The existing nearest-|cell-want| search then reproduces prop_atlas_cell exactly (11->16, 13->16, 21->24), so no new native and no second copy of fb.c's midpoint rule. uitest.c:1384 WANT[3] must move from {12,16,22} to {11,13,21} with it.

- The host test that 'proves' the ROLE column stubs a role ladder fb.c does not have
  `/home/roy/Documents/repos/zl-linux-presswork/kernel/tests/host/uitest.c:95` · backing **READ**
  ROLE_H[3] = {12,16,22} and fb_text_role_h is stubbed to return it (uitest.c:101). fb.c's real prop_cell returns 12/12/16 at ui 1 (fb.c:3356-3364). uitest.c:1407 asserts '...and LG does not, so the pane has two faces to report, not one' — true of the stub's 22, false of the kernel's 16. The green suite therefore cannot see the defect in finding 1; it asserts a property of its own table. Same shape as the 'tests that restate constants' failure this repo has hit before.

  *What to do:* fb_text_role_h's real arithmetic is three lines (fb.c:3358-3364) and depends only on ui_scale_q8, which the host test already sets via ui_theme_init_q8. Make the stub compute role_base{8,12,16}*scale floored at 12 instead of a hand table — or, after finding 1, assert against ui_text_h and delete ROLE_H from the atlas block entirely.

- The `hierarchy` statement row is unreachable — 3 of the 4 are drawn
  `/home/roy/Documents/repos/zl-linux-presswork/kernel/src/kernel.zl:6354` · backing **READ**
  TY_ROWS_N = 4 (kernel.zl:6280) and the loop runs tb_i = 1,2,3, so it draws sizes / styles / tracking only. ty_rowk_name's fall-through 'hierarchy' (6286) and ty_row_val's fall-through 'case, weight and rule weight. never size' (6318) are both dead code: index 0 is consumed by the FACES right-hand run (6291-6293), so no index ever reaches the else. The prototype draws kv('hierarchy', 'case, weight and rule weight. never size') as its fourth statement (prototype:1911). Both of the pane's own comments say four ('the four statements', 6270 and 6348).

  *What to do:* Nothing to measure: it is a design statement, not a figure. Renumber hierarchy to index 4 and set TY_ROWS_N = 5. Costs no layout budget — the space guard at kernel.zl:6351 already reserves 4 * pw_kvh().

- **INVENTED FIGURE** — SIZES compares design.h's tokens against themselves and hand-quotes 11 / 13 / 21
  `/home/roy/Documents/repos/zl-linux-presswork/kernel/src/kernel.zl:6302` · backing **BACKABLE**
  The 'live' half (kernel.zl:6297-6301) is ui_th() = ui_text_h (uikit.c:266) = UI_DP(T, type_x2[role])/2, and type_x2 is {ZD_T_SM, ZD_T_MD, ZD_T_LG} = {22,26,42} (design.h:541-543) — exactly 11/13/21 at ui 1, which uikit.c:239-241 states as measured. The right half ' px. design.h asks 11 / 13 / 21' is a typed literal of those same three tokens. So the row reads a token, then quotes the same token, and presents the pair as a gap. Worse, the comment at kernel.zl:6275-6279 claims the row exposes the 12/12/16 role-ladder collapse — that is fb_text_role_h, which this row never calls. Do not trust that comment: it asserts the opposite of what the code reads.

  *What to do:* Make the right-hand figure READ instead of typed: expose type_x2[role]/2 unscaled through a native (uikit.c:264 is already indexed by the same 0/1/2 that PW_SM/MD/LG use, kernel.zl:5462-5464) — one native, 8-arg budget untouched. And print the number that actually differs: ui_th(role) px beside atlas_h(atlas_role(role,0)) px, both read, which is the register-vs-atlas gap the comment wanted.

- Section band says 'linked' over a row that is not linked, and prints 9 beside the 8 one line above
  `/home/roy/Documents/repos/zl-linux-presswork/kernel/src/kernel.zl:6344` · backing **READ**
  pw_sect(tb_x, ..., 'every atlas linked into this image', atlas_n(), ' rows') renders 'EVERY ATLAS LINKED INTO THIS IMAGE ....... 9 rows'. Table row 2 has in = 0 (ui.c:735, the 24x48 font_big.c that is not in kernel/SOURCES) and ty_rows draws it ABSENT (kernel.zl:6262). One row above, FACES prints ty_present() = 8 'atlases in this image' (kernel.zl:6292-6293). ty_present's own comment (kernel.zl:6105-6108) warns against precisely this: a count 'printed over a table with an ABSENT row in it would be the pane contradicting itself in two adjacent lines'. The count is honest ('9 rows'); the word 'linked' is the false part.

  *What to do:* Wording only — both atlas_n() and ty_present() are already read. Either 'every atlas this image describes' with atlas_n(), or keep 'linked' and pass ty_present().

- SIZE column drops the unit and repeats the number already in the ATLAS column
  `/home/roy/Documents/repos/zl-linux-presswork/kernel/src/kernel.zl:6253` · backing **READ**
  pw_dec(..., atlas_h(i) * 10, 1, ...) draws digits, '.', digits and nothing else — pw_dec_run (kernel.zl:5541-5567) emits no unit. So the column reads '16.0', '32.0', '48.0'. The prototype's cell is '11.0 px' (prototype:1895-1897) and its SIZE means the register's rendered size, not the atlas cell. Here atlas_h was already drawn a few glyphs to the left in the ATLAS column ('16 x 16', kernel.zl:6245), so the column is a second unitless copy of a number the reader already has. The comment at kernel.zl:6250-6252 defends this as 'the size it is actually baked at' — but that is what column 0 already says.

  *What to do:* Append ' px' (one sys_vs-style run, one call, fits on a line). Better: make the column the DEVICE height the register on that row resolves to — ui_th(role) is already a native and is already called at kernel.zl:6297-6301, so the column becomes the one reading that is not duplicated anywhere else on the pane.
