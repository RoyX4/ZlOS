#!/usr/bin/env python3
"""exercise.py - drive zlOS through every command the shell advertises.

verify.sh proves the kernel boots and its transcript has not changed; it does
that by typing five keys.  This types every command in help(), waits for each
one's own output, photographs the ones that only exist on screen, and says
which of them worked.

It boots the ISO rather than -kernel because GRUB is what supplies the
multiboot framebuffer tag; without it every graphical demo refuses with "needs
the framebuffer console" (kernel/CLAUDE.md).  -display none still emulates the
card, so the framebuffer is real and QMP can screendump it.

WAITING.  Pass/fail never depends on a wall clock - every verdict waits for the
shell prompt to come back, or for text the command itself printed.  The one
timed thing is a short settle before a screendump, which decides nothing: it
only lets a frame finish rendering before the picture is taken.

EXITING A DEMO.  Several demos block until a key arrives.  key_get() checks
serial first (kernel.zl:106), so a serial byte ends them - but WHICH byte
matters, and getting it wrong wedges the run with every later command
swallowed:
  - paint treats space as "clear the canvas", not "quit".  Send ESC.
  - snake needs TWO keys: one ends the game, one dismisses game-over.
This is also how snake's PS/2-only read was found: it used kbd_scan(), which
sees the scancode queue and nothing else, so no serial byte could ever end it
on a machine without a PS/2 keyboard.  It uses key_get() now.

SCREENDUMPS.  QEMU writes the PPM asynchronously, so the file is deleted first
and waited for - "it exists and is non-empty" is otherwise satisfied instantly
by the PREVIOUS run's picture, which silently compares against stale pixels.

  ./exercise.py                 boot the ISO, run every step
  ./exercise.py --uefi          same, but through OVMF like the real laptop
  ./exercise.py --only k,u,o    just those steps
"""
import argparse, json, os, shutil, socket, subprocess, sys, tempfile, time

PROBE_DIR = os.path.dirname(os.path.abspath(__file__))
KERNEL_ROOT = os.path.abspath(os.path.join(PROBE_DIR, "..", ".."))
HERE = KERNEL_ROOT
PROMPT = "zl> "
OVMF_CODE = "/usr/share/OVMF/OVMF_CODE_4M.fd"
OVMF_VARS = "/usr/share/OVMF/OVMF_VARS_4M.fd"

