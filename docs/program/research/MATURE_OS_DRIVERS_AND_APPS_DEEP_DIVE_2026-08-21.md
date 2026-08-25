# Mature OS drivers and applications deep dive — 2026-08-21

This is the driver, service, application, utility, game, and user-facing-feature
expansion of the eight-repository mature-OS audit. It is intentionally more
granular than [`MATURE_OS_AUDIT_2026-08-21.md`](MATURE_OS_AUDIT_2026-08-21.md)
and incorporates the corrections in
[`MATURE_OS_REFUTATION_AND_OMISSIONS_2026-08-21.md`](MATURE_OS_REFUTATION_AND_OMISSIONS_2026-08-21.md).
The purpose is clean-room product and architecture research for zlOS. No source
was copied and no reference checkout was modified.

## Evidence contract and scope

The audited commits are Brook `76c3155394d325855019271f2bc3e33c951a6b5e`,
Astral `465437a974480cbf9f2eb9c190caec81f22919e3`, banan-os
`c915c064e8e42c4fa2e11754baf6c8dc6fb215ef`, LemonOS
`15f607e8be86fcaf9c86a68c91915e820c9e935e`, SerenityOS
`3d83e4509fd20d7438e1ae8470ffe668c136229c`, MollenOS
`f7d88d4d6f77528aaad8f7840b0b33f1d99add85`, hhuOS
`39bf35c6b98bf8665690349579a698a3478be815`, and duckOS
`3634e410c359c97e3b8d038fbffcdd07eddf34d8`.

I read the active build graphs, driver/device registration or discovery paths,
service and application target lists, package/port manifests, CI workflows,
tests, documentation, and representative implementations. “Every” below means
every meaningful first-party device class and shipped target discoverable from
those graphs, plus every named third-party application recipe. It does not mean
that every vendored line was re-audited.

Evidence labels:

- **S** — substantive source exists.
- **B** — the active default build graph reaches it.
- **T** — a focused test source or configured test target covers it.
- **CI** — inspected CI actually invokes the relevant test/runtime path.
- **CI-build** — CI compiles/packages the target but does not execute its
  behavior; this is deliberately weaker than **CI** runtime evidence.
- **D** — disabled, commented out, conditional on absent external input, or not
  reached by the default build.
- **3P** — third-party, vendored, submodule, port, or binary-package input; it
  is not evidence of a native implementation.
- **R?** — no runtime observation was made in this pass.

No repository was built or booted for this expansion. Therefore even `S/B/T/CI`
is not a fresh runtime or hardware receipt. A screenshot, README claim, recipe,
or filename never upgrades a row beyond what its actual graph establishes.

Primary reachability evidence:

| Repository | Driver/build evidence | Application/service/package evidence | Test/CI evidence |
|---|---|---|---|
| Brook | `src/kernel/CMakeLists.txt`, `src/kernel/drivers/CMakeLists.txt`, `src/drivers/DOCS.md` | `scripts/build_apps.sh`, `scripts/build_all.sh`, `scripts/create_nix_disk.sh`, `tools/DOCS.md`, individual Nix derivations | `src/tests/{host,}/CMakeLists.txt`, `scripts/run-tests.sh`, `scripts/e2e_wayland_input.sh` |
| Astral | `kernel-src/Makefile`, `kernel-src/{io,fs}`, init/auth registries | root `Makefile`, `recipes/`, `distro-files/`, `tools/`, `service-scripts/` | no native workflow or project test graph found |
| banan-os | `kernel/CMakeLists.txt`, kernel device-class directories | `userspace/programs/CMakeLists.txt`, `ports/`, individual program targets | `userspace/tests/CMakeLists.txt`; no workflow found |
| LemonOS | `Kernel/CMakeLists.txt`, `Kernel/Modules/CMakeLists.txt` | `System/CMakeLists.txt`, `Applications/CMakeLists.txt`, IDL/interfaces | `.github/workflows/{ci,release}.yml`, application test sources |
| SerenityOS | `Kernel/CMakeLists.txt`, device/USB/PCI managers and registries | application/game/service/utility CMake files and service manifests | `.github/workflows/{serenity-template,lagom-template}.yml`, target runner and component tests |
| MollenOS | root/module CMake graph, driver/service YAML manifests | `services/CMakeLists.txt`, `apps/CMakeLists.txt`, `BUILDING.md` | `.github/workflows/{build,test}.yml`, host test configuration/runner |
| hhuOS | `cmake/device`, `cmake/filesystem`, static kernel composition | `cmake/application/CMakeLists.txt`, per-app targets and READMEs, `.gitmodules` | `.github/workflows/{build,release}.yml`, `ctest` app source |
| duckOS | `kernel/CMakeLists.txt`, architecture/device sources | service/application/coreutils CMake graphs, bundle resources | kernel test registry/custom QEMU target, `.github/workflows/build-os.yml` |

## Compressed result

The reference set contains far more *initialization paths* than complete driver
lifecycle implementations, and far more *application targets* than launch,
failure, persistence, accessibility, or hardware receipts. The best transferable
patterns are:

1. Serenity's common async device-request model, driver management layers,
   separate display connectors, isolated parser services, and coherent desktop
   application suite.
2. Mollen's service-owned device/filesystem/process policy, machine-readable
   driver match manifests, and read-only application image model.
3. Brook's loadable-module symbol gate, subsystem defect ledgers, host-side
   driver/state-machine tests, and end-to-end Wayland input probe.
4. banan's broad storage/USB/input coverage, explicit server split, and
   reproducible port recipes.
5. Astral's unusually wide modern hardware set plus a complete Unix/X11 ports
   environment, while keeping the kernel graph simple enough to inspect.
6. Lemon's generated interface contracts and small coherent GUI/system suite.
7. hhuOS's readable, centrally composed hardware graph and unusually rich
   educational graphics/game workload.
8. duckOS's compact full desktop stack: typed River IPC, Pond compositor,
   Quack mixer, application bundles, file associations, and a usable set of
   native desktop programs.

The shared failure pattern is equally clear: probe succeeds, resources are
claimed, normal I/O works, but reset, timeout, cancellation, hot-unplug,
interrupt teardown, peer death, restart, and bounded queue ownership are absent
or weak. zlOS should treat those lifecycle transitions as part of the feature,
not post-feature hardening.

| Repository | Build-graph surface counted in this pass |
|---|---|
| Brook | 12 loadable modules; 21 normal static test/demo programs plus optional external CoreMark; layered native Wayland/package/diagnostic tools and ports |
| Astral | 5 native C administration/trace/profile targets, distro/session scripts and 307 third-party target recipes |
| banan-os | 62 default userspace programs, 16 unregistered test binaries and 128 port directories |
| LemonOS | 5 system targets, 22 application targets, 9 utility targets and 4 loadable kernel modules |
| SerenityOS | 51 graphical applications, 13 games, 29 services and 223 C/C++ utilities |
| MollenOS | 10 reachable driver/filesystem modules, 7 active services and 2 native utility apps; 1 additional NIC module and several sources are unreachable/disabled |
| hhuOS | one centrally composed static device graph and 45 application targets, including separately licensed ports/submodules |
| duckOS | 4 services, 12 graphical/VM application targets, `dsh` and 22 core utilities |

## Cross-project driver coverage

| Device family | Strongest source references | Material limits | Clean-room zlOS lesson |
|---|---|---|---|
| Firmware/platform | Serenity ACPI/DT/EFI/SMBIOS and three architectures; Astral uACPI; banan AML/EC/battery; hhu ACPI/APM/SMBIOS | Secondary-architecture and suspend/resume parity are uneven | One platform graph with ACPI/DT providers, typed discovered resources, suspend/resume callbacks, and architecture-parity receipts |
| PCI and interrupts | Serenity ECAM/legacy/VMD/MSI; banan PCI/APIC; hhu PCI/ISA/APIC/PIC; Mollen deviced PCI enumeration | Removal, BAR reallocation, MSI-X teardown, and IOMMU policy are generally incomplete | Resource broker owns BARs, IRQs, DMA domains and revocation; drivers receive bounded capabilities |
| Storage | Serenity AHCI/NVMe/SD/USB BOT+UAS/VirtIO; Astral AHCI/NVMe/VirtIO; banan AHCI/ATA/NVMe/USB MSC | Recovery, flush ordering, hot removal, cancellation, integrity injection | Common async block contract plus queue simulators, reset state machines, barriers, power-cut tests and per-device health telemetry |
| USB | Serenity UHCI/EHCI/xHCI + hub/HID/BOT/UAS; banan xHCI + hub/HID/MSC; Astral xHCI/hub/HID; Mollen UHCI/OHCI/EHCI + hub/HID/MSC | Mollen xHCI is unreachable; Lemon xHCI enumeration is commented out; detach paths are weak across projects | Controller-independent USB core, device-tree ownership, generation-tagged endpoints, cancel-on-disconnect, repeated attach/detach tests |
| Input | Serenity PS/2, USB/I2C HID, VirtIO; banan PS/2 and USB HID/joystick; Brook PS/2/VirtIO/xHCI; hhu PS/2 | Trust and exclusive-grab policy, layout state, composite HID, teardown | Normalize events in an input service; capability-gated grabs; descriptor fuzzing; per-seat state; no compositor-only secret syscall |
| Network | Serenity E1000/RTL8168/VirtIO/Cadence; Astral RTL8169/VirtIO; banan E1000/E1000E/RTL8169; duck E1000; hhu NE2000/RTL8139 | Protocol recovery and credential policy lag enumeration; hhu has UDP but no TCP | NIC ring model separated from deterministic ARP/IP/UDP/TCP state machines; packet loss/reorder/retransmit tests; namespace-aware socket policy |
| Graphics | Serenity Intel/Bochs/VMware/VirtIO/3dfx/simplefb; Brook Bochs/VirtIO; hhu VBE; duck Bochs/multiboot/RPi framebuffer | Display unplug/modeset rollback/fences/GPU isolation are scarce | Display connector contract, atomic modeset transaction, software fallback, buffer/fence ownership, virtual and physical display receipts |
| Audio | Serenity AC97/HDA + AudioServer; banan AC97/HDA + AudioServer; Brook HDA; duck AC97 + Quack; hhu SoundBlaster/PC speaker | Lemon HDA is largely stubbed; device loss and stream backpressure are weak | Hardware endpoint driver behind restartable mixer, negotiated formats, bounded rings, per-stream volume, underrun telemetry, disconnect recovery |
| Serial/timers/pseudo-devices | All have some UART/console/timer/pseudo-device support; Serenity has VirtIO console and multiple RTC/timer providers | These are often treated as boot scaffolding, not lifecycle-managed drivers | Register them through the same ownership/observability model; console fallback and clock-source failover need explicit state |

