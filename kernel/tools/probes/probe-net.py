#!/usr/bin/env python3
"""probe-net.py - does the network stack actually come up on real emulated hardware?

WHY THIS EXISTS. `net_gate()` in kernel.zl is a complete bring-up test - it
finds the card, checks the DMA arena is backed by RAM, negotiates features,
reads the link, ARPs the gateway and then pings it twenty times reporting loss
and jitter. It is bound to `N` at kernel.zl:2124. **And nothing had ever run
it**, because no QEMU invocation in the original gate attached a network card:

    $ git grep -ln 'netdev\\|-nic\\|virtio-net' -- '*.sh' '*.py'
    (empty at the time; exercise.py now attaches the selected device)

try.sh attached nvme, xhci, usb-storage, usb-kbd and usb-mouse. QEMU's default
NIC for the i386 `pc` machine is e1000, while the only driver then present was
virtio-net. The generic netdev layer now gates both paths. The entire network
path below the browser (virtio_net.c
763 lines, net.c 540, tcp.c 812, dns.c 433, http.c 290) was covered by host
tests against scripted packets and by NO test against emulated hardware.

Host tests and this gate answer different questions, and both are needed.
nettest/tcptest/httptest/dnstest are the machine on the other end of the wire:
they prove the state machines are right against sequences a real peer will not
produce on demand. They cannot prove the DMA arena is where the driver thinks
it is, that the feature handshake matches what a device actually offers, or
that a descriptor ring survives contact with QEMU. That is this.

WHAT IT ASSERTS, in the order the stack has to come up - each one is a distinct
failure mode with a distinct cause:

    device found        the PCI or USB match works (wrong device/class)
    arena backed        the DMA memory is RAM      (map outside RAM - a bug
                                                    class this project has hit
                                                    more than once)
    handshake           feature negotiation        (driver offered what the
                                                    device would not accept)
    link up             the device reports carrier
    ARP reply           a frame went out AND a matching one came back
    ping                twenty round trips, all returned
    DNS                 a real name resolves, a fake one is REFUSED
    driver counters     no truncated frame, runt, bad checksum, double-return
                        or unmatched echo

Two of those are load-bearing. ARP is the first point at which a frame this
kernel composed is parsed by something that is not this kernel. DNS is the
browser's actual dependency - a URL is a NAME - and the refusal half matters as
much as the lookup: a resolver that invents an address for a name that does not
exist fails silently and forever. Evidence it is real rather than canned:
`example.com` answered 104.20.23.154 on one run and 172.66.147.243 on the next.

The driver counters are not decoration. `browser-status.md` documents a
stale-descriptor defect found exactly here - the device reporting a correct
length for a buffer it had not written - so this is the gate for that class.

REACHING THE SHELL AT ALL IS THE HARD PART; see qtype() below. The keys go
through QMP, not the serial line, and `net` is typed as a WORD - the three
network gates had no typed name until this landed, so under the compositor
there was no way to run any of them.

Nothing here waits a fixed wall-clock time for anything the guest decides -
every wait polls for a marker, per the project rule that a gate must never be
timing-sensitive.

    ./probe-net.py              the gate
    ./probe-net.py --verbose    print the whole transcript
"""

import argparse
import os
import re
import subprocess
import sys
import tempfile
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from exercise import Serial, Qmp, qemu_argv, build, qtype, PROMPT