# name, keys, kind, exit, want, forbid
#
# `keys` IS A TYPED COMMAND NAME NOW, not a keypress. It used to be "h", "20f",
# "?"  - single characters that run_command dispatched on directly. The
# compositor is the boot state on every path that has a framebuffer, and in it
# the shell is a window whose input is a LINE: characters accumulate until
# Enter. So the driver appends a carriage return to each of these and term.c
# looks the word up. The old form did not fail loudly; it typed five characters
# into a line buffer and waited forever, which is why this note exists.
#   kind  "text" - it prints its result and returns on its own
#         "gfx"  - it draws; photograph it while it is up, then end it
#   exit  None        - returns by itself
#         "ser:X"     - send X down the serial line
#         "qmp:KEY"   - send a PS/2 scancode (for anything reading kbd_scan)
#   want / forbid are matched against what THIS step printed, not the whole log
STEPS = [
    ("help",       "help",   "text", None, ["help              this help",
                       "quit              halt the machine"], []),
    ("fib",        "fib 20", "text", None, ["6765"], []),
    ("sumsq",      "sum 10", "text", None, ["385"], []),
    ("uptime",     "uptime",   "text", None, ["uptime:", "ticks at 100 Hz"], []),
    ("cpuid",      "cpuid",   "text", None, ["CPU:"], []),
    ("pokepeek",   "poke",   "text", None, ["read back: 42"], []),
    ("beep",       "beep",   "text", None, ["beep!"], []),
    ("pci_intel",  "pci",   "text", None,
        ["PCI bus scan", "devices found:", "display adapter", "modesetting:"], []),
    ("xhci",       "usb",   "text", None,
        ["USB host controller (xHCI)", "it is ours", "no-op command completed",
         "devices attached:"],
        ["could not take the controller", "not backed by RAM", "enumeration failed"]),
    # j and = run a 60 s timer of their own, so they need a ceiling above it
    ("usbkbd",     "usbkbd",   "text", "ser:\x1b",
        ["USB keyboard test", "keyboard on slot"], ["no keyboard"]),
    ("cpu",        "cpu",   "text", None,
        # TCG does not expose deterministic cache leaves; KVM/real hardware
        # does. Vendor/topology/features are the portable command contract,
        # while cache-leaf decoding is covered where the CPU supplies it.
        ["vendor:", "topology:", "features:"], []),
    ("nvme",       "nvme",   "text", None,
        ["admin + I/O queues created - it is ours", "read them back, all match"],
        ["not backed by RAM"]),
    ("sched",      "sched",   "text", None,
        ["tasks now runnable:", "switches="], []),
    ("smp",        "smp",   "text", None,
        ["cores online now: 4 of 4", "running on more than one core"], []),
    ("usbstor",    "usbstor",   "text", None,
        ["bulk IN and OUT endpoints configured", "INQUIRY:",
         "read a sector off a USB stick"], []),
    # QEMU has no LPSS I2C - the pass here is that it says so instead of hanging
    ("i2c_hid",    "i2c",   "text", None, ["I2C-HID touchpad"], []),
    # "keys" drives the emulated KEYBOARD rather than the serial line. The
    # input-events demo reads in_next(), the unified PS/2+USB event queue, and
    # never looks at serial - so this is the one step that proves a real key
    # press reaches zlOS. On the laptop that is the only way in.
    # The key arrives as a CHAR event, which is the USB HID route (a PS/2 key
    # would come through as down/up scancodes) - so this also proves the xHCI
    # keyboard stack, the only keyboard a UEFI laptop has after boot services
    # go away.
    # This step used to assert only that SOME keystroke was reported, and it
    # passed for four rounds while ESC was broken over USB: the demo was ending
    # on its 30 s timer, not on the key, and from outside those are the same
    # thing - both finish at the prompt. So assert the difference directly.
    # "ESC - done" is printed only on the early exit and "timed out" only on
    # the timer, which makes want/forbid exact rather than circumstantial.
    # "down " proves the key arrived as a KEY event and not just a character,
    # and 0x0112 is KEY_UP - the arrows the demo tells you to try, which the
    # HID decoder used to drop on the floor.
    ("input",      "input",   "keys", None,
        ["input events", "char ", "'a'", "down ", "0x0112", "ESC - done"],
        ["timed out"]),
    ("virtio_gpu", "gpu",   "gfx",  None,
        ["virtqueue up, DRIVER_OK", "TRANSFER_TO_HOST_2D + RESOURCE_FLUSH acknowledged"],
        ["refused our feature negotiation", "flush failed", "GET_DISPLAY_INFO failed"]),
    ("colorbars",  "bars",   "gfx",  None,
        ["14 colours, painted straight into the framebuffer"], ["needs the framebuffer"]),
    ("windows",    "windows",   "text", None,
        ["compositor is already the desktop"], ["needs the framebuffer"]),
    ("cube3d",     "cube",   "gfx",  "ser: ", [], ["needs the framebuffer"]),
    ("anim",       "anim",   "gfx",  "ser: ", [], ["needs the framebuffer"]),
    # The old full-screen mouse demo printed a final coordinate. It is a normal
    # concurrent Pointer app now, so the gate is the reported window plus a
    # changed screenshot; probe-mouse.py owns exact device coordinates.
    ("mouse",      "mouse",   "gfx", None,
        ["wm: win"], ["needs the framebuffer"]),
    ("snake",      "snake",   "gfx",  "ser:\x1b\x1b", [], ["needs the framebuffer"]),
    ("paint",      "paint",   "gfx",  "ser:\x1b", [], ["needs the framebuffer"]),
    ("format",     "format", "text", None,
        ["zlfs on NVMe", "mounted:"], ["format failed", "no usable NVMe"]),
    ("fs_list",    "ls",   "text", None, ["zlfs files:"], ["RAM files:"]),
    ("modeset",    "mode",   "gfx",  None,
        ["modesetting with our own driver"], ["the card refused that mode"]),
    # KNOWN ISSUE, ordered last on purpose. With usb-tablet attached, running
    # the input-events demo ('=') first leaves the USB keyboard not delivering,
    # so the editor never receives its ESC and wedges the shell - and anything
    # after it in the list then fails too. Measured: editor alone passes,
    # mouse->editor passes, input->editor fails, and detaching the tablet makes
    # input->editor pass. Six hypotheses tested and none held (see
    # docs/input-stack.md). Kept here, last, so the failure is visible and
    # named rather than hidden by narrowing the test.
    ("editor",     "edit 3",  "gfx",  "qmp:esc", ["wm: win"], ["needs the framebuffer"]),
    ("editor_file", "ls", "text", None, ["/user/notes.txt"], []),
    ("clear",      "clear",   "text", None, [], []),
]
SETTLE = 2.5          # seconds of rendering before a photograph - decides nothing
MIN_DELTA = {"cube3d": 0.005, "anim": 0.005, "mouse": 0.005}
# Two demos run a 60 s timer of their own and print "timed out" when it expires.
# Their ceiling has to clear that, or the harness gives up on a healthy demo.
CEILING = {"usbkbd": 75.0, "input": 75.0, "cpu": 120.0}