## Cross-project application-platform coverage

| Repository | Native/shared application platform | External platform layer | Clean-room lesson |
|---|---|---|---|
| Brook | kernel window/compositor syscalls, Wayland relay, small native Wayland clients, crash-dump helper | musl, Wayland/Cairo/FFmpeg/Nix/Weston/NetSurf and game ports | A compatibility bridge can unlock apps quickly, but the compositor and package parser remain high-trust boundaries |
| Astral | small admin/trace/profile utilities and kernel POSIX surface | mlibc, X.Org, GTK3, Qt6, SDL2, GStreamer, WebKitGTK, Wine and XBPS recipes | A ports ecosystem is a product layer; preserve a native recovery/diagnostic layer underneath it |
| banan-os | LibGUI widgets, LibImage, LibFont, LibInput, LibAudio/loaders, LibClipboard, LibELF, LibDEFLATE and LibQR; dedicated window/audio/clipboard servers | large X/GTK/SDL/media/game ports tree | Shared libraries plus small servers yield a coherent desktop; move hostile decoders out of client processes |
| LemonOS | LibLemon core/filesystem/graphics/IPC/system/shell APIs, LibGUI widgets and interface compiler | mlibc plus FFmpeg/LibreSSL/libpng/ncurses and other patches | Generated interfaces are the seam; add lifecycle/capacity semantics to the generator |
| SerenityOS | native GUI/Gfx/Desktop/Core/IPC/IDL, Web/JS/Media/PDF/ImageDecoder, Audio, GL/SoftGPU, SQL, TLS/SSH, Unicode/Locale, VT/Shell and many domain libraries | ports exist outside this focused first-party inventory | Broad apps remain maintainable when domain libraries and isolated services are reusable rather than copied per app |
| MollenOS | libc/libm/libos/libddk/libusb, generated Gracht IPC contracts, service APIs, PE loader and VaFS packaging | several bundled/submodule runtime libraries and external `vali-userspace` | A small kernel checkout can define strong system seams, but external GUI claims must stay external evidence |
| hhuOS | Lunar UI, Pulsar 2D/3D/audio/game framework, utility graphics/sound/network libraries, TinyGL/PortableGL | multiple emulator/game/rendering submodules | A workload-oriented library stack drives hardware maturity; provenance must remain per component |
| duckOS | libapp bundles/launch, LibUI widgets/layout/file views, LibPond, typed LibRiver IPC, LibSound, LibGraphics/3D, LibTerm/TUI, LibSys/Debug | Uxn is an embedded VM target rather than the base app platform | This is the most compact coherent app framework; add capability and protocol versioning without losing that simplicity |

## IanNorris/Brook

### Driver inventory

Brook's default kernel target statically includes PCI, ACPI/APIC, FAT/ext2/ext4
VFS adapters, GPT, VirtIO block, input/display/audio/network cores, RTC/KVM
clock, watchdog, framebuffer terminal, compositor and window support
(`src/kernel/CMakeLists.txt`). It also builds **12** relocatable modules through
`src/kernel/drivers/CMakeLists.txt`; `check_module_symbols` inspects the compiled
`.mod` imports against the kernel export table.

| Driver/class | Evidence and behavior | Reachability and gaps |
|---|---|---|
| VirtIO block | `virtio_blk_mod.cpp`; queue-backed block registration | **S/B/R?** module plus a separate statically linked `src/kernel/src/virtio_blk.cpp`; require explicit duplicate-owner exclusion, timeout/reset/cancel and flush evidence |
| PS/2 keyboard and mouse | Separate modules deliver scan/button/motion input | **S/B/R?**; unload must quiesce IRQs and input consumers |
| VirtIO input | Keyboard, mouse and tablet event transport | **S/B/R?**; driver ledger says unload does not unregister IRQs |
| VirtIO net | TX/RX virtqueues and interrupt-driven receive | **S/B/T/R?** via host TCP/network tests at stack level; missing IRQ teardown and RX concurrency hazard are documented |
| VirtIO RNG | Entropy acquisition through a virtqueue | **S/B/R?**; shared entropy-pool access is documented as unsynchronized |
| VirtIO GPU | Display resource/scanout support; its own `VIRTIO_GPU_DOCS.md` records scope | **S/B/R?**; require fence/resource lifetime, reset and modeset rollback proof |
| Bochs display | VBE-dispi mode setting and scanout | **S/B/R?**; BAR-size validation is explicitly missing |
| Intel HDA | PCI HDA, BDL/DMA PCM playback | **S/B/R?**; documented missing DMA publication barrier and incomplete play-state locking |
| xHCI | Host controller, enumeration, hub-facing USB, HID keyboard/mouse and mass storage | **S/B/R?**; large single module; teardown, cancellation and reconnect need separate gates |
| ext4 | lwext4 plus Brook OS/VFS adaptation | **S/B/R?**, substantially **3P**; do not count vendored lwext4 tests as Brook integration proof |
| MLFQ scheduler | Runtime-selectable scheduler policy rather than a hardware driver | **S/B/T/R?**; host suite loads scheduler policies dynamically |
| Built-in platform/core drivers | PCI, ACPI, APIC, RTC, KVM clock, watchdog, framebuffer/terminal, FAT/ext2, GPT, pseudo/ram disks | **S/B**, some host **T** for ext2/VFS/device registry; freestanding test ELFs are build targets, not registered CTest executions |

The best Brook idea is the combination of compiled-module symbol validation and
durable defect ledgers. The important correction is that `scripts/build_all.sh`
runs host CTest with `|| true`, so the convenient “full build” script can stay
green after a host test failure. The dedicated host CTest configuration remains
real; the wrapper's exit semantics do not.

### First-party user-facing inventory

Brook has several layers rather than one application registry:

| Layer | Exact targets/features | Evidence boundary |
|---|---|---|
| Kernel-resident recovery UI | framebuffer terminal, compositor/window primitives, debug overlay/panic screen and built-in shell | **S/B/R?**; useful when userspace fails, but normal desktop policy should remain outside the kernel |
| Static musl test/demo programs | `bbtest`, `crash_test`, `demo`, `echotest`, `exectest`, `fibonacci`, `forktest`, `futexstress`, `hello`, `mandelbrot`, `memtest`, `pipetest`, `primes`, `schedstress`, `scm_rights_test`, `stresstest`, `symlink_test`, `syscheck`, `sysinfo`, `wayland_test`, `wmtest`; optional external CoreMark | **S/B/R?** through `scripts/build_apps.sh`; `coremark_wrapper` is skipped and CoreMark requires a sibling source tree; these are mostly probes, not polished apps |
| Native desktop/server tools | `waylandd` (Wayland-to-kernel compositor relay), `brook-files` (two-pane browser, breadcrumb, sorting), `brook-edit` (save/search/tab/duplicate), `brook-player` (FFmpeg-backed audio/video), `brook-console` (filterable kernel-log viewer), `wayland-calc`, `brook-fbtest` | **S/B/R?** via individual Nix derivations and disk-image scripts; `brook-player` depends on 3P FFmpeg |
| Package tools | `nix-search`, `nix-install`, `nix-fetch`, `nar-unpack`, index generator | **S/B/R?**; binary-cache download, dependency closure, NAR extraction, profile links; untrusted archive/network handling makes this a security boundary |
| Media/diagnostic tools | `sinetest`, `wavplay`, `mp3play` (3P minimp3), profiler and panic/crash decoder/receiver tools | **S/B/R?** where disk scripts include them; several are host-side diagnostics rather than OS apps |
| Protocol/conformance packages | `wayland-smoke`, `wayland-shm-smoke`, `wayland-xdg-smoke`, `wraptest`, `ladybird-tests`; `brook-cursor-theme`, `brook-fonts` and `brook-weston-data` support client behavior | Mixed **S/B/3P/R?**; these are probes/support packages, and Ladybird is external; each should report protocol assertions rather than only launch |
| Games/ports | Brook platform layer for optional external DOOM; repository-hosted Quake II adaptation; a QuakeGeneric source/library tree; NetSurf staging/package; Weston demos (`flower`, `eventdemo`, `clickdot`, `multi-resource`, `presentation-shm`) | Mixed **S/B/D/3P/R?**. DOOM is skipped without external source/WAD; QuakeGeneric is not called by the normal full-build script; Quake/NetSurf/Weston are ports, not Brook-native app logic |
| Large external desktop experiments | Nix image recipes mention Mousepad, GIMP, NetSurf and Ladybird tests | **3P/R?**; package inclusion or screenshots are not current launch receipts |

Legacy/alternate standalone sources under `src/user` (`hello`, `cowsay`) and
`src/userspace` (`cow_test`, `dyntest`, `syscall_test`, `tcp_listen_test`,
`thread_test`) are not referenced by an inspected build graph and are **D**.
They must not be added to the shipped-program count.

`scripts/e2e_wayland_input.sh` is unusually valuable: it injects pointer events
and asserts the kernel → `waylandd` → Weston client path and first blits. It is a
local executable probe, not inspected CI. Conversely Brook's compositor syscalls
allow arbitrary callers to grab input, map scanout and map other windows' buffers;
the app breadth must not be mistaken for a security boundary.

### Brook lessons for zlOS

- Build module ABI/import checks into the normal artifact gate, then add unload,
  IRQ quiescence and stale-handle tests.
- Keep subsystem hazard files beside source but make severe open hazards block
  ledger promotion.
- Preserve small syscall/Wayland/storage probes, but make the orchestrator
  propagate their failures and emit an artifact-bound receipt.
- Package tooling is a hostile-input subsystem. Use signed metadata,
  content-addressed temporary files, path-safe extraction, transactional profile
  switching and rollback.

## Mathewnd/Astral

### Driver inventory

Astral's kernel Makefile compiles every C file beneath `kernel-src` except uACPI
tests, so the following are default-build reachable rather than aspirational
directory names.

| Driver/class | Source/feature | Reachability and gaps |
|---|---|---|
| Firmware/platform | uACPI integration, ACPI device glue, x86-64 APIC/SMP, PCI, framebuffer and console, PC COM serial | **S/B/3P/R?** for uACPI; init ordering cannot propagate failure, detect cycles, time out or roll back |
| Storage | AHCI/SATA, NVMe, VirtIO block, generic block layer | **S/B/R?**; add bounded waits, queue reset, flush/barrier, cancellation and hot-remove tests |
| USB/input | xHCI split into controller/ring/hub/transfer files; USB core/hub; HID and report parser | **S/B/R?**; no project-native test suite found; detach/cancel/generation behavior is the critical gap |
| Network | RTL8169 and VirtIO net; loopback; ARP, Ethernet, IPv4, UDP, TCP, local sockets | **S/B/R?**; no deterministic stack tests found; route/retransmit/window/error paths need proof |
| Audio | Intel HDA hardware driver and kernel audio layer; userland `sndiod` recipe/service | **S/B/R?**, sndio is **3P**; separate hardware evidence from mixer/service evidence |
| Filesystems | tmpfs, devfs, eventfs, ext2, FAT12/16/32, initrd, pipefs, sockfs, a small `abc` FS | **S/B/R?**; crash consistency, corrupt-image handling and mount teardown need targeted gates |
| Pseudo/terminal | pseudo-devices, PTY/TTY/terminal, poll/iovec/logging | **S/B/R?**; important application-enabling substrate, not hardware proof |