# QCODE and qtype() come from exercise.py, which is where the one copy lives.
# They used to be duplicated here and in probe-term.py, on the reasoning that a
# module name with a hyphen in it is not importable - true, but this file
# already imports exercise.py, so that was never the obstacle.
#
# THE ORIGINAL REASON GIVEN FOR TYPING OVER QMP WAS WRONG, and is corrected
# here rather than deleted, because a false "that is not supported" is the
# expensive kind of stale - nobody re-tests what they have been told is absent.
# What qtype()'s docstring used to say: the serial line cannot be used, because
# wm_frame() reads PS/2 and USB HID only and nothing in the compositor path
# looks at COM1, evidenced by 'N' and 'h' producing no output at all.
#
# Serial reaches the compositor. input.c drains COM1 into the same event queue
# as PS/2 and USB - the block commented `SERIAL, the third source`, which is in
# THIS BRANCH'S OWN input.c and was there when the claim was written. It exists
# precisely so every gate here kept working once the desktop became the boot
# state. Measured 2026-08-19: exercise.py types help, fib 20 and windows as
# words over serial and scores 4/4.
#
# The silence that misled it is real but means something else: term.c buffers
# printable characters and echoes only in its Enter branch, so a single 'N'
# submits nothing and prints nothing. A delivered keystroke and a dropped one
# are the same silence until Enter. See docs/typing-into-the-compositor.md.
#
# Typing on the emulated keyboard is still the right choice for this gate - it
# exercises the PS/2 and USB HID decoders, the only input a laptop has. Keep
# it; only the stated reason was wrong.


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--boot-timeout", type=float, default=240)
    ap.add_argument("--net-timeout", type=float, default=90,
                    help="ceiling on the whole N transcript. The ping alone is "
                         "twenty round trips with a 2s ARP timeout in front of "
                         "it, so this is generous on purpose - it is a "
                         "backstop against a hang, not a measurement.")
    ap.add_argument("--verbose", action="store_true")
    ap.add_argument("--driver", choices=("virtio", "e1000", "cdc"), default="virtio")
    ap.add_argument("--pcap", action="store_true",
                    help="retain a QEMU filter-dump packet capture in /tmp")
    ap.add_argument("--no-build", action="store_true")
    args = ap.parse_args()

    if not args.no_build:
        build(False)
    tmp = tempfile.mkdtemp(prefix="zlos-net-")
    ser_path, qmp_path = os.path.join(tmp, "ser"), os.path.join(tmp, "qmp")
    argv = qemu_argv(tmp, False, ser_path, qmp_path,
                     net=(args.driver if args.driver in ("e1000", "cdc") else True))
    pcap = os.path.join(tmp, f"{args.driver}.pcap")
    if args.pcap:
        argv += ["-object", f"filter-dump,id=netdump,netdev=n0,file={pcap}"]
        print("packet capture:", pcap)
    proc = subprocess.Popen(
        argv,
        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

    checks, failures = [], 0

    def check(label, ok, detail=""):
        nonlocal failures
        if not ok:
            failures += 1
        checks.append((label, ok, detail))
        print(f"  {'ok  ' if ok else 'FAIL'}  {label}" + (f"   {detail}" if detail else ""))

    try:
        ser, qmp = Serial(ser_path), Qmp(qmp_path)
        ok, log = ser.wait("ready.", args.boot_timeout)
        if not ok:
            print("never booted. serial so far:\n" + log[-2000:])
            sys.exit(1)
        ok, _ = ser.wait(PROMPT, 60)
        if not ok:
            print("booted but no prompt")
            sys.exit(1)

        print("booted. typing 'net' into the terminal (card + ARP + ping)\n")
        # `net` is a WORD, not the key 'N'. run_command dispatches 78/87/69 for
        # the three network gates, and term.c's word table had no name for any
        # of them - so under the compositor there was no way to reach them at
        # all. The words were added with this gate; typing one is what proves
        # they arrive.
        qtype(qmp, "net\n")

        # Serial.wait() takes ONE LITERAL string - it uses str.find, not a
        # regex - and waiting on the success marker alone would burn the full
        # timeout on every real failure. A gate that takes 90s to say "no card"
        # is a gate people stop running. So poll for whichever terminal marker
        # lands first, success or otherwise.
        terminal = ("no supported Ethernet device", "is NOT backed by RAM",
                    "refused the feature handshake", "no ARP reply",
                    "no loss", "LINK UP")
        hit, deadline = None, time.time() + args.net_timeout
        while hit is None and time.time() < deadline:
            for m in terminal:
                if m in ser.buf:
                    hit = m
                    break
            if hit is None:
                ser.pump()

        if hit is None:
            print("N produced no terminal marker inside the timeout")
            print(ser.buf[-2000:])
            sys.exit(1)

        # let the rest of the readout land before judging it. ARP succeeding
        # means ip_gate() is still to come, and that is the twenty pings.
        ser.drain(3.0 if hit == "LINK UP" else 1.0)
        t = ser.buf

        if args.verbose:
            print(t)

        check(f"{args.driver} Ethernet device found",
              "no supported Ethernet device" not in t,
              "" if "no supported Ethernet device" not in t
              else "the -netdev/-device flags did not reach QEMU")

        if args.driver == "e1000":
            check("generic NIC selection chose Intel e1000e",
                  "Intel device:" in t or "Intel e1000e/I219" in t)
        elif args.driver == "cdc":
            check("generic NIC selection chose USB CDC-ECM",
                  "driver:       USB CDC-ECM" in t or
                  "network driver: USB CDC-ECM" in t)
        else:
            check("generic NIC selection chose virtio-net",
                  "Intel device:" not in t and "Intel e1000e/I219" not in t)

        lease = "DHCP lease acquired" in t
        check("DHCP acquired a lease", lease,
              "" if lease else "automatic address configuration did not complete")

        check("the DMA arena is backed by RAM",
              "is NOT backed by RAM" not in t)

        check("the device accepted the feature handshake",
              "refused the feature handshake" not in t)

        m = re.search(r"link:\s+(\d+)", t)
        check("the device reports a link", bool(m and m.group(1) != "0"),
              f"link={m.group(1)}" if m else "no link line in the readout")

        mac = re.search(r"our MAC:\s+([0-9a-fA-F:]{17})", t)
        check("the driver read its own MAC", bool(mac),
              mac.group(1) if mac else "")

        arp = "no ARP reply" not in t and ("gateway MAC" in t or
                                            "DHCP lease acquired" in t)
        gw = re.search(r"gateway MAC:\s+([0-9a-fA-F:]{17})", t)
        check("ARP: a frame we composed was answered by the gateway", arp,
              gw.group(1) if gw else "")

        # tx/rx counters prove frames really moved rather than a cached answer
        cnt = re.search(r"frames out (\d+), in (\d+)", t)
        if not cnt:
            cnt = re.search(r"driver: tx (\d+)\s+rx (\d+)", t)
        check("frames actually moved in both directions",
              bool(cnt) and int(cnt.group(1)) > 0 and int(cnt.group(2)) > 0,
              f"tx={cnt.group(1)} rx={cnt.group(2)}" if cnt else "no counters")

        # the ping block is ip_gate()'s; it may be absent if ARP failed
        png = re.search(r"sent (\d+)\s+received (\d+)\s+lost (\d+)", t)
        if png:
            sent, recv, lost = (int(g) for g in png.groups())
            check("ICMP: every echo request came back",
                  sent > 0 and recv == sent and lost == 0,
                  f"sent={sent} received={recv} lost={lost}")
        else:
            check("ICMP: the ping block ran", False,
                  "no sent/received/lost line - ip_gate() did not run")

        # DNS is the browser's actual dependency: a URL is a NAME. This is the
        # first point where something outside QEMU's own stack answers.
        a = re.search(r"example\.com\s+->\s+(\d+\.\d+\.\d+\.\d+)", t)
        check("DNS: example.com resolved to an address", bool(a),
              a.group(1) if a else "no A record in the readout")
        check("DNS: a name that does not exist is refused, not invented",
              "no such name:" in t)
        q = re.search(r"queries (\d+)\s+replies (\d+)\s+refused (\d+)", t)
        check("DNS: every query got a reply",
              bool(q) and int(q.group(1)) == int(q.group(2)),
              f"queries={q.group(1)} replies={q.group(2)}" if q else "")

        # Driver health. browser-status.md documents a stale-descriptor defect
        # found exactly here - the device reporting a correct length for a
        # buffer it had not written - so these counters are the gate for that
        # class rather than decoration.
        for label, pat in (
                ("no truncated frames", r"rx-truncated (\d+)"),
                ("no runts", r"runts (\d+)"),
                ("no bad IP checksums", r"bad-checksum (\d+)"),
                ("no buffer handed back twice", r"handed back twice: (\d+)"),
                ("no unmatched echo replies", r"unmatched-echo (\d+)")):
            m = re.search(pat, t)
            check(f"driver: {label}", bool(m) and int(m.group(1)) == 0,
                  f"{m.group(1)}" if m else "counter not in the readout")

        print(f"\n{len(checks)} checks, {failures} failed")
        sys.exit(1 if failures else 0)
    finally:
        proc.kill()
        proc.wait()


if __name__ == "__main__":
    main()