class Serial:
    def __init__(self, path):
        self.sock = _connect(path)
        self.sock.settimeout(0.2)
        self.buf = ""

    def pump(self):
        try:
            data = self.sock.recv(65536)
        except (socket.timeout, OSError):
            return False
        if not data:
            return False
        self.buf += data.decode("latin-1").replace("\r", "")
        return True

    def wait(self, marker, ceiling):
        deadline = time.time() + ceiling
        while time.time() < deadline:
            i = self.buf.find(marker)
            if i >= 0:
                out, self.buf = self.buf[:i + len(marker)], self.buf[i + len(marker):]
                return True, out
            self.pump()
        out, self.buf = self.buf, ""
        return False, out

    def drain(self, secs):
        end = time.time() + secs
        while time.time() < end:
            self.pump()

    def send(self, s):
        self.sock.sendall(s.encode("latin-1"))


class Qmp:
    def __init__(self, path):
        self.f = _connect(path).makefile("rwb")
        self.f.readline()
        self.cmd("qmp_capabilities")

    def cmd(self, name, **args):
        msg = {"execute": name}
        if args:
            msg["arguments"] = args
        self.f.write((json.dumps(msg) + "\n").encode()); self.f.flush()
        while True:
            line = self.f.readline()
            if not line:
                return None
            reply = json.loads(line)
            if "event" not in reply:
                return reply

    def sendkey(self, key):
        # input-send-event, NOT send-key. send-key routes through the active
        # console's handler and under -display none there is none, so the key
        # is silently dropped - measured: send-key never arrived, this does.
        for down in (True, False):
            self.cmd("input-send-event", events=[
                {"type": "key",
                 "data": {"down": down, "key": {"type": "qcode", "data": key}}}])

    def screendump(self, path, device=None):
        # Delete first. QEMU writes the PPM asynchronously, so "the file exists
        # and is non-empty" is satisfied instantly by LAST run's picture - which
        # silently compares this run's frame against a stale one.
        try:
            os.unlink(path)
        except FileNotFoundError:
            pass
        r = self.cmd("screendump", filename=path, **({"device": device} if device else {}))
        if r is None or "error" in r:
            if device:                       # older QEMU, or no such qdev id
                return self.screendump(path)
            return False
        last = -1
        for _ in range(200):                 # wait for it to appear AND settle
            if os.path.exists(path):
                size = os.path.getsize(path)
                if size > 64 and size == last:
                    return True
                last = size
            time.sleep(0.05)
        return os.path.exists(path) and os.path.getsize(path) > 64