Astral also has a central authorization-listener API with system, filesystem,
credential, process and network scopes and deny-first aggregation. That is a
better clean-room reference than a string-based per-app permission list, but it
has no native test evidence here.

### Native system/user tools

The repository's first-party userland is deliberately small:

- `mount`: filesystem mounting utility.
- `netd`: hostname and network configuration daemon.
- `netinfo`: interface/network status utility.
- `prof`: profiling consumer.
- `systrace`: system-call tracing utility.
- `astral-bootstrap` and `astral-chroot`: distribution bootstrap helpers.
- `startwm` and `startnscde`: session launch helpers.
- OpenRC service scripts for `sndiod`, `sshd`, and `xdm`.

These are **S/B/R?** through recipes/distro files, except the daemons behind the
service scripts are **3P**. The default minimal image explicitly installs mlibc,
Bash, coreutils, OpenRC, Vim, Nano, mount, shadow/sudo, XBPS, network base,
fastfetch, disk utilities, `netinfo`, `systrace`, sndio and process utilities.

### Complete third-party recipe surface

There are **307** target recipes. They are integration/packaging evidence, not
307 Astral-native applications. Grouped without dropping names:

- **Base/toolchain/administration:** `agetty`, `astral`, `autoconf`,
  `automake`, `base`, `bash`, `binutils`, `bzip2`, `cmake`, `coreutils`,
  `diffutils`, `distro-files`, `dosfstools`, `e2fsprogs`, `fastfetch`, `file`,
  `findutils`, `freestnd-c-hdrs`, `freestnd-cxx-hdrs`, `gawk`, `gcc`, `gettext`,
  `git`, `grep`, `groff`, `gzip`, `hexdump`, `ksh93`, `less`, `limine`, `llvm`,
  `m4`, `make`, `make-ca`, `meson`, `mount`, `nano`, `nasm`, `ncurses`,
  `neofetch`, `ninja`, `openrc`, `parted`, `patch`, `perl`, `pkgconfig`, `prof`,
  `psutils`, `python`, `python3-psutil`, `python3-xdg`, `python3-yaml`, `qemu`,
  `rsync`, `ruby`, `sed`, `shadow`, `sudo`, `tar`, `unzip`, `util-macros`,
  `vala`, `vim`, `wget`, `xbps`, `xz`, `zlib`, `zstd`.
- **Networking/security/data:** `curl`, `cups`, `dbus`, `gnutls`, `iana-etc`,
  `irssi`, `libarchive`, `libevent`, `libgcrypt`, `libgpg-error`, `libidn2`,
  `libpsl`, `libsecret`, `libsoup`, `libtasn1`, `libunistring`, `nghttp2`,
  `net-base`, `netd`, `netinfo`, `nettle`, `openjdk17`, `openssh`, `openssl`,
  `p11-kit`, `sqlite`, `systrace`, `xdg-dbus-proxy`, `xdg-utils`.
- **Audio/video/image/text libraries and tools:** `alsa-lib`, `aspell`,
  `cairo`, `double-conversion`, `enchant`, `flac`, `fontconfig`, `fonts-dejavu`,
  `fontutil`, `freetype`, `fribidi`, `gdk-pixbuf`, `glib`, `glib-networking`,
  `graphene`, `gsettings-desktop-schemas`, `gst-plugins-base`,
  `gst-plugins-good`, `gst-plugins-sndio`, `gstreamer`, `harfbuzz`, `icu`,
  `imagemagick`, `imlib2`, `iso-codes`, `lame`, `lapack`, `lcms`, `libaom`,
  `libavif`, `libburn`, `libepoxy`, `libexpat`, `libffi`, `libiconv`,
  `libid3tag`, `libintl`, `libisoburn`, `libisofs`, `libjpeg-turbo`, `libmad`,
  `libogg`, `libpng`, `libsndfile`, `libstb`, `libtiff`, `libvorbis`, `libwebp`,
  `libxml`, `libxslt`, `libyaml`, `mocp`, `mpg123`, `openal`, `openjpeg`, `opus`,
  `pango`, `pixman`, `shared-mime-info`, `sndio`.
- **X11/desktop/graphics stack:** `adwaita-icon-theme`, `appres`,
  `at-spi2-core`, `editres`, `evtest`, `fvwm`, `glfw`, `glu`, `gtk3`,
  `hicolor-icon-theme`, `iceauth`, `libXau`, `libXdmcp`, `libice`, `libsm`,
  `libx11`, `libxaw`, `libxcb`, `libxcrypt`, `libxcursor`, `libxcvt`,
  `libxdamage`, `libdrm`, `libevdev`, `libxext`, `libxfixes`, `libxfont2`,
  `libxft`, `libxi`, `libxinerama`, `libxkbcommon`, `libxmu`, `libxpm`,
  `libxrandr`, `libxrender`, `libxshmfence`, `libxt`, `libxtst`, `libxv`,
  `libxxf86vm`, `listres`, `luit`, `mesa`, `mesa-demos`, `mkfontscale`, `mtdev`,
  `nscde`, `openmotif`, `pyqt6`, `pyqt6-sip`, `qtbase6`, `rgb`, `sdl2`,
  `sessreg`, `stalonetray`, `twm`, `viewres`, `wine`, `x11-utils`,
  `x11-xserver-utils`, `xauth`, `xbitmaps`, `xcb-proto`, `xcb-util`,
  `xcb-util-cursor`, `xcb-util-errors`, `xcb-util-image`, `xcb-util-keysyms`,
  `xcb-util-renderutil`, `xcb-util-wm`, `xclock`, `xcmsdb`, `xdm`, `xdotool`,
  `xdpyinfo`, `xdriinfo`, `xedit`, `xev`, `xeyes`, `xf86-input-evdev`,
  `xf86-video-fbdev`, `xfd`, `xfile`, `xfishtank`, `xfontalias`,
  `xfontmiscethipic`, `xfontmiscmisc`, `xfontsel`, `xgamma`, `xhost`, `xinit`,
  `xkbcomp`, `xkbfile`, `xkeyboardconfig`, `xkill`, `xlsatoms`, `xlsclients`,
  `xlsfonts`, `xmessage`, `xmodmap`, `xnedit`, `xneko`, `xorg-server`,
  `xorgproto`, `xprop`, `xrandr`, `xrdb`, `xrefresh`, `xscreensaver`, `xset`,
  `xsetmode`, `xsetpointer`, `xsetroot`, `xsettingsd`, `xstdcmap`, `xterm`,
  `xtrans`, `xvidtune`, `xvinfo`, `xwininfo`.
- **Applications/games/runtime demonstrations:** `ace-of-penguins`, `brogue`,
  `btop`, `doomgeneric`, `doomgeneric-x11`, `links`, `lwjgl`, `lwjgl3`,
  `mednafen`, `nyancat`, `os-test`, `tyrquake`, `webkitgtk`.
- **Support libraries not covered above:** `frigg`, `gmp`, `libatomic`,
  `jna`, `libfontenc`, `libgfortran`, `libgomp`, `libquadmath`, `libsmarter`,
  `libtool`, `libuuid`, `linux-headers`, `mlibc`, `mlibc-headers`, `mpc`, `mpfr`,
  `pcre2`, `readline`.

The recipe names above expose an important audit boundary: the source recipes
also mention items not present as final target recipes, and a recipe does not
prove its current binary downloads, install, launch, rendering, audio, or
persistence. zlOS should import Astral's ecosystem ambition only with per-port
install-and-launch receipts.

### Astral lessons for zlOS

- Match Astral's breadth only after one reusable driver lifecycle contract
  exists; otherwise every controller repeats the same missing recovery work.
- Preserve the small native admin-tool layer even if most desktop software is
  ported. Native diagnostics are how the system remains observable when ports
  fail.
- Port catalogue state must distinguish recipe parsed, source verified, built,
  installed, launched, smoke-tested, failure-tested and hardware-tested.

## Bananymous/banan-os

### Driver inventory

The kernel CMake source list makes the main hardware graph explicit and reaches
both x86-64 and i686 architecture glue. These are kernel-resident drivers, not
runtime-loadable modules.

| Driver/class | Source/feature | Reachability and gaps |
|---|---|---|
| ACPI/platform | ACPI/AML namespace, operation regions, embedded controller, battery system; APIC/PIC, CPUID | **S/B/R?**; battery data is exported to userland TaskBar; suspend/resume and EC failure recovery are not established |
| PCI/storage | legacy ATA/ATABus, AHCI controller/device, NVMe controller/namespace/queue, SCSI layer, partitions and disk cache | **S/B/R?**; broad real-hardware surface; queue reset, cancellation, flush ordering and removal remain the hard gates |
| USB | xHCI controller/device, USB manager/core, hubs, HID keyboard/mouse/joystick, mass-storage and USB-SCSI | **S/B/T/R?**; joystick/framebuffer probes exist, but compiled test programs are not a configured runner; detach and controller reset need proof |
| Input | PS/2 controller/keyboard/mouse/keymap and USB HID classes | **S/B/R?**; layouts are user-selectable with `loadkeys`; seat/grab authority and disconnect cleanup need contracts |
| Network | E1000, E1000E, RTL8169, loopback, ARP/IPv4, UDP, partial TCP, Unix-domain sockets | **S/B/R?** plus unregistered TCP/UDP/Unix-socket test binaries; TCP remains explicitly incomplete |
| Audio | AC97 and Intel HDA controller/function-group support, generic audio controller and `/dev/audioN` | **S/B/R?**; AudioServer discovers devices; require mixer/device reconnect and format negotiation tests |
| Graphics/terminal | boot framebuffer device, framebuffer/text terminals, virtual TTY, serial, PTY | **S/B/R?**; WindowServer supplies desktop composition, but graphics hardware acceleration is not present |
| Filesystems/pseudo-devices | ext2, FAT, tmpfs, devfs, procfs, USTAR init module, pipes/eventfd, null/zero/random/debug/framebuffer devices | **S/B/R?**; ext2/FAT corruption, cache flush and forced I/O failure are not CI-gated |
| Time/entropy | HPET, PIT, RTC and kernel random source | **S/B/R?**; clock failover and entropy provenance need explicit quality/health state |
| Banos loadable-driver ABI | privileged syscall loads relocatable ELF sections, resolves exported kernel symbols/relocations and invokes a driver descriptor; `driver-install` is its client | **S/B/R?**; unloading/refcounts, machine verification, full bounds/overflow validation, signature/provenance and symbol-conflict policy are TODO/incomplete; credential escalation also undermines the root check |

