# WiFi and Bluetooth on zlOS — the actual plan

Roy asked for both. This is the build plan, not a feasibility argument.

**It also corrects two things I got wrong in `what-is-actually-impossible.md`
and `beyond-the-kernel.md`, both written the same day.** Those documents listed
Bluetooth as a vendor-gated wall and put the iwlwifi blob at 2–3 MB. Measured on
this machine:

| Claim | Reality |
|---|---|
| "Bluetooth is a firmware-blob wall" | **Wrong.** BT on this laptop is a **standard USB device** speaking a **published open standard** (HCI). It is one of the *more* tractable subsystems on the board |
| "iwlwifi blob is 2–3 MB" | **1.3 MB** (`iwlwifi-Qu-b0-hr-b0-77.ucode`, 1,406,572 bytes). Still larger than the 1.07 MB kernel, but I overstated it |
| "WiFi needs a signed blob you can't get" | **Wrong for the right hardware.** The AR9271's firmware is **50 KB and open source** |

---

## Bluetooth — much closer than it looked

### What is actually on this laptop

```
lsusb        → Bus 001 Device 004: ID 8087:0026 Intel Corp. AX201 Bluetooth
lsusb -t     → Class=Wireless, Driver=btusb
```

**Bluetooth is on USB, not CNVi.** The WiFi half of the AX201 is welded into the
chipset; the Bluetooth half is an ordinary USB device on the bus zlOS already
drives. That is the whole difference.

And it is a **standard class device** — the descriptors are exactly what the
Bluetooth SIG specifies for every BT dongle ever made:

```
bInterfaceClass       224  Wireless
bInterfaceSubClass      1  Radio Frequency
bInterfaceProtocol      1  Bluetooth
  EP 1 IN    Interrupt   <- HCI events
  EP 2 OUT   Bulk        <- ACL data out
  EP 2 IN    Bulk        <- ACL data in
  (control endpoint 0)   <- HCI commands
  EP 3 IN/OUT Isochronous <- SCO voice, alt settings, ignore for now
```

### Why that matters so much

**HCI — the Host Controller Interface — is a published open standard.** It is
not reverse-engineering. The Bluetooth SIG specification is free to download and
every dongle on earth implements the same commands. There is no undocumented
vendor protocol here, which is the exact opposite of the iwlwifi situation.

**And zlOS already has all three transfer types it needs.** `xhci.c` implements
control (enumeration), interrupt IN (the HID keyboard) and bulk IN/OUT
(`configure_bulk()` at `xhci.c:1709`, written and proven for mass storage).
Bluetooth needs precisely those three and nothing else.

So the transport layer — usually the hardest part of a new subsystem — is
mostly already written.

### The one catch, and how to sidestep it

Intel's BT controller wants a firmware upload: `intel/ibt-0040-0041.sfi`,
**704 KB**, over HCI vendor commands.

Two options:

- **Ship the Intel blob.** It is redistributable and the upload path is a
  documented HCI vendor-command sequence. ~300 lines.
- **Use a CSR-based USB dongle** (~£8). CSR8510 and similar carry firmware in
  **ROM** — plug it in, it answers HCI commands immediately, **zero firmware
  upload**. This is the clean-room path and it means the BT stack can be
  developed and proven *before* touching Intel's blob at all.

**Recommendation: build against a CSR dongle, add Intel firmware upload later.**
It removes a whole failure mode from the bring-up.

### Build order and sizes

| Step | Lines | Note |
|---|---|---|
| 1. HCI-over-USB transport | ~400 | Mostly wiring `xhci.c`'s existing endpoints. **First milestone: read the controller's BD_ADDR** |
| 2. HCI command/event layer | ~1,200 | Command queue, event dispatch, link control |
| 3. Inquiry + scan | ~400 | **Second milestone: zlOS lists nearby devices on screen.** Genuinely demoable |
| 4. L2CAP | ~1,500 | The multiplexing layer everything else sits on |
| 5. SDP (service discovery) | ~1,000 | Needed before you can connect to anything usefully |
| 6. Pairing / Secure Simple Pairing | ~1,500 | Needs **ECDH P-256 + AES-CMAC** — shares crypto with TLS. Do that once |
| 7. HID over L2CAP | ~800 | **Third milestone: a real Bluetooth keyboard types into zlOS** |
| | **~6,800** | to a working BT keyboard and mouse |
| *later:* A2DP audio | +3,000 | AVDTP + an SBC encoder, and it needs a sound driver first |