# ---- typing on the EMULATED KEYBOARD ---------------------------------------
# THE CANONICAL COPY. probe-term.py and probe-net.py each carry their own,
# because a module name with a hyphen in it is not importable and neither could
# import the other; both import THIS file already, so this is where the one
# copy belongs. When either lands on this line, delete its local QCODE/qtype
# and import these instead.
#
# Typing over the emulated keyboard is a DIFFERENT ASSERTION from typing over
# serial, not a workaround for it - serial reaches the compositor perfectly
# well (input.c "SERIAL, the third source"). This exercises the PS/2 and USB
# HID decoders, which is the only input a laptop has and the only path with no
# wire to fall back on. Use it when the keyboard is the thing under test.
#
# A character with no qcode is a hard error, never a silent skip: a probe that
# quietly drops a keystroke asserts against a command nobody typed and passes
# for the wrong reason. probe-mouse.py already cost this project one of those.
QCODE = {
    " ": "spc", "\n": "ret", "-": "minus", "=": "equal",
    ".": "dot", ",": "comma", "/": "slash", ";": "semicolon", "'": "apostrophe",
    ":": "semicolon",
}
for _c in "abcdefghijklmnopqrstuvwxyz":
    QCODE[_c] = _c
for _c in "0123456789":
    QCODE[_c] = _c


def qtype(qmp, text, settle=0.12):
    """Type `text` on the emulated keyboard, one key at a time.

    input-send-event, not send-key: under -display none there is no active
    console handler and send-key is silently dropped. Both the PS/2 and the USB
    path turn a key into an EV_CHAR (input.c handle_scancode and the xhci_key
    drain), so either keyboard QEMU routes this to will do.
    """
    for ch in text:
        code = QCODE.get(ch)
        if code is None:
            raise RuntimeError(f"no qcode for {ch!r} - add one rather than skipping it")
        qmp.sendkey(code)
        time.sleep(settle)


def _connect(path):
    s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    for _ in range(400):
        try:
            s.connect(path); return s
        except (FileNotFoundError, ConnectionRefusedError):
            time.sleep(0.05)
    raise RuntimeError("socket never appeared: " + path)