All of those sources are in the default kernel target. The 16 userspace tests
(`fork`, framebuffer, globals, joystick, mouse, shared mmap/library state,
`popen`, pthread, setjmp, sort, TCP, TLS, UDP, Unix socket, window) are only
build dependencies; their CMake file registers no CTest runner. The project has
no inspected CI workflow. Therefore “test source exists” is not “test passed.”

### First-party services and desktop applications

The default userspace graph builds **62** named programs. The user-facing set is:

| Target | Feature | Evidence/limits |
|---|---|---|
| `init` | username login, session environment and desktop/shell startup | **S/B/R?**; passwordless login and the `setreuid` escalation make this untrusted as an auth design |
| `DynamicLoader` | ELF dynamic-loading runtime | **S/B/R?**; application-enabling system component |
| `WindowServer` | window registry/composition, shared buffers, damage, input, wallpaper/config | **S/B/R?**; good SHM/damage pattern; server must own and validate dimensions, handles and authority |
| `TaskBar` | task buttons, launcher integration, clock and battery display | **S/B/R?**; useful example of `/dev/batteries` as observable device state |
| `ProgramLauncher` | searchable program launch prompt | **S/B/R?** |
| `ClipboardServer` | centralized clipboard service with client library | **S/B/R?**; add MIME types, size limits, history policy and session isolation |
| `AudioServer` | discovers `/dev/audioN`, mixes/serves application audio | **S/B/R?**; add bounded per-client rings and device-loss recovery |
| `Terminal` | GUI terminal using PTY/TTY support | **S/B/R?** |
| `Shell` | command parsing/execution, pipelines/redirection/environment | **S/B/R?**; separate from the GUI terminal |
| `ImageViewer` | GUI image viewing through LibImage | **S/B/R?**; image parsing belongs in a restricted decoder process |
| `snake` | terminal snake game | **S/B/R?** |
| `image` | terminal/image rendering utility | **S/B/R?** |
| `audio`, `audioctl` | play an audio file; inspect/control audio settings | **S/B/R?** |
| `dhcp-client`, `resolver`, `nslookup` | DHCP lease acquisition, name-resolution service, DNS query | **S/B/R?**; lease renewal/rebind and resolver restart behavior need tests |
| `http-server` | small HTTP server | **S/B/R?**; protocol parser/limits are a hostile-input boundary |
| `driver-install` | install a supplied driver/input file | **S/B/R?**; must be signed, atomic and capability-gated in zlOS |
| `loadfont`, `loadkeys` | load console font and keyboard layout | **S/B/R?** |
| `bananfetch`, `meminfo`, `top` | system summary, memory details and interactive process monitor | **S/B/R?**; strong observability seed |
| `sudo`, `id`, `whoami`, `nologin` | identity/elevation/session utilities | **S/B/R?**; `sudo` is not a security reference because the kernel credential path is exploitable |
| `poweroff`, `sync`, `kill` | shutdown, filesystem synchronization and process signaling | **S/B/R?** |

The complete native command set, including the above, is: `audio`, `audioctl`,
`AudioServer`, `bananfetch`, `basename`, `cat`, `cat-mmap`, `chmod`, `chown`,
`ClipboardServer`, `cp`, `dd`, `dhcp-client`, `dirname`, `driver-install`,
`DynamicLoader`, `echo`, `env`, `false`, `find`, `getopt`, `http-server`, `id`,
`image`, `ImageViewer`, `init`, `kill`, `less`, `ln`, `loadfont`, `loadkeys`,
`ls`, `meminfo`, `mkdir`, `mv`, `nologin`, `nslookup`, `poweroff`,
`ProgramLauncher`, `pwd`, `resolver`, `rm`, `Shell`, `sleep`, `snake`, `sort`,
`stat`, `sudo`, `sync`, `TaskBar`, `tee`, `Terminal`, `test`, `top`, `touch`,
`tr`, `true`, `u8sum`, `uname`, `whoami`, `WindowServer`, and `yes`.

This suite is notably coherent: it covers boot → login → launcher/taskbar →
window/clipboard/audio servers → terminal/shell → diagnostics and network
configuration, rather than only isolated demos.

The large Advent of Code 2023/2024/2025 source trees are explicitly commented
out in `userspace/CMakeLists.txt`; they are **D** workload/source material, not
default userspace applications.

### Third-party ports

The **128** port directories are package recipes/adaptations, not native app
implementations. The complete surface is:

`ClassiCube`, `SDL2_image`, `SDL2_mixer`, `SDL3`, `SDL_mixer`, `SpecSeek`,
`SuperTux`, `SuperTuxKart`, `bash`, `binutils`, `bochs`, `boost`, `butterscotch`,
`bzip2`, `ca-certificates`, `cairo`, `ccleste`, `cmake`, `curl`, `dbus`,
`dejavu-fonts-ttf`, `doom`, `expat`, `ffmpeg`, `fontconfig`, `freetype`,
`fribidi`, `gcc`, `gdk-pixbuf`, `git`, `glib`, `glm`, `glu`, `gmp`, `gtk3`,
`halflife`, `harfbuzz`, `icu`, `libICE`, `libSM`, `libX11`, `libXau`, `libXaw`,
`libXcomposite`, `libXcursor`, `libXdamage`, `libXdmcp`, `libXext`, `libXfixes`,
`libXfont2`, `libXft`, `libXi`, `libXinerama`, `libXmu`, `libXpm`, `libXrandr`,
`libXrender`, `libXt`, `libXtst`, `libarchive`, `libatk`, `libdrm`, `libepoxy`,
`libffi`, `libfontenc`, `libiconv`, `libjpeg-turbo`, `libmikmod`, `libogg`,
`libpng`, `libpthread-stubs`, `libsndfile`, `libspng`, `libtiff`, `libuv`,
`libvorbis`, `libwebp`, `libxcb`, `libxkbfile`, `libxml2`, `libxshmfence`,
`links`, `llvm`, `lua`, `lynx`, `make`, `mesa`, `mesa-glx`, `mpc`, `mpfr`,
`nano`, `nasm`, `ncurses`, `nyancat`, `openal-soft`, `openssh`, `openssl`,
`pango`, `pcre2`, `physfs`, `pixman`, `python3`, `qemu`, `quake2`,
`sdl12-compat`, `sdl2-compat`, `shared-mime-info`, `tar`, `tcc`, `tcl`,
`timidity`, `tinygb`, `tuxracer`, `util-macros`, `vim`, `xash3d-fwgs`,
`xbanan`, `xcb-proto`, `xclock`, `xeyes`, `xkbcomp`, `xkeyboard-config`,
`xorgproto`, `xtrans`, `xz`, `zlib`, `zsh`, and `zstd`.

The recipe framework pins sources, checksums and dependency relationships and
tracks patch/build hashes; that is the strongest packaging pattern in this set.
It still needs an artifact-bound install/launch receipt before any port is called
available.

### banan lessons for zlOS

- Reuse the coherent service/app composition and ports provenance pattern, not
  the credential model.
- Make batteries, storage health, network state and audio endpoints queryable
  through typed device properties consumed by TaskBar/System Monitor.
- Create one application manifest containing executable, icon, MIME handlers,
  requested capabilities, singleton policy, service dependencies and smoke test.

## LemonOSProject/LemonOS

### Driver inventory

| Driver/class | Source/feature | Reachability and gaps |
|---|---|---|
| Platform | ACPI, APIC, PCI, PS/2, SMP, serial, timer and framebuffer video console | **S/B/R?** in the kernel target |
| Storage | legacy ATA, AHCI, NVMe, GPT and partition devices | **S/B/R?**; recovery/timeout/cancel/flush tests are not present |
| Filesystems | in-kernel FAT32/tmpfs/TAR plus loadable ext2 module | **S/B/R?**; ext2 unmount is an assertion stub and truncate does not fully free blocks |
| Network | interface/adapter layer, IP sockets, UDP and TCP; loadable Intel 8254x module | **S/B/R?**; several socket blocking/interface-selection paths are TODO |
| Audio | kernel audio layer plus loadable AC97/HDA `pcaudio.sys` | **S/B/R?**; HDA reports hardware but master/output volume methods return zero, channel setting returns `ENOSYS`, and sample write is effectively a stub |
| USB | substantial xHCI controller/ring/event source | **S/B-but-disabled/R?**: file compiles into the kernel, but `Initialize()` has PCI enumeration commented out, so no controller instance is created |
| Modules | relocatable ext2, Intel 8254x, PC audio and test module | **S/B/R?**; useful loader boundary; unload/resource revocation not established |

The distinction “compiled but operationally unreachable” matters most for xHCI
and HDA. zlOS's feature ledger must represent `compiled`, `matched`, `initialized`,
`I/O observed`, `failure-injected`, and `recovered` separately.

### System services

| Target | Feature | Evidence/limits |
|---|---|---|
| `init.lef` / Lemond | system initialization and service launch | **S/B/CI-build/R?** |
| `netgov.lef` | network configuration/governor | **S/B/CI-build/R?** |
| `kmod.lef` | load/list/control kernel modules | **S/B/CI-build/R?** |
| `login.lef` | GUI login/session launch | **S/B/CI-build/R?**; unsalted SHA-256, digest logging, ignored credential-drop results and a successful `setgid` stub make it unsafe as a model |
| `lemonwm.lef` | window manager/compositor with damage and input paths | **S/B/CI-build/R?**; good typed GUI boundary, but validate buffers/regions and peer lifecycle |
| `FTerm` source | framebuffer terminal source directory | **D/R?**; no target in `System/CMakeLists.txt`, so do not count as shipped from this graph |

Lemon's IDL-generated service/interface model is the reusable behavior: the
protocol definitions and libraries give components a named boundary. The kernel
message endpoint's mismatched semaphore accounting can deplete sender credits,
so typed syntax alone does not solve capacity ownership, cancellation or peer
death.

### Applications and utilities

The application graph builds **22** executables and installs all 22:

| Target | User-facing feature | Evidence/limits |
|---|---|---|
| File Manager | graphical browsing and file operations | **S/B/CI-build/R?** |
| Shell and `lsh` | graphical desktop shell/menu and command-line shell | **S/B/CI-build/R?** |
| Terminal | resizable GUI terminal | **S/B/CI-build/R?** |
| TextEdit | graphical text editor | **S/B/CI-build/R?** |
| ImgView | resizable image viewer | **S/B/CI-build/R?**; isolate image decoding in zlOS |
| AudioPlayer | metadata-aware audio playlist/player using FFmpeg libraries | **S/B/CI-build/3P/R?** for codecs |
| VideoPlayer | FFmpeg-backed video playback | **S/B/CI-build/3P/R?** |
| LemonMonitor | process/system monitor | **S/B/CI-build/R?** |
| SysInfo | system information viewer | **S/B/CI-build/R?** |
| DeviceManager | device browser/manager | **S/B/CI-build/R?**; strong user-visible endpoint for driver state |
| Run | graphical command launcher | **S/B/CI-build/R?** |
| Welcome | onboarding/welcome application | **S/B/CI-build/R?** |
| Minesweeper and Snake | native GUI games | **S/B/CI-build/R?** |
| JSONDump | JSON inspection/dump utility | **S/B/CI-build/R?** |
| `steal` | OpenSSL-backed URL/network retrieval tool | **S/B/CI-build/3P/R?** |
| GraphicsTest and GUITest | rendering/widget demonstrations | **S/B/CI-build/R?**, not app acceptance tests |
| IPCTest and SignalTest | IPC and signal behavior probes | **S/B/CI-build/R?** |
| `tests.lef` | aggregate source-level application tests | **S/B/CI-build/R?**; its assertion helpers do not propagate failure and main returns zero, so it is false-green |

The separate utility graph adds **nine** installed commands: `cat`, `echo`,
`rm`, `ls`, `uname`, `hexdump`, `ps`, `playaudio`, and `lemonfetch` (a graphical
system-summary utility). `playaudio` is FFmpeg-backed; the remainder are native
Lemon command or UI source. The four `.li` service manifests register Lemond,
LemonWM, NetworkGovernor and the desktop Shell, providing deployment evidence
separate from compilation.

CI builds disk images in two jobs/configurations but does not boot them or run
the app/test targets. Application breadth is therefore build-reachable, not
runtime-gated.

### Lemon lessons for zlOS

- Generate both sides of every UI/service protocol, validators, trace decoders,
  fuzz corpus scaffolding and conformance tests from one zlIDL definition.
- Treat device manager, system monitor and welcome/onboarding as core OS apps,
  not polish: they expose driver state, failure and discoverability.
- Server queues need an explicit capacity owner, deadline, cancellation,
  disconnect cleanup, request identity and restart semantics.

## SerenityOS/serenity

Serenity has the broadest integrated driver and native application surface in
this set. Its kernel CMake list is also unusually useful audit evidence: common
drivers are named explicitly, then architecture-specific platform drivers are
appended under x86-64, AArch64 and RISC-V branches.

### Driver and device inventory

| Family | Built implementations | Evidence and lifecycle limits |
|---|---|---|
| PCI/bus | legacy and memory-backed host bridges, generic ECAM, PCI API/device/identifier, volume-management device, MSI on x86-64; OpenCores I2C and I2C HID; VirtIO PCI transport | **S/B/R?**; coherent discovery layer. Add hot-remove, rebalance, MSI-X teardown, IOMMU/DMA-domain and suspend/resume receipts |
| USB controllers/core | UHCI, EHCI, PCI/device-tree xHCI, root hubs, configurations, pipes/transfers and management | **S/B/T/R?**; USB source has tests through broader target test architecture, but no fresh per-controller runtime proof here; require cancellation and generation-safe reconnect |
| USB classes | HID and mass storage with both Bulk-Only Transport/SCSI and UAS | **S/B/T/R?**; BOT+UAS breadth is best in set; malformed descriptors, device loss and outstanding-command cancellation are essential gates |
| Input | PS/2 keyboard/mouse, USB HID keyboard/mouse, I2C HID, VirtIO input; x86 VMware mouse | **S/B/T/R?**; HID parsing library has host-testable structure. Add composite-device fuzzing, per-seat state and capability-gated grabs |
| Storage | AHCI ATA, NVMe poll/interrupt queues and namespaces, PCI SD host + SD card, RPi SD host, USB BOT/UAS, VirtIO block, partitions, loop devices | **S/B/T/R?**; common async request/management layers are the strongest pattern. Reset, flush, power loss and removal remain feature gates |
| Network adapters | Intel E1000, Realtek RTL8168, VirtIO net and Cadence GEM (including RPi RP1 GEM); loopback | **S/B/T/CI/R?** at stack/adapter test architecture level; current hardware receipts not inspected. Deterministic ring and packet-loss/reorder tests still belong in zlOS |
| Network protocols | Ethernet/ARP, IPv4/IPv6 structures, ICMP, local sockets, UDP, TCP, routing, MDIO, sysfs observability | **S/B/T/CI/R?**; one of the most testable stacks here; service/network namespace and recovery semantics still matter |
| GPU/display | Intel native display (G33-era plane/transcoder/PLL/GMBus path), Bochs/QEMU, VMware SVGA, VirtIO GPU/3D device, 3dfx Voodoo, simple/boot framebuffer and connector abstraction | **S/B/T/R?**; excellent connector/adapter split. Add atomic commit/rollback, fence ownership, display unplug, GPU reset and isolation |
| Audio | AC97, Intel HDA codec/controller/stream/ring/format, PC speaker and central audio management | **S/B/T/CI/R?** plus AudioServer; require device-loss recovery, negotiated stream format, bounded buffering and underrun/overrun receipts |
| Serial/console | 16550 ISA/PCI serial, VirtIO console/ports, PL011 and RPi mini-UART; virtual consoles, PTYs | **S/B/R?**; useful fallback paths. Make console failover observable and avoid treating debug serial as the only truth channel |
| Time/interrupts | x86 APIC/IOAPIC/PIC, APIC timer, HPET, PIT, RTC, TSC; ARM GICv2/v3, ARMv8 timer/PL031; RISC-V PLIC/timer; Goldfish RTC | **S/B/R?** per architecture; clock-source selection, drift, suspend/resume and failover need deterministic tests |
| Firmware/platform | ACPI, device tree, EFI protocols/services, SMBIOS; x86 PC BIOS mapping; ARM PSCI/RPi mailbox/GPIO/watchdog/fan/PWM/clocks; RISC-V SBI | **S/B/R?**; broadest architecture platform graph. Secondary-architecture target tests are skipped by the default runner despite CI labels |
| Entropy/security devices | VirtIO RNG, architecture entropy, random device, KCOV device/instances, `/dev/null`, zero, full, memory, device-control, self-TTY | **S/B/T/CI/R?** depending on configuration; KCOV is coverage tooling, not a production driver claim |
| Filesystems | ext2, FAT, ISO9660, Plan 9, FUSE, RAMFS, procfs, sysfs, devpts, dev-loop plus FIFO/inode watcher and mount namespaces | **S/B/T/CI/R?**; rich observable sysfs hierarchy. Filesystem corruption/power-loss/hot-storage behavior still needs artifact-specific gates |

The cross-cutting design is more important than any individual device: async
requests, storage management, display connectors, USB management, sysfs
registries and common device base classes let many drivers share behavior. zlOS
should copy that *shape*, then require every implementation to satisfy the same
lifecycle automaton.

### System and desktop services

The service graph contains **29** named directories. Six (`ConfigServer`,
`EchoServer`, `FileOperation`, `LookupServer`, `SSHServer`, `WebServer`) are also
configured for host/Lagom contexts; the remaining Serenity system services are
guarded by `SERENITYOS`.

| Service | Function | Boundary worth preserving or fixing |
|---|---|---|
| SystemServer | init/supervision and system-mode orchestration | Declarative service manifests and readiness; add dependency-cycle, restart-budget and degraded-mode receipts |
| LoginServer | login/session establishment | Session-scoped capabilities, credential verification and clean logout |
| LaunchServer | MIME/URL/program launch routing | One source of truth for handlers and requested capabilities |
| ConfigServer | centralized domain/group/key settings | Watchable typed settings, transactional write and schema/version migration |
| WindowServer | display, windows, composition, input and desktop coordination | Server-owned geometry/buffers; validate IPC and support compositor restart without app corruption |
| Taskbar | task list, app menu and desktop status UI | Treat as replaceable client, not privileged kernel policy |
| Clipboard | clipboard service | MIME-aware, bounded, per-session, ownership/death cleanup |
| NotificationServer | notification delivery and UI | Stable IDs, replace/update, actions, rate limits and quiet policy |
| AudioServer | stream mixing/output | Bounded per-client queues, negotiated formats and hardware reconnect |
| FileOperation | centralized copy/move/remove progress | Transaction/cancel/progress/conflict contracts and crash recovery |
| FileSystemAccessServer | brokered filesystem access for restricted clients | Strong pattern: capability-scoped portal rather than ambient path access |
| ImageDecoder | isolated image decoding | Strong hostile-parser pattern; add hard memory/time/output bounds and one-shot workers |
| RequestServer | restricted HTTP/network request broker | Capability-scoped network portal, redirect/TLS/cookie limits and cancellation |
| WebContent | isolated browser content process | Parser/render sandbox; documentation may describe stronger future process splits than current code |
| WebWorker | worker execution process | Isolate long-running web work and terminate with page/session |
| WebDriver | browser automation protocol service | Useful end-to-end testing seam; authenticate/control exposure |
| NetworkServer | network configuration/control service | Make adapter/route/DNS state observable and transactionally applied |
| DHCPClient | address lease client | Renew/rebind/expiry/link-change state machine |
| LookupServer | hostname/address resolution | Cache TTL/negative answers, restart and malformed-response handling |
| DeviceMapper | device mapping service | Device-node identity and hotplug coordination |
| KeyboardPreferenceLoader | applies keyboard layout/preferences | Per-seat/session settings and device reconnect |
| CrashDaemon | crash collection/coordination | Artifact-bound symbols, privacy, quotas and uploader opt-in |
| ChessEngine | reusable chess computation service | Example of moving a reusable long-running engine out of UI |
| SQLServer | SQL database service | Per-database authority, transaction durability and untrusted-query limits |
| SSHServer | secure remote shell service | **S/B/R?**; security-critical, require protocol conformance and credential hardening |
| TelnetServer | plaintext remote terminal | **S/B/R?**; debug-only by policy, never a secure remote-management default |
| EchoServer | network echo/demo | Probe, not production service |
| WebServer | HTTP file/server utility | Parser/path/TLS boundaries require fuzz and traversal tests |
| SpiceAgent | VM guest integration | Clipboard/display/input policy must remain capability-scoped |

