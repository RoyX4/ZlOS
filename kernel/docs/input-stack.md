# The input stack: why arrow keys did not work, and what shape fixes it

Written after the bug where arrow keys never reached an application in the
compositor while every ordinary character worked perfectly.

## The report, and why its first guess was wrong

The symptom was exact and correctly measured: in the browser app, injecting
qcode `spc` scrolled the document every time, and injecting qcode `down` did
nothing at all. Both are handled by the same `browser_key()`. So `EV_CHAR`
reached the app and the extended keys did not.

The natural suspect was the PS/2 decode path. Arrows, Page Up/Down, Home, End,
Insert and Delete are all `0xE0`-prefixed two-byte scancodes, and a driver that
mishandles the prefix loses **exactly that set** and nothing else. It fits the
evidence perfectly.

It is also wrong. `hosttest/inputtest.c` drives the real `input.c` with a
scripted scancode stream and the PS/2 path decodes all nine of them correctly,
before any change:

```
PS/2 down   (E0 50/E0 D0)    ->  KEY_DOWN 0x113  KEY_UP 0x113
PS/2 up/lt/rt/pgup/pgdn      ->  KEY_DOWN 0x112  0x110  0x111  0x116  0x117
```

`idt.c` never mattered either. Its IRQ1 handler pushes whatever byte port 0x60
produced into a ring, `0xE0` included, and takes no view on what it means.

**The keyboard in the repro was not the PS/2 one.** `try.sh` attaches
`-device usb-kbd`, and QEMU routes typing to the USB keyboard once it exists.
The failing path was USB HID.

## The actual defect

`input_poll()` drained USB like this:

```c
/* USB HID, which already hands us decoded characters */
for (int i = 0; i < 8; i++) {
    int c = xhci_key();
    if (!c) break;
    evq_push(EV_CHAR, (u32)c, mods, 0, 0);
}
```

A character queue **cannot carry an arrow key.** There is no character for Up.
`hid_to_ascii()` in `xhci.c` had no case for usages 0x4F–0x52, so it returned 0,
and 0 already means "nothing was typed". The key vanished with no error at any
layer — no dropped-event counter, no unknown-key path, nothing to grep for.

Two independent things had to be true for a fix, and neither was:

1. **`hid_to_ascii()` had no entry for the arrows.** Adding four cases does not
   help on its own, because there is no character to return.
2. **The transport was the wrong shape.** Even with a mapping, the only way out
   of `xhci_key()` was an `int` that `input_poll()` pushed as `EV_CHAR`. Pushing
   `0x113` as a character produces an event of the wrong *type* — measured:

   ```
   USB down (hypothetical)  ->  CHAR 0x113
   ```

   `wm.c` routes on type, and an app asking "was this a key press" would never
   see it. So the fix could not be local to `hid_to_ascii()`; the transport had
   to change.

`wm.c` was never involved. `route_key()` forwards every event type to the
focused window's `app_event`, including `EV_KEY_DOWN`. It is transparent.

## The shape it has now

**`xhci.c` reports what the hardware said and stops there.** Its queue carries
packed HID events rather than characters:

```
bits  7:0   HID usage ID   (never 0 for a real key, so 0 = queue empty)
bits 15:8   HID modifier bitmap from the same report
bit  16     1 = press, 0 = release
```

**`input.c` owns all translation.** It already had a keymap, a caps-lock rule
and a control-code rule for PS/2. Duplicating those for USB is how two
keyboards drift apart until a bug reads as "works on the laptop, not on the
external one". So the printable USB keys are translated into the set-1 scancode
they correspond to (`hid_to_sc1()`) and handed to the *existing* `to_char()`.
One keymap, one policy, both keyboards. Only the keys with no character get a
`KEY_*` code directly, via `hid_to_key()`.

`handle_hid_event()` is deliberately the same shape as `handle_scancode()`. If
they diverge, a key works on one keyboard and not the other, which is the class
of bug this whole file exists to stop.

### Three things that were missing and are not related to arrows

Found while fixing the above; each was silently absent the same way.

- **USB sent no key releases.** The boot report is a snapshot of what is held,
  and `hid_decode()` only ever diffed for *new* presses. Nothing ever cleared
  `key_down[]` or stopped auto-repeat, so a held key would have repeated until a
  different one was pressed. Releases are emitted now.
- **Ctrl did nothing over USB.** `hid_to_ascii()` handled shift and only shift,
  so Ctrl+W arrived as `'w'` — the compositor's close key was dead on an
  external keyboard. Going through `to_char()` fixes it for free.
- **A held shift alone was invisible.** Pressing shift with no other key sends a
  report with no usage IDs at all, so nothing downstream learned it. `xhci.c`
  publishes the live modifier bitmap now and `input_shift()` ORs both keyboards.

### One detail worth not undoing

`key_down[]` has three regions, not two:

```
0x000..0x0FF   PS/2 set 1
0x100..0x1FF   PS/2 set 1, 0xE0-prefixed
0x200..0x2FF   USB HID usage IDs
```

A USB `'a'` is usage 0x04 and a PS/2 F9 is scancode 0x04. Sharing a slot means
releasing one un-holds the other. `inputtest.c` asserts this specifically.

`xhci_key()` still exists and still returns a character, reading its **own**
queue. The zl `usb_key` builtin compares what it gets against 13 and 27, and
`kernel.zl` calls it in two places. Two queues rather than one also means the
shell reading characters and the compositor reading events cannot steal
keystrokes from each other, which one queue would have allowed.

## What is still missing, deliberately

Neither keyboard grew these; they are named here so the next reader does not
assume the stack is complete.

- **The USB keypad is unmapped.** HID usages 0x54–0x63 get no `KEY_*` and no
  character. PS/2 does map the keypad, through `sc_plain`, so this is one place
  the two keyboards still disagree. Doing it properly needs a Num Lock policy —
  the keypad sends the same usage whether Num Lock is on or off and the *host*
  decides whether that means `2` or Down — and that was more than this fix.
- **No LED feedback.** Caps Lock and Num Lock toggle the internal latch on both
  paths, but nothing sends the HID output report or the PS/2 `0xED` command, so
  the physical light never changes.
- **`i2c_hid.c` is untouched and still unproven** — QEMU has no Intel LPSS I2C,
  so the ThinkPad's built-in keyboard has never been exercised through it.
- **PrintScreen, ScrollLock and Pause** are mapped on neither path.

## The test

`hosttest/inputtest.c` compiles the exact `input.c` that ships in the kernel
against fake hardware. No GPU, no root, no QEMU — it runs anywhere in
milliseconds.

```
cd kernel/hosttest && ./build.sh && ./inputtest
```

The assertions that matter are the parity ones: the same key must produce the
same event from either keyboard. A test that only asks "did something happen"
passes on the broken code, because something did happen — just not for arrows.
Assert on event **type and code**, or this bug comes back invisibly.
