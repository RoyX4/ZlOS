> **DATED SEQUENCING ANALYSIS — NOT THE CURRENT QUEUE.** This was written
> against `desktop/overnight-compositor` on 2026-08-18. Fleet revalidation later
> established that Wave 0 had already landed. Keep the dependency reasoning, but
> use [`docs/PROJECT-STATUS.md`](../../../docs/PROJECT-STATUS.md) and
> [`docs/REMAINING-WORK.md`](../../../docs/REMAINING-WORK.md) for current state.

Verified the load-bearing claims against the source before ordering anything. Here's the plan.

---

# zlOS driver build order — dependency-first

Written against the tree at `desktop/overnight-compositor`, 2026-08-18. Every claim below that carries weight was re-checked against the source, not taken from the specs.

---

## 0. The heap question, answered first

**The heap does not gate this list. Do not build it first.**

I checked: `grep` for `malloc|kmalloc|calloc|free(` across `kernel/*.c` returns two hits, both `fb_blur_free` — a slot recycler, not an allocator. The no-heap claim is true. But go through the 33 specs and ask which one *needs* dynamic allocation:

| Spec | What it needs | Heap? |
|---|---|---|
| hda | one 128-byte-aligned DMA block | fixed address |
| eth / e1000 | ~1.05 MiB of rings + buffers | fixed address |
| wifi | 1.4 MB firmware image + rings | fixed address |
| fat32 | one sector buf, one cluster buf, one FAT window | static |
| cache.c | fixed set-associative arena | **its own spec says no-heap forces the right design** |
| tpm / rtc / acpi / entropy / gpt / pm | nothing | static |

Zero of them. The heap spec is honest about what it actually gates — "a document browser's DOM, TLS bignums and X.509 parsing, an ELF loader, hosting zl's own interpreter" — and none of that is on this board.

**What *is* worth taking from that spec is the smaller half: the physical memory map plus a reservation table.** zlOS currently does not know how much RAM exists (`mem_upper`/`mmap_addr` are declared in `console.c:112,117` and never read), so every `*_ram_ok()` probe is a guess, and there are 33 hardcoded bases across six files whose only collision defence is a comment. That is Wave 1, and it is ~200 lines, not 560.

Concrete evidence the collision risk is real, re-derived just now:

```
fb.c:135   HI_BLUR   0x0C000000      fb.c:139  BLUR_LIMIT = HI_NVME - HI_BLUR = 16 MiB
fb.c:1747  static unsigned int *arena_next = (unsigned int *)HI_BLUR;   /* live bump allocator */
i2c_hid.c:104  HID_BUF      0x0C900000
i2c_hid.c:239  HID_DESC_BUF 0x0C900100
```

The touchpad's report buffer sits 9 MiB inside the blur arena. `fb.c`'s own map comment lists six owners and omits `i2c_hid.c`. The three `_Static_assert`s check base ordering only, not span containment, so they cannot catch it.

Correction to the survey on this point: it called the collision "latent, no code reads or writes the blur arena yet." That is *nearly* right — `blur_alloc`/`blur_slot` capture are fully implemented (fb.c:1747-1977), but `grep` for `fb_blur_capture|fb_blur_draw|blur_alloc` outside fb.c returns **nothing**. So it is latent in exactly the same way `lt_armed` is latent: one caller away. A full-screen blur at 2560x1440x4 is 14.7 MB, which walks straight through `HID_BUF`. It will present as "the touchpad started returning garbage after we added window blur."

---

## 1. What actually blocks the most

Not a driver. Five edits to shared files, ~150 lines total, unblocking roughly a dozen downstream items:

| Edit | Lines | Unblocks |
|---|---|---|
| `acpi_find_table()` → external linkage (`apic.c:171` drops `static`) | **1 word** | TPM2, HPET, FADT (reboot/poweroff), ECDT (battery), MCFG (ECAM), LPIT |
| `pci.c` capability-list walk + PMCSR D3→D0 | ~60 | eth, hda, lpss_i2c — all three are **in D3hot right now** |
| `xhci.c` `event_wait()` slot+dci filter; `reset_endpoint()` ring parameter | ~40 | bluetooth, MSC correctness, USB mouse. **Two confirmed live bugs** |
| `write_msr()` in `cpu.c` (does not exist anywhere in the tree) | ~3 | all power-management and RAPL work |
| CRC32 (poly 0xEDB88320) | ~25 | GPT parsing |

The single highest leverage-per-line item in the entire survey is deleting the word `static` from one line of `apic.c`.

---

## Wave 0 — unblock layer

Everything here is an edit to a file that already exists. None of it is a new driver.