### Native graphical applications

All **51** application directories are explicitly added by
`Userland/Applications/CMakeLists.txt`:

| Application | User-facing feature |
|---|---|
| 3DFileViewer | inspect and render 3D model files |
| About | OS/version/about dialog |
| AnalogClock | desktop analog clock |
| Assistant | global search/action palette across apps, calculator results, terminal commands, URLs and files |
| Browser | native multi-process web browser using LibWeb and broker/content services |
| BrowserSettings | browser preferences UI |
| Calculator | desktop calculator |
| Calendar | calendar/event UI |
| CalendarSettings | calendar preferences |
| CertificateSettings | certificate/trust management UI |
| CharacterMap | Unicode character browsing/copying |
| ClockSettings | time/clock preferences |
| CrashReporter | inspect/report process crashes |
| Debugger | process debugger frontend |
| DisplaySettings | resolution, display and wallpaper settings |
| Escalator | graphical privilege-elevation prompt/client |
| FileManager | filesystem navigation, launch and file operations |
| FontEditor | bitmap font editing |
| GamesSettings | shared game preferences |
| Help | documentation/manpage browser |
| HexEditor | binary/hex viewing and editing |
| ImageViewer | image viewing/navigation |
| KeyboardMapper | interactive keymap construction/editing |
| KeyboardSettings | layout and keyboard preferences |
| Magnifier | screen magnification accessibility tool |
| Mail | email client |
| MailSettings | mail account/preferences UI |
| Maps | map browsing client |
| MapsSettings | map provider/preferences UI |
| MouseSettings | mouse speed/scroll/button preferences |
| NetworkSettings | adapter/address/route configuration UI |
| PDFViewer | PDF document viewer |
| PartitionEditor | disk partition inspection/editing |
| Piano | playable keyboard/synthesis application |
| PixelPaint | layered raster editor with tools and filters |
| Presenter | presentation viewer/editor |
| Run | command/application launch dialog |
| Screenshot | area/window/screen capture |
| Settings | central settings launcher |
| SoundPlayer | audio player |
| SpaceAnalyzer | disk-usage visualization |
| Spreadsheet | spreadsheet editor with readers/writers and focused tests |
| SystemMonitor | process, CPU, memory, network and system monitoring |
| Terminal | terminal emulator |
| TerminalSettings | terminal profile/preferences UI |
| TextEditor | general text editor |
| ThemeEditor | system theme editing and preview |
| UsersSettings | account/user management UI |
| VideoPlayer | video playback |
| Weather | network weather client |
| Welcome | first-run/welcome information |

This is the best application-composition reference: settings are split into
discoverable panels, administration has GUI front ends, hostile formats can be
brokered, and the desktop includes accessibility (`Magnifier`, keyboard/mouse
settings), creation tools (PixelPaint, FontEditor, Spreadsheet, Presenter),
diagnostics and daily-use clients. zlOS should not reproduce each UI literally;
it should reproduce the coverage of user jobs and the common platform services.

### Native games

The **13** build-reachable games are `BrickGame`, `Chess`, `ColorLines`,
`FlappyBug`, `Flood`, `GameOfLife`, `Hearts`, `MasterWord`, `Minesweeper`,
`Snake`, `Solitaire`, `Spider`, and `TwentyFourtyEight` (`2048`). They exercise
animation, timers, input, audio, persistence/preferences, board/AI logic and
desktop packaging. They are first-party application source, not ports.

### Utility surface

`Userland/Utilities/CMakeLists.txt` defines **223** C/C++ utilities (plus an
optional Jakt hello-world). The required/recommended subsets drive base-system
composition; all source entries remain build-reachable components. Functionally
they cover:

- POSIX/core file, text and process tools (`cat`, `cp`, `find`, `grep`, `sed`,
  `tar`, `ps`, `kill`, user/group management, mount and filesystem tools).
- hardware/system diagnostics (`lspci`, `lsusb`, `lsblk`, `lscpu`, `lsirq`,
  `dmesg`, `memstat`, `pmap`, `sysctl`, `strace`, `profile`, `kcov-example`).
- networking (`arp`, `ifconfig`, `route`, `ping`, `traceroute`, `nc`, `host`,
  `netstat`, `ntpquery`, WebSocket/Telnet probes).
- format/media/compiler tools (image/audio conversion, PDF, Matroska/ISOBMFF,
  JavaScript/Wasm, C++ lexer/parser/preprocessor, GLSL, ELF/disassembly).
- desktop control (`chres`, `keymap`, `wallpaper`, `notify`, `open`, `shot`,
  `wsctl`, `network-settings`).
- focused test/probe binaries for unveil, bind-to-device, pthreads, IMAP, JPEG
  round-trip and fuzz plumbing.

The exact list is preserved in the appendix below so small utilities are not
silently collapsed out of the comparison.

### Verification reality

Serenity has the strongest CI in the group: host Lagom tests run under ASan and
UBSan, and x86-64 target tests boot in QEMU and must write `Failed: 0`. However,
the same target runner skips actual tests on AArch64 and RISC-V unless `--force`
is supplied; CI does not supply it and still accepts a generated `Failed: 0`.
That means multiarchitecture build/boot coverage, not multiarchitecture target
test parity. No fresh app-launch or physical-hardware run was performed here.

### Serenity lessons for zlOS

- Define shared driver frameworks first: async request, resource ownership,
  device registry, sysfs-like telemetry, display connector, USB core and network
  adapter contracts.
- Use broker services for filesystem/network/parser access and launch manifests
  with monotonic capabilities.
- Track user jobs, not app count: setup, accessibility, creation, communication,
  media, diagnostics, administration, recovery and onboarding all need owners.
- CI labels must be derived from executed receipts; an architecture job cannot
  claim tests if its runner intentionally skipped them.

## Meulengracht/MollenOS

Mollen has the cleanest kernel/service/driver-policy split but the smallest
native application catalogue in this checkout. Its configured initial GUI app,
`vioarr.run`, and broader desktop are supplied by the separate `vali-userspace`
project described in `BUILDING.md`; they are not source audited here.

### Build-reachable driver/module inventory

| Driver/module | Feature | Evidence and gaps |
|---|---|---|
| PS/2 | x86 controller ports, keyboard, mouse and scan-code set 2 | **S/B/R?** through architecture module graph; add repeated init/teardown and controller-command timeout tests |
| UHCI, OHCI, EHCI | USB 1.x/2.0 host controllers with shared scheduler/transfer layer | **S/B/R?**; EHCI is enabled by default; validate cancellation, bandwidth/schedule ownership, reset and hotplug |
| USB hub | hub class/device discovery | **S/B/R?**; service dependency is commented in manifest; reconnect/peer ordering needs explicit readiness |
| USB HID | report parsing/handling input module | **S/B/R?**; descriptor fuzz and output/report lifecycle are unproven |
| USB mass storage | bulk and UFI protocols over common SCSI/SATA definitions | **S/B/R?**; require command timeout, sense data, reset recovery and unplug tests |
| AHCI | controller/ports/transactions and storage dispatch | **S/B/R?**; good separation, but error recovery and flush/cancel proof absent |
| MFS | native filesystem module with bucket map, record and directory/file operations | **S/B/R?**; needs corrupt-image/crash consistency and disk-full tests |
| ValiFS | read-only VaFS/Zstd-backed application/system image module | **S/B/3P/R?** for VaFS/Zstd components; immutable bytes aid packaging, not process isolation |

Material source that is *not* default reachable:

- xHCI has only `modules/serial/usb/xhci/main.c`; the serial USB CMake graph
  adds common/UHCI/OHCI/EHCI/hub but never xHCI. It is **D**, not a working
  xHCI driver.
- the Intel E1000 family source and match manifest are extensive, but
  `modules/CMakeLists.txt` never adds `eth`; it is **D**.
- `syslogd` has framebuffer/serial/TTY console source, but its service
  subdirectory is commented out; it is **D**.

This distinction corrects the tempting directory-based feature count.

### Device and system services

| Service | Function | Evidence/limits |
|---|---|---|
| `deviced` | PCI enumeration, device records, initrd/module discovery and config parsing | **S/B/R?**; machine-readable driver matching is excellent; add authoritative resource ownership and hot-remove |
| `filed` | VFS, mounts, storage layouts, GPT/MBR, memory/file/device storage, filesystem module interface | **S/B/T/CI/R?** for focused VFS utilities in host unit graph; scope/ownership enforcement remains incomplete |
| `processd` / `phoenix` | process creation, PE loading/imports/relocations/cache, debugger and service discovery | **S/B/R?**; the PE duplicate-module destructor bug is a concrete memory-safety failure; harden all image parsing and lifetime state |
| `served` | application install/load/setup/state using Chef packages and JSON | **S/B/R?**; valuable package lifecycle boundary; add signature, transaction, rollback and per-user authority |
| `sessiond` | session/user lifecycle service | **S/B/R?**; expand into credential-aware per-session namespaces and capability baselines |
| `usbd` | controller/device/hub ownership and port events | **S/B/R?**; good policy split; needs driver/service restart and outstanding-transfer cancellation |
| `netd` | socket domains and adapter/network manager | **S/B/R?**; `internet`, `local`, `Bluetooth` and unspecified domains exist as service code, but project README says Internet is unsupported; do not promote source names to runtime support |

Services and drivers carry YAML manifests with service paths, dependencies or
match rules. That is the strongest driver-registry idea here. The YAML numeric
class/subclass encodings are opaque and some dependencies are commented; zlOS
should use a schema-validated symbolic manifest generated from typed constants.

### Native applications and user-facing features

Only two small app targets are present and built:

- `osstat`: prints OS version, build date/hash/type and architecture injected by
  CMake.
- `systat`: reports live system status through OS APIs.

The default initial application `vioarr.run`, window system and full graphical
userspace are external to this checkout. It is therefore inaccurate to credit
this repository with their implementation. Mollen's user-facing contribution
here is the *packaging/service substrate*: PE applications, read-only VaFS
images, shared libraries, served installation state and separate writable disk
layout.

CI builds i386/amd64 images, CodeQL builds, and host unit tests. The top-level
unit-test configuration comments out services as a group and only explicitly
adds `filed`; CI test strength is therefore narrower than the service catalogue.
The test runner also has a last-command-wins risk documented in the parent
audit. No target boot/app launch was performed here.

### Mollen lessons for zlOS

- Put match metadata, dependencies and service endpoints in one schema-checked
  driver/application manifest.
- Separate immutable application bytes from writable config/data/cache/state,
  but enforce isolation with capabilities and namespaces.
- Make disabled/unreachable sources visible in the ledger; they are backlog
  evidence, never feature evidence.