**Step 3 is the one to aim at first.** "zlOS scanned and found my phone" is a
real, visible, self-contained result off roughly 2,000 lines, and it needs no
pairing, no crypto and no L2CAP.

---

## WiFi — pick the hardware and it stops being a wall

The mistake in the earlier document was treating "WiFi" as one thing when the
answer depends entirely on which radio you talk to.

| Path | Firmware | Documented? | Lines | Verdict |
|---|---|---|---|---|
| **AR9271 USB dongle** | **50 KB, open source** | `ath9k_htc` is clean in-tree code | **~8,000** | **Do this one** |
| RTL8188EU USB dongle | in-driver blob | `rtl8xxxu`, well reverse-engineered | ~9,000 | Fine, second choice |
| ESP32 as a WiFi co-processor over UART/SPI | on the ESP | AT commands or SPI protocol | **~400** | **The cheat, and it works** |
| **AX201 (this laptop's own radio)** | **1.3 MB Intel blob** | **No** — only the iwlwifi source | ~40,000+ | Last. Kind-1 hard, not gated |

### Why AR9271 is the right target

The firmware is **`open-ath9k-htc-firmware`** — an actual open-source project,
not a signed vendor binary. It is already on this machine at
`/lib/firmware/htc_9271.fw`, **50,980 bytes**. That single fact removes the
entire "a vendor holds a key" objection: you can read the firmware, rebuild it,
and understand what the hardware is doing. Nothing else in the WiFi space offers
that. Dongles are ~£12 on eBay.

### Build order and sizes (AR9271)

| Step | Lines | Note |
|---|---|---|
| 1. USB enumeration + bulk pipes | ~300 | Again, `xhci.c` already has this |
| 2. Firmware upload | ~200 | 50 KB over the bulk pipe |
| 3. WMI command layer | ~1,500 | The chip's host↔firmware protocol, and `ath9k_htc` documents it by example |
| 4. TX/RX of raw 802.11 frames | ~1,200 | **Milestone: capture beacon frames.** Monitor mode is the free first win |
| 5. Scan + parse beacons | ~600 | **Milestone: zlOS lists nearby SSIDs.** No crypto needed |
| 6. Auth + association | ~1,200 | Open networks work at the end of this |
| 7. WPA2-PSK 4-way handshake | ~1,500 | PBKDF2 + HMAC-SHA1 + AES-CCMP |
| 8. Crypto primitives | ~1,500 | **Shared with TLS.** Write once, use twice |
| | **~8,000** | to associating with a real WPA2 network |

**Step 5 is the demo.** "zlOS scanned the air and printed my SSID list" needs no
crypto, no association, no IP stack — about 3,800 lines, and it is unmistakably
WiFi working.

### The ESP32 cheat, stated honestly

An ESP32 module over UART or SPI does the entire radio stack itself and answers
simple commands. ~400 lines on the zlOS side and you have internet. It is
genuinely *less* impressive than driving a radio directly — you are using
somebody else's WiFi chip as a modem — but it is a legitimate ladder rung and it
unblocks the whole network stack, TLS and browser work *today* while the AR9271
driver is being written.

---

## Where this sits against everything else

Both of these need the same thing underneath: **the network stack from
`beyond-the-kernel.md` §2** (ARP/IP/UDP/TCP/TLS, ~10,000 lines). WiFi delivers
ethernet frames; everything above the frame is identical whether they arrived by
wire, dongle or radio.

Total for "zlOS has WiFi and Bluetooth and can open a web page":

| | Lines |
|---|---|
| Bluetooth to a working keyboard | ~6,800 |
| WiFi (AR9271) to WPA2 association | ~8,000 |
| Network stack to HTTPS | ~10,000 |
| Document browser | ~13,200 |
| *shared crypto, counted once* | −1,500 |
| | **~36,500** |

About three times the current hand-written kernel. Every line of it bounded,
specified, and testable — and two of the four have a visible demo inside the
first 3,000 lines.

**Ordering note.** None of this is blocked by the display work, and none of it
blocks the display work — different files, different hardware. But the modeset
is one caller away from done (`HANDOFF.md`: nothing arms `lt_armed`) and it is
the higher-value flex. Finish that, then start here.

## The corrected wall list

After this document, the genuinely vendor-gated list is down to **four**:

- Widevine/DRM video — cryptographically gated by design, nothing to build
- Cellular modem — signed firmware plus carrier certification
- Shipping a Secure-Boot-signed image for *other people's* machines
- GPU 3D acceleration on modern parts — signed GuC/HuC

**Bluetooth is off the list entirely.** WiFi is off it too, provided you choose
the radio rather than letting Intel choose it for you.