### 0.0 — Display Phase 0.1 comes first, and it is not on this list

`grep` confirms: nothing outside `hosttest/dpll_test.c` calls `intel_link_train_arm`. The cold-start modeset works on hardware and zlOS still cannot light its own panel. `kernel/HANDOFF.md` and `kernel/docs/archive/superseded/display-roadmap.md` both name this as the point of the project, and at least six specs in this survey independently say "do not let this jump the queue." **Finish 0.1 before anything below.** Wave 0's items are small enough to land in the gaps around it; Waves 1+ are not.

### 0.1 — `acpi_find_table()` external linkage
- **Why now:** six downstream items are gated on one keyword.
- **Unblocks:** acpi.c, rtc, hpet, tpm, battery, ECAM in pci.c.
- **Size:** 1 word + a prototype.
- **Biggest risk:** none. This is free.

### 0.2 — `pci.c`: capability walk + power management
- **Why now:** measured on this machine right now — eth `00:1f.6` PMCSR `0x210b` (D3hot), hda `00:1f.3` PMCSR `0x010b` (D3hot), lpss_i2c `00:15.0` PMCSR `0x0b` (D3hot). `pci_enable()` only ORs into COMMAND; it cannot wake any of them. MMIO on a D3 device returns `0xFFFFFFFF`, which every driver will read as "every status bit set" or "device absent."
- **Unblocks:** eth, hda, i2c_hid, and every future PCI driver.
- **Size:** ~60 lines (walk from 0x34, find cap ID 0x01, clear PMCSR[1:0], wait 10 ms).
- **Biggest risk:** this is the silent-failure landmine of the whole survey. Without it a working driver looks like missing hardware, and it **will not reproduce on a cold boot** — only on a warm boot out of Linux. Fix it in `pci.c` as a shared function; do not patch it per-caller.

### 0.3 — `xhci.c`: two confirmed bugs

Read out of the source, not inherited:

```c
/* xhci.c:604 */
static int event_wait(int want, u32 *param, u32 *status, u32 *ctrl, int spins)
    ...
    if (t == want) {          /* matches on TYPE only — no slot, no endpoint */
```

```c
/* xhci.c:940 */
static int reset_endpoint(int slot, int dci)
    ...
    u32 ring = EP0_RING(slot);   /* hardcoded, regardless of dci */
    ring_init(ring);
    ep0_enqueue[slot] = 0;
    ...
    cmd_submit((u64)(ring | 1u), 0, 16 /* Set TR Dequeue Ptr */, (slot<<24)|(dci<<16));
```

`reset_endpoint` is already called with the bulk DCI at line 1848 — it aims the bulk endpoint at EP0's ring and wipes EP0 underneath it. That is live, not hypothetical. `cmd_wait()` five lines above `event_wait` shows the correct filtering shape.

- **Why now:** these are the shared functions the keyboard, pointer, mass-storage, and any future Bluetooth path all run through. CLAUDE.md's rule applies literally: fix the shared function once.
- **Unblocks:** safe MSC stall recovery, USB mouse, Bluetooth.
- **Size:** ~40 lines.
- **Biggest risk:** the fix changes behaviour on paths that currently pass their gates by luck. Run `exercise.py` and the `probe-*.py` set before and after, and diff.

### 0.4 — `write_msr()` and MONITOR/MWAIT intrinsics in `cpu.c`
- **Why now:** `read_msr()` exists at `cpu.c:48`; there is no writer anywhere in the tree.
- **Unblocks:** all of Wave 2's power work.
- **Size:** ~10 lines of inline asm, same pattern as `do_cpuid`.
- **Biggest risk:** none by itself. The risk lives in what calls it (see 2.5).