- Adopt service-owned policy only with peer-death cleanup, restart, handle
  revocation, deadlines and request capacity ownership.

## hhuOS/hhuOS

### Driver and filesystem inventory

hhuOS has a central, static composition graph under `cmake/device` and
`cmake/filesystem`. It targets 32-bit x86 and is optimized for clarity and
teaching rather than modern desktop isolation.

| Family | Built implementations | Evidence and limits |
|---|---|---|
| Bus/CPU/interrupt | ISA, PCI enumeration/devices, I/O ports, MSRs, FPU, SMP, PIC, local APIC/IOAPIC | **S/B/CI-build/R?**; no hotplug/MSI/IOMMU resource broker |
| Graphics | VBE mode discovery, linear framebuffer node, color graphics adapter and terminal node | **S/B/CI-build/R?**; no general window/compositor service; applications draw through Lunar/Pulsar/GL libraries |
| Input | PS/2 controller/device, keyboard and mouse | **S/B/CI-build/R?**; no USB HID path |
| Network | loopback, NE2000 and RTL8139 with network filesystem driver/packet reader | **S/B/CI-build/R?**; ARP/Ethernet/IPv4/ICMP/UDP exist, but no TCP |
| Storage | IDE, AHCI, floppy and virtual disk; CHS conversion and partition handling | **S/B/CI-build/R?**; no NVMe/USB storage; add timeout/reset/flush/removal and corruption gates |
| Filesystems | ACPI, FatFs, ISO9660, memory, process, QEMU, SMBIOS and TAR filesystems | **S/B/3P/CI-build/R?**; FatFs is a submodule; filesystem-as-observability interface is educational but needs authority controls |
| Audio | PC speaker and SoundBlaster PCM | **S/B/CI-build/R?**; `beep` and `play` provide user-facing paths; no centralized multi-client mixer |
| Ports | parallel and 16550-style serial/simple serial | **S/B/CI-build/R?** |
| Platform/time | ACPI, APM, BIOS calls, QEMU fw_cfg, SMBIOS, machine abstraction; ACPI/APIC/HPET/PIT/RTC/CMOS timers | **S/B/CI-build/R?**; readable provider selection; suspend/resume and clock failover unproven |

The entire graph is linked into the image; there is no general runtime module
or userspace driver isolation. More importantly, ring-3 does not create a
security boundary: arbitrary user callers can map physical I/O, mount/unmount,
kill processes and shut down, and syscall handlers directly dereference caller
pointers. zlOS should reuse readability, never the authority model.

### Complete application catalogue

All **45** entries are added by `cmake/application/CMakeLists.txt` and produce
image targets, though several depend on separately licensed submodules/assets.

| Application(s) | Feature and provenance |
|---|---|
| `shell` | native command shell and command-line parser |
| `cat`, `cp`, `date`, `echo`, `head`, `hexdump`, `kill`, `ls`, `mkdir`, `ps`, `pwd`, `rm`, `rmdir`, `touch`, `tree` | native file/text/process core utilities |
| `mount`, `unmount` | native filesystem administration |
| `ip`, `ping`, `uecho`, `nettest` | address/route administration, ICMP, UDP echo and network probing |
| `keyboard` | global keyboard-layout selection |
| `shutdown`, `uptime`, `smbios` | power/reboot, uptime and firmware-table inspection |
| `beep`, `play` | PC-speaker beep-file and sound-card WAV playback |
| `ctest`, `membench` | C-library behavior probe and memcpy/memset benchmark |
| `asciimate` | terminal ASCII-animation player; redistributable build controls bundled media |
| `demo` | native demos for ant simulation, colors, fonts, keyboard, OpenGL, particles, polygons, sprites and widgets |
| `portablegl`, `tinygl` | graphics demonstrations (info/gears/triangle/cubes); based on separate graphics libraries/submodules |
| `view3d` | native 3D model viewer |
| `battlespace` | native 3D space-shooter |
| `bug` | native Bug Defender/Space-Invaders-style game |
| `dino` | native dinosaur platform/action game |
| `rogue3d` | native 3D rogue-like/shooter with enemies, HUD, items, player/projectiles and rooms |
| `classicube` | **3P** ClassiCube/Minecraft Classic client submodule with hhuOS platform integration |
| `clownmdemu` | **3P** Mega Drive/Genesis emulator core with hhuOS frontend |
| `doom` | **3P** DoomGeneric submodule/port; game data licensing is separate |
| `litenes` | **3P** LiteNES emulator integration |
| `peanut-gb` | **3P** Peanut-GB emulator integration with audio/palettes |
| `quake` | **3P** QuakeGeneric integration; data/assets are separate and redistributable-build sensitive |

This application set is an excellent workload ladder: terminal I/O →
filesystem/network tools → audio → 2D widgets → software OpenGL/3D → native
games → emulator/engine ports. It gives driver and library development visible
consumers even without a desktop shell.

Build CI constructs several boot-image variants (`towboot`, Limine, VDD, GRUB)
and packages license material, but does not boot or exercise applications. The
only explicit libc-oriented app is `ctest`; it is not invoked by the workflow.
Submodules for FatFs, TinyGL, PortableGL, Peanut-GB, QuakeGeneric, DoomGeneric,
ClownMDEmu, ClassiCube and LiteNES retain separate provenance.

### hhuOS lessons for zlOS

- Generate a readable system composition view even if device registration is
  dynamic; clarity is a feature.
- Keep a deliberate workload ladder and a tiny native diagnostic for every
  driver class.
- Never infer first-party ownership from an in-tree submodule, and never ship
  games/emulators without separate code/data/firmware license records.
- Education-friendly direct access should be an opt-in privileged lab profile,
  not the normal authority model.

## byteduck/duckOS

duckOS is the smallest repository here that still provides a coherent native
desktop, sound server, typed IPC library, DHCP client, app bundles, shell and
daily-use graphical programs.

### Driver and kernel device inventory

| Family | Built implementation | Evidence and limits |
|---|---|---|
| x86 platform | i8042 PS/2 controller, keyboard/mouse, PATA, Bochs VGA, AC97, CMOS/PIT/RTC, PCI | **S/B/CI-build/R?** for i686; i8042 self-test is disabled/problematic on real hardware; no USB/NVMe/AHCI |
| AArch64/RPi | mini-UART, GPIO, mailbox, framebuffer, device info, ARM timer/MMU | **S/B/R?**; CI only builds i686; several i8042 paths are marked TODO for AArch64 |
| Common display | multiboot VGA/framebuffer and generic VGA device | **S/B/R?**; Pond consumes framebuffer and software-composes windows |
| Network | E1000 adapter, loopback/manager/router, ARP/IP, UDP and partial TCP | **S/B/R?**; retransmission, receive-window scaling, ARP timeout, connection timeout and locking TODOs are explicit |
| Storage/filesystems | common disk/partition cache, x86 PATA; ext2, procfs, PTYFS and SocketFS | **S/B/T/R?**; kernel tests cover memory/containers, not storage recovery; add forced error, corrupt ext2, flush and cold-boot tests |
| Audio | x86 AC97 `/dev/snd0` consumed by Quack | **S/B/R?**; no hardware reconnect or negotiated device formats |
| TTY/pseudo devices | virtual TTY, PTY controller/mux/slave, kernel log, null/zero/random | **S/B/R?**; enables GUI Terminal and dsh |

Kernel tests are compiled into the image and can be selected by the custom
`tests` QEMU target; the inspected GitHub workflow only performs `make install`
and image creation, not `make tests`. Thus tests are **T-configured**, not CI
runtime evidence.

### Services and protocols

| Service | Feature | Evidence/limits |
|---|---|---|
| `init` | reads service configuration, starts boot services, waits for children | **S/B/CI-build/R?**; it exits when all children die and has no demonstrated restart policy/readiness graph |
| `pond` | compositor/window server, display/input, fonts and client/window lifecycle | **S/B/CI-build/R?**; typed River functions/messages cover open/destroy/move/resize/invalidate/focus/title/reparent/hints/display info and input events; client-supplied size validation has a TODO |
| `quack` | audio server/mixer with per-client shared sample buffers | **S/B/CI-build/R?**; client disconnect cleanup exists; missing-card behavior, write errors, bounded backpressure and device reconnect need work |
| `dhcpclient` | discovers interfaces and performs DHCP | **S/B/CI-build/R?**; accepts ACK then exits, cannot handle offers in its receive path and has no renew/rebind/expiry lifecycle |

River provides named buses/endpoints, typed function/message binding and client
connect/disconnect callbacks. It is a compact zlIDL reference, but not an
authority model: pair it with capability-bearing handles, message-size bounds,
deadlines and peer identity checks.

### Graphical applications

The application graph builds **12** entries:

| Application | User-facing feature |
|---|---|
| `about` | OS/about information |
| `calculator` | desktop calculator |
| `terminal` | Pond GUI terminal over PTY |
| `4inarow` | native four-in-a-row game |
| `ducksweeper` | native minesweeper game with elapsed-time UI |
| `monitor` | process manager, memory usage, process inspector and virtual-memory layout views |
| `sandbar` | desktop/task bar and application menu with modules |
| `files` | icon/grid file browser with navigation bar and file launching |
| `editor` | text editor with text-file association resources |
| `viewer` | image pan/zoom viewer plus audio-viewer/player widget and file associations |
| `3demo` | software 3D rendering demonstration |
| `uxn` | Uxn virtual machine/runtime; installed as a program rather than an app bundle |

The `MAKE_APP` rule installs each executable and its resources into
`apps/<name>.app`, so code, icons and file-type metadata travel together. This is
a good application-bundle shape. Add a signed manifest rather than deriving
meaning from directory layout alone.

### Native command-line programs

`dsh` is the native shell. The **22** core utilities are `open`, `cat`, `chmod`,
`chown`, `cp`, `echo`, `free`, `ln`, `ls`, `mkdir`, `mv`, `ps`, `pwd`, `rm`,
`rmdir`, `touch`, `truncate`, `play`, `date`, `uname`, `kill`, and `profile`.
Together they cover app/MIME launching, filesystem administration, identity
metadata, process/memory inspection, sound playback and profiling. All are
**S/B/CI-build/R?**.

Security qualification matters: shared-memory IDs are guessable/sequential and
the grant syscall lacks ownership verification; signal 0 and a negative process
group branch have authorization defects. Pond/Quack shared buffers therefore
must not be treated as safe capability transport.

### duckOS lessons for zlOS

- Use one bundle for binary, icon, translations, MIME handlers and resources,
  augmented with signed capability/dependency/smoke-test metadata.
- River's concise typed endpoint shape is worth reproducing; add versioning,
  bounds, capability transfer, cancellation and restart semantics at generation
  time.