def ppm_sample(path):
    """(w, h, [sampled pixels]) from a QEMU PPM.

    Sampled on a fixed grid so two frames of the same size sample the SAME
    pixels and can be compared directly.
    """
    with open(path, "rb") as fh:
        blob = fh.read()
    if not blob.startswith(b"P6"):
        return None
    fields, i = [], 2
    while len(fields) < 3:
        while i < len(blob) and blob[i:i + 1].isspace():
            i += 1
        if blob[i:i + 1] == b"#":
            while i < len(blob) and blob[i] != 0x0A:
                i += 1
            continue
        j = i
        while j < len(blob) and not blob[j:j + 1].isspace():
            j += 1
        fields.append(int(blob[i:j])); i = j
    i += 1
    w, h, _ = fields
    px = blob[i:]
    step = max(1, (w * h) // 40000) * 3       # a full 1920x1200 scan is pointless
    return w, h, [bytes(px[p:p + 3]) for p in range(0, len(px) - 2, step)]


def frame_delta(before, after):
    """Fraction of sampled pixels that differ between two frames."""
    if not before or not after or len(before[2]) != len(after[2]):
        return None
    a, b = before[2], after[2]
    diff = sum(1 for i in range(len(a)) if a[i] != b[i])
    return diff / len(a) if a else 0.0


def build(uefi):
    script = "./tools/images/mkusb.sh" if uefi else "./tools/images/mkiso.sh"
    r = subprocess.run([script], cwd=HERE, capture_output=True, text=True)
    if r.returncode != 0:
        print(script, "failed:\n", r.stdout[-2000:], r.stderr[-2000:]); sys.exit(1)


def qemu_argv(tmp, uefi, ser_path, qmp_path, tablet=True, net=False):
    disk, stick = os.path.join(tmp, "nvme.img"), os.path.join(tmp, "stick.img")
    for p, mb in ((disk, 64), (stick, 32)):
        subprocess.run(["qemu-img", "create", "-f", "raw", p, f"{mb}M"],
                       check=True, capture_output=True)
    # CI/sandbox runners often expose QEMU but not /dev/kvm. `-cpu host` is
    # legal only with KVM, so choose the accelerator and CPU as one pair. This
    # is a slower proof of the same guest image, not a different boot path.
    accel = ("-cpu", "host", "-accel", "kvm") if os.path.exists("/dev/kvm") \
            else ("-cpu", "max", "-accel", "tcg,thread=multi")
    common = [
        "-m", "1G", "-smp", "4", *accel,
        "-drive", f"file={disk},if=none,id=nvm,format=raw",
        "-device", "nvme,serial=zlos001,drive=nvm",
        "-drive", f"file={stick},if=none,id=stick,format=raw",
        "-device", ("qemu-xhci,id=xhci,p2=8,p3=4" if net == "cdc" else
                    "qemu-xhci,id=xhci"),
        "-device", "usb-storage,bus=xhci.0,drive=stick",
        "-device", "usb-kbd,bus=xhci.0",
        # No usb-mouse (relative): zlOS would rather have the TABLET, which is
        # absolute - the guest is told the position instead of a delta to
        # accumulate, so the cursor cannot drift away from the host's. This is
        # what try.sh attaches, so it is what the sweep must exercise.
        # MEASURED 2026-08-18, probe-mouse-sync.py. zlOS reads TWO pointers and
        # prefers the USB one: xhci.c drives this tablet (absolute) and idt.c
        # the PS/2 mouse (relative). Both work - but only with the event type
        # that matches. With the tablet attached, 21 RELATIVE deltas move the
        # pointer zero pixels; ABSOLUTE events land it exactly (0.75/0.50 of
        # the screen -> 1439,599 on 1920x1200). Without the tablet, relative
        # events are exact instead. A pointer test that sends the wrong kind
        # will report a dead pointer and be believed.
        # tablet=False exercises the PS/2 fallback, which is what the laptop's
        # TrackPoint actually is - not a synthetic case.
        *(("-device", "usb-tablet,bus=xhci.0") if tablet else ()),
        # THE NIC IS OPT-IN, and deliberately off by default. Every probe here
        # shares this argv, so attaching a card unconditionally would add a PCI
        # device to machines whose gates photograph the `k` (PCI + GPU) readout
        # or count devices on the bus - i.e. it would change the answer of
        # gates that have nothing to do with networking. probe-net.py passes
        # net=True; nothing else needs to, and try.sh attaches one always
        # because that machine is for a person, not for an assertion.
        *(("-netdev", "user,id=n0",
           "-device", ("e1000,netdev=n0" if net == "e1000" else
                       "usb-net,bus=xhci.0,netdev=n0" if net == "cdc" else
                       "virtio-net-pci,netdev=n0")) if net else ()),
        "-no-reboot", "-display", "none",
        "-chardev", f"socket,id=ser0,path={ser_path},server=on,wait=off",
        "-serial", "chardev:ser0",
        "-qmp", f"unix:{qmp_path},server=on,wait=off",
    ]
    if uefi:
        varsf = os.path.join(tmp, "vars.fd")
        shutil.copy(OVMF_VARS, varsf)
        return ["qemu-system-x86_64"] + common + [
            "-drive", f"if=pflash,format=raw,unit=0,readonly=on,file={OVMF_CODE}",
            "-drive", f"if=pflash,format=raw,unit=1,file={varsf}",
            "-drive", f"format=raw,file={HERE}/zlOS-usb.img,if=none,id=boot",
            "-device", "usb-storage,bus=xhci.0,drive=boot",
            # -vga std, NOT -vga none. OVMF publishes no GOP this kernel can use
            # for a bare virtio-gpu-pci, so efi.c found no framebuffer and fell
            # back to a VGA text console that does not exist under UEFI - a black
            # screen, with the serial log still looking healthy. Measured with
            # probe-uefi.py: std VGA, VMware svga and virtio-vga all work.
            # virtio-gpu-pci stays as a SECOND device so the 'y' demo still has
            # one to drive without fighting the console for it.
            "-vga", "std", "-device", "virtio-gpu-pci,id=vgpu,xres=1280,yres=800",
        ]
    return ["qemu-system-i386"] + common + [
        "-cdrom", f"{HERE}/zlOS.iso",
        "-device", "virtio-gpu-pci,id=vgpu,xres=1280,yres=800",
    ]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--uefi", action="store_true")
    ap.add_argument("--only", default="")
    ap.add_argument("--outdir", default="")
    ap.add_argument("--boot-ceiling", type=float, default=180.0)
    ap.add_argument("--step-ceiling", type=float, default=40.0)
    ap.add_argument("--no-build", action="store_true")
    args = ap.parse_args()

    tag = "uefi" if args.uefi else "bios"
    outdir = args.outdir or os.path.join(HERE, "exercise-out", tag)
    steps = STEPS
    if args.only:
        keep = {s.strip() for s in args.only.split(",")}
        steps = [s for s in STEPS if s[0] in keep]

    os.makedirs(outdir, exist_ok=True)
    if not args.no_build:
        build(args.uefi)

    tmp = tempfile.mkdtemp(prefix="zlos-ex-")
    ser_path, qmp_path = os.path.join(tmp, "ser"), os.path.join(tmp, "qmp")
    proc = subprocess.Popen(qemu_argv(tmp, args.uefi, ser_path, qmp_path),
                            stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
    transcript, results = [], []
    wedged = False        # a demo trapped the shell; later steps cannot run
    try:
        ser, qmp = Serial(ser_path), Qmp(qmp_path)
        ok, boot = ser.wait("ready.", args.boot_ceiling)
        transcript.append(boot)
        if not ok:
            print("FAIL: never reached 'ready.' - it did not boot\n", boot[-1500:])
            return 1
        if not ser.wait(PROMPT, 30)[0]:
            print("FAIL: reached 'ready.' but no shell prompt")
            return 1
        print(f"booted {tag.upper()}, shell is up "
              f"({'UEFI/GOP' if args.uefi else 'GRUB multiboot'})\n", flush=True)

        for name, keys, kind, exit_how, want, forbid in steps:
            if wedged:
                print(f"  skip  {name:<11} (shell wedged by an earlier step)",
                      flush=True)
                results.append((name, None))      # None = skipped, not a failure
                continue
            problems, shot = [], ""
            dev = "vgpu" if name == "virtio_gpu" else None
            before = None
            if kind in ("gfx", "pointer"):
                # What the screen looked like before the command ran. The test
                # that actually means something is that the demo CHANGED it -
                # an absolute brightness threshold fails demos that legitimately
                # draw on black (the mouse demo clears the screen first), and
                # passes a demo that refused and left the desktop up.
                bpath = os.path.join(outdir, f"{name}-before.ppm")
                if qmp.screendump(bpath, dev):
                    before = ppm_sample(bpath)

            # Enter is what submits a line. Sending the word alone leaves it
            # sitting in the input buffer and the step times out looking like a
            # hung demo.
            ser.send(keys + "\r")

            carried = ""
            if kind == "keys":
                # Wait for the demo to be listening, then press real keys at it.
                # wait() CONSUMES what it matched, so keep it - the assertions
                # below are made against this step's whole output.
                _, carried = ser.wait("ESC ends.", args.step_ceiling)
                # "up" covers the arrows, which have no character and so only
                # exist as key events - the case that was silently missing.
                # "esc" must END the demo; the step forbids "timed out".
                for k in ("a", "b", "up", "esc"):
                    qmp.sendkey(k)
                    ser.drain(0.4)

            if kind == "pointer":
                # Absolute: send a POSITION, not deltas, and it must land there.
                # 0..32767 is the HID logical range; 75%/90% of a 1920x1200
                # screen is 1439,1079 and the step asserts on exactly that.
                ser.drain(1.0)
                qmp.cmd("input-send-event", events=[
                    {"type": "abs", "data": {"axis": "x", "value": int(0.75 * 32767)}},
                    {"type": "abs", "data": {"axis": "y", "value": int(0.90 * 32767)}}])
                ser.drain(0.5)

            if kind in ("gfx", "pointer"):
                # let the frame finish, photograph it WHILE the demo owns the
                # screen, and only then end the demo
                ser.drain(SETTLE)
                path = os.path.join(outdir, f"{name}.ppm")
                if qmp.screendump(path, dev):
                    after = ppm_sample(path)
                    if after:
                        w, h, _ = after
                        delta = frame_delta(before, after)
                        shot = (f"  [{w}x{h} {len(set(after[2]))}c"
                                + (f" {delta*100:.0f}% changed]" if delta is not None
                                   else " no before-frame]"))
                        minimum = MIN_DELTA.get(name, 0.02)
                        if delta is not None and delta < minimum:
                            problems.append(
                                f"drew nothing - only {delta*100:.1f}% of the screen changed")
                        # Only a genuinely uniform frame counts as blank. The
                        # animation is a sparse starfield on black and samples
                        # to three colours while plainly working - the delta
                        # above is what actually proves something was drawn.
                        if len(set(after[2])) < 2:
                            problems.append("screen is one flat colour")
                else:
                    problems.append("screendump failed")

            if exit_how:
                if exit_how.startswith("qmp:"):
                    qmp.sendkey(exit_how[4:])
                else:
                    ser.send(exit_how[4:])
            got, out = ser.wait(PROMPT, CEILING.get(name, args.step_ceiling))
            if not got:
                # last resort: it is in a loop neither route reached
                qmp.sendkey("spc"); ser.send(" \x1b")
                got, more = ser.wait(PROMPT, args.step_ceiling)
                out += more
                if got:
                    problems.append("needed an unexpected second exit key")
            if not got:
                # Still wedged: the shell is inside a demo that will not let go,
                # so nothing after this can be tested on this boot.
                #
                # QMP system_reset is NOT the answer here - the guest runs with
                # -no-reboot (deliberately, so a triple fault exits instead of
                # looping forever), which turns a reset into QEMU exiting and
                # the serial socket breaking under us.
                #
                # So report the remaining steps as SKIPPED rather than FAILED.
                # One stuck demo used to fail every step after it, which turned
                # a single defect into 27/30 and buried the real failure in its
                # own collateral. Skipped is the honest word: those steps were
                # never run, and calling them failures overstates the damage
                # just as much as hiding them would understate it.
                problems.append("wedged the shell - later steps could not run")
                wedged = True

            out = carried + out
            transcript.append(f"\n########## {name}  (sent {keys!r}) ##########\n" + out)
            if not got:
                problems.append("never returned to the prompt")
            problems += [f"missing {w!r}" for w in want if w not in out]
            problems += [f"said {f!r}" for f in forbid if f in out]

            print(f"  {'ok  ' if not problems else 'FAIL'}  {name:<11}{shot}", flush=True)
            for p in problems:
                print(f"          - {p}", flush=True)
            results.append((name, not problems))

        # halt lives outside the loop, so it needs the same guard: a shell that
        # is stuck inside a demo cannot be asked to halt, and reporting that as
        # a halt FAILURE blames the wrong thing.
        if wedged:
            print("  skip  halt        (shell wedged by an earlier step)", flush=True)
            results.append(("halt", None))
        else:
            ser.send("quit\r")
            ok, out = ser.wait("halting.", 30)
            transcript.append("\n########## halt ##########\n" + out)
            print(f"  {'ok  ' if ok else 'FAIL'}  halt", flush=True)
            results.append(("halt", ok))
    finally:
        proc.terminate()
        try:
            proc.wait(timeout=10)
        except subprocess.TimeoutExpired:
            proc.kill()
        with open(os.path.join(outdir, "transcript.txt"), "w") as fh:
            fh.write("".join(transcript))
        shutil.rmtree(tmp, ignore_errors=True)

    bad     = [n for n, good in results if good is False]
    skipped = [n for n, good in results if good is None]
    ok      = len(results) - len(bad) - len(skipped)
    print(f"\n{ok}/{len(results)} steps ok"
          + (f", {len(skipped)} skipped" if skipped else "")
          + (f" - failed: {', '.join(bad)}" if bad else ""))
    print(f"evidence: {outdir}")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
