# Kernel, RAM and CPU control — the gaps and the path (Windows)

Drafted 2026-08-03. What zl needs to control memory, the CPU, and the kernel on Windows, and what
is buildable at each tier. This is the deepest layer of "control the PC," and it is exactly the
layer zl's sister project Aegis (the kernel-mode antivirus) already works in - so this doubles as
"what would zl need to eventually write Aegis-class software."

**The one thing to understand first: there are TWO worlds, and a hard wall between them.**

---

## The ring model — user-mode vs kernel-mode

The CPU itself enforces two privilege levels:

```
RING 3  "user-mode"     - where every normal program runs (your browser, zl.exe)
   |    a wall the CPU enforces in hardware - you cannot cross it by choice
RING 0  "kernel-mode"   - where Windows itself and drivers run
```

- **Ring 3 (user-mode):** you can do a lot, but only THROUGH Windows. You ask the OS to read another
  process's memory, set CPU affinity, etc. Windows checks permissions and does it for you. A bug
  here crashes your program.
- **Ring 0 (kernel-mode):** total power. Touch any byte of physical RAM, any CPU register, any
  hardware. But you can only get here by **loading a driver** (a `.sys` file), which Windows requires
  to be **cryptographically signed**. A bug here crashes the WHOLE MACHINE (the blue screen).

**Everything below is tagged [R3] user-mode (FFI-reachable) or [R0] kernel-mode (needs a driver).**

---

## 1. RAM control

### [R3] Your own process's memory
- Raw pointers - read/write any byte YOUR program owns. zl needs: **the `bytes` type** and
  **pointers/addresses** (roadmap items). This is the base of everything.
- `VirtualAlloc` / `VirtualProtect` / `VirtualFree` - ask Windows for raw memory, and change its
  permissions (make a page executable, read-only, etc.). Pure **FFI** to `kernel32.dll`.
- Memory-mapped files - map a file straight into your address space. FFI.

### [R3] ANOTHER process's memory (what a debugger / cheat / AV scanner does)
- `OpenProcess` -> get a handle to another process.
- `ReadProcessMemory` / `WriteProcessMemory` - read and write ITS memory from outside.
- `VirtualQueryEx` - map out what memory regions it has.
- All **FFI** to `kernel32.dll`, all ring 3. This is how Cheat Engine, x64dbg and AV memory scanners
  work. Aegis's user-mode client does exactly this.

### [R0] Physical memory / other processes' memory without asking
- Reading raw physical RAM, walking another process's page tables directly, `MmMapIoSpace`.
- **Kernel only.** Needs a driver. This is where Aegis's driver lives.

**zl's path to RAM control:** `bytes` + pointers (items) unlock own-memory; **FFI** (item 16) unlocks
`VirtualAlloc` and cross-process read/write - the whole [R3] tier. The [R0] tier needs the driver
target (below).

---

## 2. CPU control

### [R3] What user-mode can do
- `CPUID` instruction - ask the CPU what it is (vendor, features, core count). Runnable from ring 3;
  zl needs **inline assembly** or an FFI helper to emit the instruction.
- `RDTSC` - read the CPU's cycle counter (a super-precise timer). Ring 3.
- Thread affinity - `SetThreadAffinityMask` pins your code to a specific core. FFI.
- Priority - `SetPriorityClass`. FFI.
- Performance counters - `QueryPerformanceCounter`. FFI.
- SIMD - use the vector units (SSE/AVX) for parallel math. Needs **SIMD types** (a Zig delta in
  GAPS_REALWORLD_2) or the backend emitting vector instructions.

### [R0] Privileged CPU control
- **MSRs** (Model-Specific Registers) - `RDMSR`/`WRMSR`. Control deep CPU behaviour: performance,
  power, security features, virtualization. **Kernel only** - these instructions fault in ring 3.
  Aegis reads MSRs for its telemetry.
- **Control registers** (CR0, CR3, CR4) - CR3 is the pointer to the current page tables; CR0 flips
  paging/protection. Touching these IS running the machine. Kernel only.
- **Interrupt table** (IDT) - what runs when the keyboard fires, a page faults, etc. Kernel only.
- Halting/waking cores, changing clock speed. Kernel only.

**zl's path to CPU control:** **inline assembly** (to emit `CPUID`/`RDTSC`) plus **FFI** covers the
whole [R3] tier. MSRs and control registers are [R0] - driver only.