- A useful first-party desktop can be compact if shared libraries supply file
  grids, navigation, terminal, layout, theme, application launch and audio.
- Make process memory, driver state, queues, resets and known failure visible in
  Monitor rather than only in serial logs.

## Driver lifecycle and recovery gaps by phase

The same omissions recur across otherwise unrelated implementations:

| Phase | What exists often | What is usually missing | zlOS acceptance gate |
|---|---|---|---|
| Match/discovery | PCI/USB class or ID match, ACPI/DT enumeration | match priority/conflict, signed provenance, unsupported-reason reporting | deterministic match test, exactly one owner, inspectable rejection reason |
| Resource claim | BAR mapping, IRQ registration, DMA allocation | IOMMU domain, size/overflow validation, partial-init rollback | fuzzed descriptors/BARs, forced allocation failure at every step, zero leaked claims |
| Initialization | queue/ring setup and enable bit | bounded waits, controller reset, firmware/quirk versioning | timeout injection, reset retry policy, artifact/device receipt |
| Normal I/O | single queue/read/write/interrupt path | cancellation, backpressure, ordering, concurrency, malformed completion | model test under reorder/dup/drop, sanitizer/fuzz, queue invariant receipt |
| Error | log/return/panic | per-request error, device health state, retry budget, failover | forced media/DMA/protocol/interrupt faults with exact expected result |
| Quiesce | often absent | stop new I/O, drain/cancel outstanding requests, mask IRQ, flush | bounded quiesce and idempotent repeated call |
| Detach/unload | source may have an exit hook | IRQ/DMA/handle revocation, stale callbacks, user notification | repeated attach/detach/unload under traffic, generation-tagged handles |
| Suspend/resume | platform poweroff exists | device save/restore ordering and failed resume | system DAG suspend/resume with rollback/degraded mode |
| Service restart | client disconnect may delete state | rebind device, replay safe state, reject stale handles, preserve user work | kill service mid-operation, supervised restart and convergence receipt |

## Application feature architecture for zlOS

The aggregate app catalogue should become a set of platform contracts, not a
request to clone hundreds of UIs.

### Required shared platform services

1. **App/Package Service:** signed manifests, content-addressed immutable app
   image, dependency resolution, atomic install/update/rollback, per-user data
   mounts, launch/smoke receipts and clean uninstall.
2. **Launch Service:** MIME/URL/file handlers, singleton/multi-instance policy,
   capability baseline, dependency readiness, recent documents and safe-open
   portals.
3. **Session Service:** authenticated user/session identity, environment,
   capability namespace, logout, lock, accessibility profile and crash-safe
   restoration.
4. **Window/Input Services:** server-owned window state, shared buffers with
   validated stride/format/damage, clipboard, drag/drop, per-seat input,
   capability-gated grabs and compositor restart.
5. **Audio Service:** bounded client rings, mixer, per-stream/app controls,
   format conversion, device selection/reconnect and latency/underrun telemetry.
6. **File Operation/Portal Services:** copy/move/delete transactions, progress,
   conflict policy, cancellation, trash/undo and capability-scoped file access.
7. **Decoder workers:** disposable image/font/archive/PDF/audio/video workers with
   input/output/time/memory bounds and no ambient authority.
8. **Settings/Config Service:** typed versioned schemas, watch/transaction,
   migrations, system-versus-user scope and export/reset.
9. **Notification/Crash Services:** actionable replaceable notifications,
   rate/quiet policy, artifact-bound crash symbols, privacy and quotas.
10. **Observability Service:** driver lifecycle, queues, storage health, network,
    battery, audio, logs, processes and services exposed to Device Manager and
    System Monitor through typed read-only views.

### User-job coverage map

| User job | Best observed apps | zlOS-native target |
|---|---|---|
| Onboarding/setup | Serenity/Lemon Welcome; settings suites | Welcome that configures display/input/network/account/accessibility and verifies each outcome |
| Find/launch/switch | Serenity LaunchServer/Taskbar; banan launcher/taskbar; duck Sandbar | searchable launcher, task switcher, app metadata and failure explanation |
| Files | Serenity/Lemon/banan/duck file managers | Files with breadcrumbs/grid/list/search, previews, mounts, transactional operations, trash/undo and permission portals |
| Terminal/development | all but Mollen checkout | terminal, shell, editor, debugger, profiler, hex viewer, package tools and self-hosted zl tools |
| Create | Serenity PixelPaint, Spreadsheet, Presenter, Font/Theme editors | staged native creation suite sharing document, undo, autosave, print/export and accessibility infrastructure |
| Media | Serenity/Lemon players, Brook player, duck viewer, hhu play | isolated codecs, library/playlist, device selection, resume position and robust malformed-file handling |
| Communication/network | Serenity Browser/Mail/Maps/Weather; Astral/banan ports | browser and mail only after brokered networking/parser isolation; network settings and diagnostics first |
| Administration | Serenity settings/device/partition/users; Lemon DeviceManager; monitors everywhere | Device Manager, Disk/Partition tool, Users, Updates, logs and recovery with capability prompts and rollback |
| Accessibility | Serenity Magnifier and input settings | magnifier, keyboard navigation, screen-reader semantics, high contrast, scaling, reduced motion and per-user persistence |
| Games/graphics | Serenity games, hhu native/ported workload ladder, duck games | small native games as input/audio/graphics/save-state conformance workloads before large ports |

## Clean-room implementation order

This is a complete target staged by dependency, not a reduced final scope:

1. Driver resource broker, lifecycle state machine and evidence receipts.
2. Harden existing zlOS PCI/display/xHCI/NVMe paths through reset, timeout,
   teardown, injected error and hardware receipts.
3. Common block/network/input/audio/display contracts plus deterministic host
   simulators.
4. zlIDL/River-like typed IPC with handles, bounds, deadlines, cancellation,
   peer identity, versioning and generated conformance tests.
5. Supervised device, filesystem, network, session, config, launch and package
   services.
6. App bundle/manifest, immutable package image, explicit writable mounts,
   signed provenance and atomic update/rollback.
7. Window/input/audio/clipboard/notification/file-operation/parser services.
8. Coherent base apps: Welcome, Settings, Device Manager, System Monitor, Files,
   Terminal, Shell, Text Editor, Run, Taskbar and recovery tools.
9. Creation/media/accessibility/communication suite on those shared contracts.
10. Port ecosystem with per-port build/install/launch/failure/hardware receipts;
    no recipe-only “available” state.

## Weakest evidence and non-conclusions

- No build, boot, app launch, QEMU test or physical-device run was performed in
  this expansion.
- Serenity is the only repository here with broad inspected runtime CI, and its
  secondary-architecture target tests are skipped by default.
- Brook's local e2e scripts, banan/Lemon/duck test apps, and hhu `ctest` are
  useful source/configuration evidence but not inspected CI results.
- Driver source breadth says nothing by itself about supported PCI IDs, quirks,
  real-hardware coverage, reset, suspend/resume or detach.
- Port recipes, submodules, vendored libraries, screenshots and external
  userspace repositories remain third-party or claim evidence.
- “First-party app exists” does not prove accessibility, persistence, data-loss
  safety, hostile-file safety, localization or multi-user behavior.

## Appendix — exact Serenity utility target list

This is the build graph's complete 223-entry C/C++ utility source list at the audited
commit; the optional Jakt `hello-world` is separate:

`abench`, `aconv`, `adjtime`, `allocate`, `animation`, `aplay`, `arp`, `asctl`, `base64`, `basename`, `beep`, `blockdev`,
`bt`, `cal`, `cat`, `checksum`, `chgrp`, `chmod`, `chown`, `chres`, `cksum`, `clear`, `cmp`, `comm`,
`config`, `copy`, `copy_mount`, `cp`, `cpp-lexer`, `cpp-parser`, `cpp-preprocessor`, `crypto-bench`, `cut`, `date`, `dd`, `df`,
`diff`, `dirname`, `disasm`, `disk_benchmark`, `dmesg`, `drain`, `du`, `echo`, `elfdeps`, `env`, `errno`, `expr`,
`false`, `fdtdump`, `file`, `find`, `flock`, `fortune`, `functrace`, `fusermount`, `glsl-compiler`, `gml-format`, `grep`, `gron`,
`groupadd`, `groupdel`, `groups`, `gzip`, `head`, `headless-browser`, `hexdump`, `hiddump`, `host`, `hostname`, `icc`, `iconv`,
`id`, `ifconfig`, `image`, `image2bin`, `imgcmp`, `ini`, `init`, `inputdbg`, `install`, `isobmff`, `jbig2-from-json`, `js`,
`json`, `kcov-example`, `keymap`, `kill`, `killall`, `less`, `listdir`, `ln`, `logout`, `ls`, `lsblk`, `lscpu`,
`lsdev`, `lsirq`, `lsof`, `lspci`, `lsusb`, `lzcat`, `man`, `markdown-check`, `matroska`, `md`, `memstat`, `mkdir`,
`mkfifo`, `mkfs.fat`, `mknod`, `mktemp`, `mount`, `mv`, `nc`, `netstat`, `network-settings`, `nl`, `nohup`, `nologin`,
`notify`, `nproc`, `ntpquery`, `open`, `passwd`, `paste`, `patch`, `pathchk`, `pdf`, `pgrep`, `pidof`, `ping`,
`pixelflut`, `pkill`, `pledge`, `pls`, `pmap`, `pmemdump`, `printf`, `pro`, `profile`, `ps`, `purge`, `pwd`,
`readelf`, `readlink`, `realpath`, `reboot`, `rev`, `rm`, `rmdir`, `route`, `run-tests`, `sed`, `seq`, `shot`,
`shred`, `shuf`, `shutdown`, `sizefmt`, `sleep`, `slugify`, `sort`, `sql`, `stat`, `strace`, `strings`, `stty`,
`su`, `sync`, `syscall`, `sysctl`, `tac`, `tail`, `tar`, `tee`, `telws`, `test-bindtodevice`, `test-fuzz`, `test-imap`,
`test-jpeg-roundtrip`, `test-pthread`, `test-unveil`, `test`, `test_env`, `timezone`, `top`, `touch`, `tr`, `traceroute`, `tree`, `true`,
`truncate`, `tsort`, `tt`, `ttfdisasm`, `tty`, `umount`, `uname`, `uniq`, `unveil`, `unzip`, `update-cpp-test-results`, `uptime`,
`useradd`, `userdel`, `usermod`, `utmpupdate`, `w`, `wallpaper`, `wasm`, `watch`, `watchfs`, `wc`, `which`, `whoami`,
`wsctl`, `xargs`, `xml`, `xxd`, `xzcat`, `yes`, `zip`.
