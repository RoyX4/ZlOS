# Intel AX201 Wi-Fi: implementation and evidence boundary

This is the active internal-Wi-Fi track for the physical ThinkPad. It replaces
the old advice to skip AX201. The shortest route to usable packets is still the
already-implemented USB CDC-ECM phone-tether path, but the requested target is
the laptop's internal radio and work on that target has started.

## Exact target

Linux identifies the physical device as:

```text
PCI       0000:00:14.3
identity  8086:02f0, class 02:80
name      Intel 400 Series On-Package CNVi WiFi
subsystem 8086:0070, AX201 Harrison Peak
BAR0      0xea238000, 16 KiB
firmware  iwlwifi-QuZ-a0-hr-b0-77.ucode
```

This is not the I219. The I219 is a separate wired Ethernet MAC and, on this
ThinkPad, needs Lenovo's physical Ethernet adapter before carrier can exist.
AX201 is the internal Wi-Fi radio and does not need that adapter.

## Implemented now: stage 0, read-only transport inventory

`iwlwifi.c` accepts only the exact `8086:02f0`, class `02:80` device. It resolves
the full 64-bit BAR0 and reads ten direct PCI CSR values:

- hardware-interface configuration;
- interrupt status and mask;
- GPIO, reset and general-purpose control state;
- hardware revision and RF ID;
- the two raw MAC-address CSR words at `0x380`.

The probe does **not** call `pci_enable`, enable bus mastering, acknowledge an
interrupt, write reset, request MAC access, ring a doorbell, upload firmware or
associate. It accepts only hardware revision type `0x351` (QuZ) and rejects an
unmapped/all-ones BAR. `hosttest/iwlwifitest.c` proves the exact identity filter,
64-bit BAR composition, ten bounded reads and zero device writes: **30/30**.

The command is manual:

```text
wifi
```

It records durable milestones 220 before the probe and 221 after it. If 220 is
present in ZLLOG and 221 is absent, the stop is inside the supposedly read-only
CSR access. The command is deliberately not called during boot. It prints
`transport identified; Wi-Fi is NOT connected yet` because inventory is not a
network connection.

The current known-safe USB image does not contain this experimental command and
must remain untouched until its pending desktop boot succeeds. Only after that
baseline is read back should a new image containing stage 0 be flashed.

## Implemented now: stage 1, exact API-77 artifact parser

The same source now contains an allocation-free, bounds-checked parser for the
upstream Intel TLV container. It validates the 88-byte header, magic, API 77,
every TLV header/payload/alignment boundary, section counts and the required
runtime image. Secure runtime/init sections are exposed as bounded
`{offset,data,length,type}` views; no section is copied or executed.

The target artifact is fixed to:

```text
file    iwlwifi-QuZ-a0-hr-b0-77.ucode
size    1,406,716 bytes
sha256  69ca0913ccca609dedff5e30b1d478482487fa14ad6b7be079f8cc856ac26cac
header  release/core74::f39cc7f9
build   0xf39cc7f9
layout  181 TLVs, 51 runtime sections, 1,405,180 payload bytes
```

The cryptographic allowlist is checked before the parser result can authorize a
future upload. A structurally valid but different API-77 image is refused until
it is deliberately reviewed and promoted. The focused suite now passes **54/54**,
including malformed magic/API/length/alignment inputs, the exact installed
image, and a one-byte-corrupted-image hash refusal.

The firmware is **not embedded in a zlOS image and is not uploaded**. Stage 1
removes parser/artifact ambiguity; it does not cross the hardware-write gate.

## Why AX201 is a large driver

Above the direct PCI registers, AX201 is firmware-driven. Upstream Linux's
22000-family configuration uses firmware API 77 with a single supported version
and the `iwlwifi-QuZ-a0-hr-b0-77.ucode` image. Firmware boot is only the start:
the host must build DMA queues, wait for ALIVE, load PNVM/NVM state, configure
PHY/MAC contexts, scan, authenticate, associate, complete WPA2 key exchange and
move 802.11 frames into the existing Ethernet/IP stack.

The repository already owns the upper network stack and WPA2 building blocks in
`crypto.c`. What is absent is the Intel transport and managed-mode 802.11 state
machine. That is `nobody has written it yet`, not `ZL cannot do it`.

## Ordered gates

Every device-writing stage stays manual until its physical transcript returns.
No later stage may be called automatically during boot merely because an earlier
one passed in a host harness.

| Gate | Deliverable | Proof required before promotion |
|---|---|---|
| 0 | exact PCI + read-only CSR snapshot | host 30/30, then physical 220 -> 221 and printed QuZ values |
| 1 | API-77 artifact/TLV parser and bounds checks | **implemented: 54/54**, exact installed image parsed and SHA-256 allowlisted |
| 2 | power/reset, interrupt and DMA queue transport | fake-MMIO write transcript; every wait bounded; manual physical return marker |
| 3 | firmware upload and ALIVE | exact ALIVE notification, firmware version, timeout/error persisted to ZLLOG |
| 4 | PNVM/NVM and regulatory/PHY configuration | valid hardware MAC and channel table; no transmit yet |
| 5 | command, RX and TX queues | scripted completions, ownership and reset recovery; counters exposed |
| 6 | passive/active scan | nearby SSIDs/channels printed on the ThinkPad; no association claim |
| 7 | open auth + association | association response and carrier state on a controlled access point |
| 8 | WPA2-PSK/CCMP | scripted four-way-handshake vectors, replay rejection, then controlled AP proof |
| 9 | `netdev` integration | DHCP, DNS, 20/20 ICMP, TCP/TLS/browser and disconnect/reconnect on physical radio |

## Current evidence label

As of 2026-08-23: **stages 0 and 1 are implemented and host-tested in the
isolated `codex/audit-trail` checkout. The focused suite passes 54/54, all four
kernel source targets pass the injected/removed source gate, and native UEFI
boot regression is green. Physical AX201 execution is not run. Firmware is not
embedded or loaded. Reset/DMA/ALIVE, scan, association, WPA2 and packets are not
implemented. Wi-Fi does not yet work in zlOS.**

The next user action is still the pending safe-image boot, not an AX201 test.
That separates the already-fixed I219 boot hang from any later Wi-Fi result.