---

## 3. Kernel editing

### [R3] Talking TO the kernel (the safe half)
- `DeviceIoControl` - send a command + buffer to a driver, get bytes back. This is how a user-mode
  program drives a kernel driver. Pure **FFI**. Aegis's client speaks to its driver exactly this way.
- Loading/unloading a driver - `OpenSCManager` / `CreateService` / `StartService`. FFI. (Loading an
  UNSIGNED driver needs test-signing mode or a signature - a Windows policy, not a zl limit.)
- ETW - subscribe to the kernel's own event feed (process starts, network, file, registry activity).
  FFI to `advapi32`/`tdh.dll`. Aegis consumes ETW-TI.

### [R0] BEING the kernel - writing a driver in zl
This is the real "edit the kernel" and it is a **new compile target**, not FFI:
- A `.sys` is a freestanding PE - no libc, no CRT, only kernel-exported functions (`Nt*`, `Ke*`,
  `Mm*`, `Ob*`), a strict kernel ABI, `/INTEGRITYCHECK` signing.
- Once loaded, a driver can: read/write ALL physical memory, hook system calls, register callbacks
  (`ObRegisterCallbacks`, `PsSetCreateProcessNotifyRoutine` - Aegis uses these), patch kernel
  structures (DKOM), inspect every process.
- **This is MASTER_PLAN Floor 6.** It is genuinely hard - but zl is unusually close: `nativegen`
  already emits kernel32-only freestanding PE with no runtime, which is the SAME discipline pointed
  at a different import set. And Aegis exists (13.5k lines of C, 98 phases) as the driver to validate
  a zl kernel target against.

---

## What zl needs, by tier

| tier | what it unlocks | zl needs |
|---|---|---|
| own-memory | pointers, VirtualAlloc, buffers | **`bytes` + pointers** (items) |
| [R3] full | cross-process RAM, CPU affinity, DeviceIoControl, driver load, ETW | **FFI (16) + sized ints (15)** |
| CPU instrs | CPUID, RDTSC, SIMD | **inline assembly** + SIMD types |
| [R0] kernel | physical RAM, MSRs, hooks, DKOM | **the `.sys` driver target (Floor 6)** |

**The pattern holds one more time:** the entire [R3] tier - cross-process memory editing, CPU
affinity, talking to drivers, loading drivers, ETW telemetry - is **FFI + a zl library**, exactly
like PC control and networking. Only *being* the kernel (writing the driver) is real engine work,
and it is Floor 6.

## Ordered path (Windows-only, as requested)

1. **`bytes` type + pointers** - own-process memory. Foundational, already high on the roadmap.
2. **Sized integers (15)** - to type Windows/kernel API arguments (`HANDLE`, `DWORD`, `SIZE_T`).
3. **FFI (16)** - unlocks the whole [R3] tier at once: `VirtualAlloc`, `ReadProcessMemory`,
   `WriteProcessMemory`, `OpenProcess`, `SetThreadAffinityMask`, `DeviceIoControl`, `CreateService`,
   ETW. This is the single biggest kernel/RAM/CPU-control unlock.
4. **Inline assembly** (or FFI helpers) - `CPUID`, `RDTSC`, and later the privileged instructions
   inside a driver.
5. **Proposed `memory`, `process`, and `cpu` modules under `stdlib/win/`** - libraries wrapping the [R3] APIs.
   This is where zl becomes a real systems tool: a memory scanner, a profiler, a process monitor -
   all pure zl on top of FFI.
6. **The `.sys` driver target (Floor 6)** - ring-0. The hard, later, strategic piece. Validated
   against Aegis.

**Safety note (carry forward from Aegis):** all of this stays scoped to the user's OWN machine,
via legitimate documented Windows APIs, for defensive/monitoring/educational use - the same posture
Aegis holds. Reading process memory, MSRs, loading a driver: these are what antivirus, debuggers,
profilers and anti-cheat all do with the same APIs.

**Down the line?** The [R3] tier (cross-process memory, CPU control, driver comms) is one feature
away - FFI. The [R0] tier (writing the driver itself) is Floor 6, genuinely far, but mapped, and
zl's freestanding `nativegen` output plus Aegis-as-reference make it reachable rather than fantasy.