### 0.5 — CRC32
- **Why now:** GPT needs it, nothing else provides it. `crypto.c` is SHA/HMAC/AES only.
- **Unblocks:** gpt.c.
- **Size:** ~25 lines, self-testing (golden values already measured: `0x6840E869` over this disk's 92-byte GPT header, `0xF81D13B8` over its 16384-byte entry array).
- **Biggest risk:** scope. Header CRC is over `HeaderSize` (92) bytes, not the 512-byte block; array CRC is over all 128 entries including zeroed ones. Wrong scope still produces a number.

---

## Wave 1 — physical memory map + reservation table

Not `kmalloc`. Parse the memory map, build a table of the 33 existing fixed bases, make every `*_ram_ok()` honest, and assert span containment so the blur/HID collision cannot recur.

- **Why now:** it converts the project's #1 recurring bug class ("a DMA buffer outside guest RAM, or an address truncated to 32 bits" — five occurrences logged) from a runtime mystery into a boot-time assert. Every driver in Waves 2-4 wants a new arena; this is what makes picking one safe.
- **Unblocks:** every subsequent driver's DMA region, honestly. Also unblocks display-roadmap 0.1's "a framebuffer in stolen memory."
- **Size:** ~200 lines (EFI descriptor walk using firmware's `desc_size`, E820 fallback, reservation table, span asserts). Cap at 4 GiB.
- **Biggest risk:** scope creep into `boot64.S`. This machine has 14,344 MiB above 4 GiB and the identity map is exactly 2048 × 2 MiB = 0-4 GiB. **Do not extend it in this change.** 1701 MiB below 4 GiB is 13× what zlOS uses today. Page-table work in the boot path is where "unbootable machine with no diagnostic" lives, and this repo has already lost the 64-bit boot once inside `setup_idt()` with all three gates green.

Also: do not add a lock. There are no atomic primitives in the tree, the scheduler is cooperative, and every AP is parked in `hlt`. Write `if (cpu_id() != 0) kfatal("allocator entered from an AP")` instead — that assert becomes the tripwire the day an AP gets real work.

---

## Wave 2 — cheap wins, hardware present, QEMU-or-Linux verifiable

Each item here is self-contained and none blocks another. Ordered by value-per-line.

### 2.1 — `acpi.c` + shutdown/reboot
- **Why now:** zlOS has no power-off at all (`kernel_done()` is `for(;;) hlt`) and its only reboot is an 8042 `0xFE` pulse whose validity this board's own FADT contradicts (`IAPC_BOOT_ARCH` bit 1 = 0). Firmware's declared reset is I/O `0xCF9` value `0x06`. Power-off is one 16-bit OUT: `outw(0x1804, (inw(0x1804) & 0xC3FF) | 0x3C00)` — SLP_TYP 7 read byte-for-byte out of this DSDT's `_S5_` at offset 136981.
- **Unblocks:** HPET, TPM, battery, ECAM, and the whole reboot-and-look-at-the-screen dev loop.
- **Size:** ~550 for a full table layer, ~260 if you only want reboot + poweroff. Do the small one first.
- **Biggest risk:** read-modify-write PM1a_CNT. Writing a bare `0x3C00` clears SCI_EN in the same store and drops out of ACPI mode instead of powering off. Mask is `0xC3FF`.

### 2.2 — `rtc.c` (CMOS)
- **Why now:** zlOS has no absolute time. Every file timestamp, log ordering across reboots, and eventually TLS cert validity needs it. This machine's HPET is force-disabled, so RTC is also the only hardware periodic source besides the PIT.
- **Unblocks:** timestamps, a desktop clock, log ordering.
- **Size:** ~350.
- **Biggest risk:** status register C is read-to-clear. Miss it in the IRQ8 handler and you get exactly one interrupt, then permanent silence, with no error anywhere. Second: this machine's RTC holds **local** time, measured +36000 s vs UTC. Nothing in hardware says so — make it an explicit setting.

### 2.3 — battery + AC over the EC
- **Why now:** a laptop OS that cannot say it is about to die is a toy. And this is the rare driver that in its minimal form writes **zero bytes to hardware** — AC state, charge %, voltage and power all come from reads.
- **Unblocks:** nothing, but it is the first non-display subsystem verifiable against a number the machine already publishes.
- **Size:** ~280.
- **Biggest risk:** `_BST` state-bit polarity. bit0 = DISCHARGING, bit1 = CHARGING. The survey got this backwards on first pass and concluded the machine was charging while it was visibly discharging. Fails plausible, not loud. Second: **never write any EC offset except 0x81.** The same 256-byte space holds fan control (0x2F) and thermal trip points (0x2B-0x2E).

### 2.4 — entropy (`entropy.c`)
- **Why now:** `crypto.c` is 543 lines of vector-tested SHA/HMAC/AES/PBKDF2 written explicitly for "WPA2, Bluetooth pairing and TLS", and the kernel cannot produce one unpredictable byte. It is a complete engine with no fuel. This CPU has both `rdrand` and `rdseed`.
- **Unblocks:** the entire wireless plan, any TLS, Bluetooth SSP.
- **Size:** ~600 (RDSEED seed → HMAC-DRBG-SHA256 on the existing `hmac_sha256`, plus SP 800-90B health tests). Testable in `hosttest/` against published CAVP vectors with no hardware at all.
- **Biggest risk:** SRBDS is active on this machine (`srbds: Mitigation: Microcode`) and serializes RDRAND across all 8 logical CPUs. Two APs in a 10-deep RDRAND retry loop is a cross-core stall that looks like a lockup. Draw a seed once, run the DRBG for everything else, never put RDRAND in a per-call path — and never inside `intel.c`'s timed panel sequence.

### 2.5 — MSR power/thermal telemetry (replaces both `pm` and `intel_thermal`)
- **Why now:** these are the same MSRs and the `intel_thermal` spec's own recommendation is to skip the PCI driver entirely. `cpu_temp_c()` (`cpu.c:349`) already does the temperature half. Adding RAPL package energy (0x611 / 0x606) gives "CPU is drawing 2.05 W" in the System Monitor for ~60 lines, no BAR, no MMIO, no PCI enable.
- **Unblocks:** thermal-aware behaviour later; a System Monitor that shows something real.
- **Size:** ~60 read-only. The C-state/P-state half is ~550 and belongs later.
- **Biggest risk:** **ship it read-only.** `IA32_PM_ENABLE` bit 0 is write-once — enable HWP during bring-up and `IA32_PERF_CTL` is silently ignored forever, with no reset short of a power cycle. And `MSR_PKG_POWER_LIMIT` on a 15 W part in this chassis, in an OS with no thermal governor and no fan control, is hardware-damage-adjacent. Read RAPL. Never write it.
  Secondary: the energy counter is 32-bit and wraps every ~35 h at idle, ~86 min at PL2. Subtract as unsigned 32-bit.

### 2.6 — `gpt.c`
- **Why now:** turns `nvme_read_block()` from "sector N of the raw disk" into "sector N of a partition." It is also the bounds check that stops a future FAT32 writer from destroying the GPT or the ext4 root on the test laptop.
- **Unblocks:** fat32, and safe writes of any kind.
- **Size:** ~600 including MBR/EBR.
- **Biggest risk:** this disk's array has a **hole** (entry 2 zero, 3-6 live) and is **not in LBA order** (entry 3 starts at 998463488, entry 4 at 444416). A parser that stops at the first zero entry finds 2 of 7 partitions and reports success. Also: `BPB_HiddSec` on `/dev/nvme0n1p5` reads 834,623,488 against a real start of 444,416 — never derive the partition base from the BPB.

### 2.7 — `serial.c` consolidation
- **Why now:** four independent copies of the TX poll loop exist, two of which never consult `ser_present()`, and `serial_init()` has exactly one caller (`efi.c:285`) so the BIOS-multiboot and raw_boot paths never program the divisor. Every gate in the repo runs over this wire.
- **Unblocks:** nothing, but it removes a live hazard.
- **Size:** ~280 (mostly deletion).
- **Biggest risk:** **there is no UART on this laptop and the firmware tables lie about it.** DBG2 and DBGP both declare a 16550 at 0x3F8; the PCH is actually decoding 0x3F8-0x3FF (COMA_EN set) and nothing answers, so reads return `0xFF` — LSR bit 0 "data ready" true forever, RBR returning `0xFF` forever, an infinite stream of phantom keystrokes into the input queue, on the one machine with no serial console to debug it with. `ser_present()` guards this; `zl_putc()` and `kernel_done()` do not. Second: any change to the TX path changes `verify.sh`'s golden transcript. One commit, regenerate, diff deliberately.

---

## Wave 3 — storage stack

Sequential. Each depends on the previous.

### 3.1 — block-device abstraction + NVMe multi-block
- **Why now:** `io_one()` sends exactly one 512-byte block per command into one fixed page, and PRP2 is never constructed. Measured on this drive: 44.4 µs per command at 512 B (11.5 MB/s) versus 137.6 µs at 128 KiB (953 MB/s). **A command costs ~40 µs regardless of size.** 32 separate 4 KiB commands = 1197 µs; one 128 KiB command = 138 µs. Everything downstream inherits that 85× ceiling until this changes.
- **Unblocks:** fat32, cache.c, and makes both worth building.
- **Size:** ~150 for multi-block + PRP lists, ~60 for the abstraction over nvme/msc.
- **Biggest risk:** PRP2 alignment. PRP1 may be 4-byte aligned but PRP2 and every PRP-list entry must be 4 KiB-aligned, and the list must not cross a page boundary. Get it wrong and the drive DMAs into the wrong physical address — on an identity map, over whatever else lives there.

### 3.2 — FAT32 read
- **Why now:** `fs_save`/`fs_load` in `kernel.zl` is 10 RAM slots at 0x02010000 that die at reboot. zlOS boots from a FAT32 ESP it cannot read. This is what makes the editor real.
- **Unblocks:** persistence, loading `.zl` source at runtime, eventually self-update.
- **Size:** ~800 read-only (BPB, FAT chain, LFN directory entries).
- **Biggest risk:** the EOC test. `/boot/efi`'s live FAT has `FAT[2] = 0x0FFFFFF8` and `FAT[3] = 0x0FFFFFFF` — **two different end-of-chain values in one FAT.** `if (ent == 0x0FFFFFFF)` walks off the end of the root directory into free space. The test is `(ent & 0x0FFFFFFF) >= 0x0FFFFFF8`. Fails silently, looks like directory corruption.
- **Also worth correcting in the repo:** `kernel/docs/archive/superseded/feature-catalogue-2026-08-17.md:301-303` lists filesystems as "not worth taking" because they "need a heap." FAT32 read is a counterexample — one sector buffer, one cluster buffer, one FAT window, all static. Amend that line when this lands.

### 3.3 — FAT32 write, then `cache.c`
- **Why now:** write-back is 15× faster than write-through at 4 KiB on this drive (40.9 µs vs 625.7 µs; a FLUSH costs ~585 µs). And `IO_FLUSH` is defined at `nvme.c:97` and **grep finds exactly one occurrence — the definition.** The drive's `write_cache = "write back"`, so every write zlOS makes today can already vanish on power loss. It reads back fine because the read is served from the same cache.
- **Unblocks:** a filesystem that survives a power cut.
- **Size:** ~600 fat32 write, ~600 cache.
- **Biggest risk:** `atomic_write_unit_max_bytes = 512` — only one sector is power-fail atomic, and this drive has logged **142 unsafe shutdowns**. A 4 KiB cache line written back as one command can land as any mixture of its eight sectors. Do not design a cache that assumes line-granular atomicity. Second, and larger: **never let the ThinkPad's internal NVMe be a write target** until 3.1's bounds checks are in and proven. LBA 0-33 is the GPT, 2445312 is the ext4 root that holds this repo. Develop in QEMU against `/tmp/zlos-nvme.img` and on the host harness.

---

## Wave 4 — present hardware, harder, still verifiable

### 4.1 — `lpss_i2c` / finish `i2c_hid.c`
- **Why now:** it is the only path to the touchpad, `i2c_hid.c` already exists at 315 lines, and it has never executed. The fixes are concrete: SCL timing is wrong by 2.7× (the code writes HCNT 60 / LCNT 130 against a **measured 216 MHz** source clock, giving ~1.09 MHz on a bus firmware declares as 400 kHz; correct values are 191 / 345), the LPSS shim is never taken out of reset, and D3hot is never cleared.
- **Unblocks:** the laptop's touchpad.
- **Size:** ~250 (a fix to the bottom third, not a new file).
- **Biggest risk:** `i2c_find()` selects on class 0x0C / subclass 0x80 and **calls `pci_enable(i)` before any identity check.** On this machine the third match is `00:1f.5`, the **PCH SPI flash controller**. It lands on the right device by scan-order luck only. Match on device ID 0x02e8/0x02e9. Never on class.
  Secondary: no test loop exists. QEMU has no LPSS I2C and the hosttest trick does not transfer — reaching BAR0 from Linux needs an unbind that kills the trackpad on the machine you are working on. Budget for boot-the-laptop-or-nothing, and make the first milestone "prints VID 0x06CB PID 0xCD8B", not "cursor moves".

### 4.2 — `hda.c` analog half only
- **Why now:** ~800 lines for sound out of the laptop speakers, no blob, no firmware, and — verified — no vendor magic verbs. The Linux quirk for this exact SSID (`17aa:22be`, traced in `alc269.c:7858`) is a connection-list override plus preferred-DAC hints and two control renames. **No COEF writes, no GPIO writes.** dmesg even confirms the empty fixup: `"ALC285: picked fixup  for PCI SSID 17aa:22be"`.
- **Unblocks:** zlOS making a sound. The display-audio half stays deferred (no monitor, every ELD slot reads `eld_valid 0`).
- **Size:** ~800 for analog.
- **Biggest risk:** **speaker damage, and it is the one real hardware risk in this wave.** Pin amps 0x14/0x17/0x21 read `[0x80 0x80]` (muted) while DACs 0x02/0x03 read `[0x57 0x57]` (nsteps 0x57 — maximum). Unmuting a pin before a known-good sample stream sends uninitialised DMA — white noise at 0 dBFS — into the internal speakers at full scale, and a sustained DC offset into a laptop speaker coil is a thermal failure, not a loud noise. **Bring up on headphone pin 0x21 with nothing plugged in, or set the DAC amp low first.** Never touch pin 0x14 during bring-up.
  Prerequisite: build the read-only `hosttest/` BAR0 probe first, mirroring `gpu-dev.sh`. Every MMIO offset in that spec came from a header file, not from this silicon — which is exactly the situation that produced the 86-defect modeset audit.

### 4.3 — Framebuffer WC mapping + scrollback viewport
- **Why now:** the front buffer is **uncacheable, not write-combining.** `/proc/mtrr` reg00 covers 0x80000000-0xFFFFFFFF as `uncachable`, which contains both the BAR2 aperture and the GOP framebuffer at 0xC0000000. `boot64.S:81` maps everything WB with no PAT bit; UC wins. `fb.c:83`'s comment ("Video memory is write-combining") is false as zlOS maps it. Every `\n` at 2560x1440 moves 13.7 MB inside the back buffer and then presents 3.4 Mpx across that UC boundary.
- **Unblocks:** a usable boot log on a machine with no serial port.
- **Size:** ~650, of which the scrollback viewport (~150) is the part testable in QEMU today. The damage half is already built and gated (`hosttest/fbbench.c` asserts rect count and no-bleed) — do not rebuild it.
- **Biggest risk:** the PAT write. `IA32_PAT` (MSR 0x277) needs the SDM's exact CR0.CD/WBINVD/CR4.PGE sequence and **must be identical on every logical processor.** zlOS wakes 4 cores and parks 3 in `cli; hlt` sharing one CR3; those 3 still hold reset-default PAT. Inconsistent PAT across cores is undefined behaviour in the SDM, not a slow path. **Measure `resource2_wc` vs `resource2` in `hosttest/` first** — that is a clean A/B with zero register writes, and it may show the gap is not worth the hazard. Do the measurement before writing any PAT code.

---

## Wave 5 — untestable on this machine, push down

These have no device, no cable, or no safe development loop. This project's method is verifying against real hardware; a driver that cannot be checked is worth much less. **Do not report a green QEMU run as hardware success** — that is precisely the mistake already logged for `lt_armed`, where every write path passed in the harness and was unreachable from zlOS itself.

| Item | Why untestable | Verdict |
|---|---|---|
| **eth / I219** | Silicon present at `00:1f.6`, MAC `54:05:db:64:87:da`, but the X1C Gen8 has **no RJ45**. `carrier 0`, all 24 statistics read 0 — this NIC has never carried a frame. Needs a Lenovo 4X90Q84427 dongle. | Buy the £10 dongle first, or build only the QEMU `-device e1000` ring core (~500 lines) and stop before the pch_cnp layer. Do not claim it works. |
| **net stack (ARP/IP/ICMP/UDP)** | Pure software, fully testable — but only against a link layer, and there is none. | virtio-net under QEMU (~400 lines) is the honest first target. It makes the ~1700-line protocol work provable on the desk today. Reasonable to do out of order if hardware is unavailable. |
| **USB hub** | No external hub attached; every real device is at `Lev=01` on a root port. | Buy a hub before writing the port-reset path. Also fix `configure_endpoint()`'s dropped slot dword 2 (`tt_info`) — a latent bug that silently erases TT routing the moment anything sits behind a HS hub. |
| **USB mouse** | Driver exists (~200 lines in `xhci.c`) and works against QEMU `usb-tablet`. No real mouse attached; the boot-mouse relative branch has never met one. | Plug a mouse in. `ptr_abs` is inferred from `wMaxPacketSize >= 6`, which is not a protocol discriminator — a gaming mouse gets decoded by the absolute-tablet branch and the cursor teleports to garbage. |
| **USB mass storage** | Driver exists (~250 lines) and was verified against **QEMU's emulated `usb-storage` only**. `kernel/docs/plans/wireless-plan.md:54` calls bulk "written and proven for mass storage" — that claim is overstated and should be corrected in the repo. | Needs Wave 0.3 first. Then a real stick. Expect the first-command CHECK CONDITION to break it (QEMU never stalls). |
| **Bluetooth (AX201)** | Present and real, but gated on pushing 801,412 bytes of signed Intel firmware (`ibt-19-0-4.sfi`) through ~3,300 commands at ~493 µs each. And zlOS has no filesystem to source it from. | Prove the transport against an £8 CSR ROM dongle **first** — it answers HCI immediately with no upload, which stops transport bugs and firmware bugs hiding behind each other. Also: `wireless-plan.md` names the wrong file (`ibt-0040-0041.sfi`, 704 KB); correct it. |
| **WiFi (AX201)** | The radio is soldered on and works, but above ALIVE there are no registers — scan/auth/assoc are host commands into a firmware whose API (77) has `min == max` and no compatibility window. | ~15,000 lines for managed-mode-only. The repo doc's "40,000+" figure is the maximal version; the bounded one is 15k. Both are too big for now. Build the read-only `hosttest/wifi-dev.sh` BAR0 probe if you want to make progress cheaply. |
| **UVC camera** | Camera is present (13d3:5405) but `xhci.c` has **zero isochronous support** — TRB type 5 and EP type 5 appear nowhere — and no `SET_INTERFACE`. UVC alt 0 has no endpoints. | ~700 lines, and isoch is the bulk of it. Genuinely blocked on new xHCI work. |
| **Thunderbolt PCIe tunnelling** | Present and enumerable, but `pci_scan()` is a flat `for (bus = 0; bus < 4; bus++)` with no bridge recursion, so buses 5/6/7/0x2d are invisible. | The **DP half is free** — it is just display-roadmap Phase 3.2 external DP, zero Thunderbolt code. Take that. Skip the PCIe half: 900 lines whose payoff is "devices in a dock appear," for which you then need drivers anyway, and `ICM_APPROVE_DEVICE` hands a stranger all of physical RAM (`iommu_dma_protection = 0`, zlOS has no IOMMU). |

---

## 2. Not worth doing at all

Blunt list. Each of these should be closed, not deferred.

**`smbus.c` — do not build.** The controller is present and enabled, and there is **nothing on the other end of the wire.** Memory is soldered LPDDR3 ("Row Of Chips", blank part number, all-zero serial — the signature of no SPD device), the battery is behind the ACPI EC, all 10 hwmon devices trace elsewhere, and `i2c-0` has zero child devices. Linux's own driver managed exactly one transaction attempt: `"SMBus is busy, can't use it!"`. Someone will write it, write a bus scan, find nothing, and spend a day debugging a working driver. Build the EC battery path (2.3) instead — that is where the data actually is.

**MEI / HECI — do not build.** The only client that touches anything zlOS cares about is HDCP (`b638ab7e-...`, bound to i915 on this box), zlOS has zero HDCP code and no plan for it, and HDCP on an internal panel is pointless for a hobby OS. Worse: there is **no safe development loop.** `mei_me` holds `ea247000-ea247fff` exclusively, so every iteration is a blind reboot — the opposite of what made `intel.c` tractable. And a botched reset can leave the ME wedged in a state that survives a warm reboot, on the one test machine. The ME version facts you might want are readable from PCI config HFS_1..6 with no mailbox at all.

**`intel_thermal` as a PCI driver — do not build.** Despite the name it contains no thermometer; BAR0 is the MCHBAR aperture and the only feature is MMIO RAPL. Everything except PL4 is available from MSRs using the `read_msr()` that already exists. Rung 7 work for a rung 3 problem. Folded into 2.5.

**Intel IPU camera — the target does not exist.** `8086:02b0` on this machine is PCIe Root Port #9, class 0x060400, driven by `pcieport`. Comet Lake-U's PCH has no IPU block; both IPU driver ID tables on this box confirm 02b0 is not an IPU ID anywhere. Anything written against it would be a driver for a PCIe root port.

**eMMC / SDHCI — do not build.** Absent at three independent levels: no PCI function of class 08-05 exists; Linux *has* `sdhci-pci` with the matching Comet Lake alias and did not bind it; and the firmware itself defines `Device (PEMC)` at `_ADR 0x001A0000` with a `_STA` that returns 0 whenever `EMCE` is 0, which it is. The SD side is worse — `PSDC` appears once in the entire DSDT as a dangling path with no device behind it. There is no card slot on this chassis. And `xhci.c:1746` already implements SCSI-over-BOT, so a $5 USB card reader reads SD cards on zlOS today for zero new lines.

**TPM TIS transport — defer indefinitely; build the event-log parser instead.** The log half is free, self-verifying, and pure data: 51,313 bytes, 89 events, and it **replays 20/20** (SHA-1 and SHA-256, all 10 PCRs) once you know PCR0 is seeded with 31 zero bytes then the startup-locality byte `0x03`. That gives you the BIOS build (`N2WET51W`), proof Boot Guard was the CRTM, and the Secure Boot state, with zero hardware risk. The TIS transport, by contrast, has no host-side dev loop (reads are not side-effect-free and `tpm_tis` holds the chip), nothing in zlOS consumes what it would produce, and the dictionary-attack lockout is measured in hours with no soft reset. If the motivation was "zlOS has no entropy source" — this CPU has `rdseed`. That is a one-instruction problem, not a 700-line one.

**Watchdog (iTCO) — low priority, and check the lock first.** ~250 lines and it would work, but firmware demonstrably ran the lockdown path partway (`TCOCTL` bit 0 `TCO_BASE_LOCK` is set, measured `0x101`), and coreboot sets `TCO_BASE_LOCK` and `TCO1_CNT.TCO_LOCK` together. If `TCO_LOCK` is set the watchdog cannot be started at all and **the writes fail silently** — no fault, no status bit, and you get a watchdog you believe is armed and is not. Read `TCO1_CNT` bit 12 before writing anything. Genuine value for the display bring-up loop (turns a hang into a reboot), but verify the lock first or it is 250 wasted lines.

---

## 3. Read-only from Linux vs. needs metal

The `hosttest/gpu-dev.sh` method — map a BAR read-only from Linux userspace, iterate in seconds — is what turned `intel.c` from speculative into verified. It does not transfer everywhere. Here is where it does.

### Fully verifiable read-only from Linux, no zlOS boot needed

| Item | How |
|---|---|
| **entropy** | Pure logic + published CAVP vectors. `hosttest/entropytest.c` next to `cryptotest.c`. Zero hardware. |
| **gpt.c** | Byte parsing over `dd` output. The whole disk's GPT is already dumped and CRC-verified. |
| **fat32 read** | Three live FAT32 volumes on this box, including `zlOS-usb.img` itself. Loopback file + host harness. |
| **cache.c** | Pure logic. Test against a RAM-backed fake `nvme_read_block`, including power-cut simulation by dropping unflushed writes. |
| **TPM event log** | Already done — 20/20 replay, no device access at all. |
| **acpi.c table parsing** | Every table is in `/sys/firmware/acpi/tables/`, checksums already verified. |
| **net stack (ARP/IP/ICMP/UDP)** | Captured frames with verified checksums already exist as regression vectors (IPv4 `0x2306`, ICMP `0xCEB1`/`0xD6B1`, delta exactly `0x0800`). |
| **hda BAR0 probe** | `hosttest/` mmap of `resource0`. **Build this before writing one line of `hda.c`** — every MMIO offset in that spec is from a header, not from silicon. |
| **fb WC-vs-UC measurement** | `resource2_wc` and `resource2` both exist on this machine. Clean A/B, zero register writes. Do this before any PAT code. |
| **wifi CSR probe** | `hosttest/wifi-dev.sh` on the `gpu-dev.sh` pattern. Turns the register map from "upstream says" into "measured." |
| **e1000 register probe** | `hosttest/eth_probe.c` — and there is a free correctness check: `RAL(0)` must read `0x64DB0554` and `RAH(0)` `0x8000DA87`, computed from the measured MAC. That one check converts the whole register table from "sourced" to "verified." |

### Needs zlOS booted on the metal — no host loop possible

| Item | Why the host loop fails |
|---|---|
| **display Phase 0.1** | The write path is the point. Already has the best harness in the repo; still needs the boot. |
| **lpss_i2c / touchpad** | QEMU has no LPSS I2C, and reaching BAR0 from Linux needs an unbind that **kills the trackpad on the machine you are working on.** Boot-the-laptop-or-nothing. Print `COMP_TYPE`, `COMP_PARAM_1`, `HCNT`, `LCNT`, `TX_ABRT_SOURCE` on the boot console. |
| **rtc / acpi shutdown / reboot** | Either the machine goes dark or it does not. No failure output. Print a distinct serial marker before each write so a failure says which step was reached. |
| **hda write path** | Speaker damage risk; must be on real silicon with real codecs. |
| **watchdog** | Arming it *is* the test, and the test reboots the laptop. |
| **MEI** | BAR held exclusively; blind reboot per iteration. This is the reason not to build it. |
| **Bluetooth** | Firmware upload is stateful and cold-boot-vs-warm-boot dependent. A driver written and tested only after a warm reboot looks perfect on the desk and hangs on the first cold boot. |

### Two process notes that will otherwise cost a day

Other Claude sessions edit this same checkout — the tree is dirty right now (`CLAUDE.md`, `build.sh`, `kernel/src/arch/x86/idt.c`, `kernel/boot/efi.c`, `kernel/src/drivers/input/xhci.c` and 10 more modified, `crypto.c` untracked). Run `git status` before any gate or commit, and isolate with a worktree for anything multi-commit.

This box is 4 cores / 15 GB and has been OOM-killed before by agent fan-out plus QEMU. Check loadavg before pairing a QEMU gate with parallel agents.

### Weakest link in this plan, said unprompted

The ordering above assumes display Phase 0.1 lands first and unblocks nothing else — which is true structurally but not true for attention. Waves 2 and 3 are ~4,000 lines of genuinely useful, genuinely safe, genuinely verifiable work, and every one of those items is more pleasant than finishing `lt_armed`. That is the actual risk to this plan: not that the dependency order is wrong, but that the boring blocked thing stays blocked while the interesting unblocked things get built around it.
