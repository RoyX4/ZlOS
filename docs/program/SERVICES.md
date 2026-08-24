# Complete system-service registry

Services own policy, parsing, shared state and cross-application facilities that
do not belong in the mechanism kernel. `Current kernel` means migration starts
from substantive existing kernel behavior; it does not mean the final service
boundary exists.

Every service inherits MP-04 and DA-19: versioned descriptor and protocol,
explicit role/authority/exposure, dependencies/readiness, authenticated
endpoints, message/byte/connection/resource budgets, health and redacted audit,
crash budget/backoff/quarantine, dependency-loss/reconnect, reverse shutdown,
peer-death cleanup and authenticated management.

## Core process, authority and lifecycle services

| ID | Service | Primary phase | Starting asset / destination responsibility |
|---|---|---|---|
| SVC-001 | Init and Service Supervisor | MP-04 | replace ad-hoc init with DAG, readiness, rollback and reaper |
| SVC-002 | Process Manager | MP-04 | create/exec/wait/signal/control through capabilities |
| SVC-003 | Capability/Handle Broker | MP-04 | derivation, transfer, revoke, delegation receipts |
| SVC-004 | zlIDL Schema Registry | MP-04 | generated protocols, versions and conformance |
| SVC-005 | Session Manager | MP-04 | login/session lifecycle, seats and resource roots |
| SVC-006 | User/Credential Service | MP-04 | users/groups/credential transitions/password policy |
| SVC-007 | Policy/Authorization Service | MP-04 | deny-by-default operation and object policy |
| SVC-008 | Secrets/Keyring Service | MP-04 | encrypted scoped secrets, consent, revoke and zeroing |
| SVC-009 | Audit/Event Service | MP-00 | typed effects, identity, redaction, anchoring and drops |
| SVC-010 | Clock/Timezone Service | MP-04 | wall clock/timezone separate from kernel deadlines |
| SVC-011 | Resource/Quota Manager | MP-04 | process/service memory, CPU, handles, I/O and connection budgets |
| SVC-012 | Crash/Recovery Manager | MP-18 | crash collection, symbols, quarantine and recovery action |

## Driver and device management services

| ID | Service | Primary phase | Starting asset / destination responsibility |
|---|---|---|---|
| SVC-020 | Device Manager | MP-05 | immutable inventory, match/bind/rebind/remove and generations |
| SVC-021 | Driver Supervisor | MP-05 | provider process lifecycle, isolation and recovery |
| SVC-022 | Firmware Inventory | MP-02 | checked ACPI/SMBIOS/FDT/UEFI facts and provenance |
| SVC-023 | DMA/IOMMU Manager | MP-05 | domains, buffers, revoke-before-reuse and fault telemetry |
| SVC-024 | Power Manager | MP-19 | shutdown/reboot/suspend, policy and ordered quiesce |
| SVC-025 | Battery/Thermal Manager | MP-19 | status, thresholds, fan policy and failsafe |
| SVC-026 | Hotplug/Event Broker | MP-05 | device add/remove/change with bounded subscriber queues |
| SVC-027 | Hardware Qualification Service | MP-19 | topology-specific tests and physical receipts |
| SVC-028 | Firmware/Microcode Update Service | MP-19 | signed staged updates, rollback and compatibility |

## Storage, files, search and package services

| ID | Service | Primary phase | Starting asset / destination responsibility |
|---|---|---|---|
| SVC-030 | Block Broker | MP-06 | current block cache -> asynchronous provider arbitration |
| SVC-031 | VFS/File Service | MP-06 | handles, namespaces, mounts, permissions and file operations |
| SVC-032 | zlfs Provider | MP-06 | current zlfs v2 -> full provider and recovery contract |
| SVC-033 | Removable Media Manager | MP-06 | consent, mount/eject, dirty state and device loss |
| SVC-034 | Volume/Partition Manager | MP-06 | GPT/MBR, formats, encryption and ownership |
| SVC-035 | File Cache/Writeback Service | MP-06 | bounded cache, pressure, flush and durability receipts |
| SVC-036 | Backup/Snapshot Service | MP-06 | scheduled/manual backup, restore, retention and verification |
| SVC-037 | Filesystem Check/Repair Service | MP-06 | offline/online validation, logged repair and rollback |
| SVC-038 | MIME/Type Registry | MP-11 | sniffing bounds, extension policy and handlers |
| SVC-039 | Thumbnail Service | MP-11 | isolated decoders, cache, quotas and privacy |
| SVC-040 | Search/Index Service | MP-11 | content/metadata index, permissions and cancellation |
| SVC-041 | Package Manager | MP-06 | signed transactional install/update/remove and live revoke |
| SVC-042 | Repository/Update Service | MP-06 | metadata trust, channels, download, staging and rollback |
| SVC-043 | Provenance/License Service | MP-00 | artifact origin, SBOM, licenses, signatures and health |
| SVC-044 | File Portal | MP-11 | user-mediated open/save/directory grants with revocation |
| SVC-045 | Archive Worker | MP-11 | length-first unpack/list/create in restricted process |

## Network, internet and remote services

| ID | Service | Primary phase | Starting asset / destination responsibility |
|---|---|---|---|
| SVC-050 | Network Device Broker | MP-08 | current netdev -> multiple providers/hotplug/failover |
| SVC-051 | Network Stack Service | MP-08 | Ethernet/ARP/NDP/IP/ICMP/UDP/TCP/local sockets |
| SVC-052 | Address Configuration | MP-08 | DHCPv4/v6, SLAAC, static and lease persistence |
| SVC-053 | Resolver Service | MP-08 | DNS cache/search/DNSSEC policy/offline errors |
| SVC-054 | TLS/Certificate Service | MP-08 | roots, validation, clocks, key isolation and revocation |
| SVC-055 | HTTP/Transfer Service | MP-08 | bounded requests, redirects, caching, resume and download |
| SVC-056 | Routing/Firewall Service | MP-08 | routes, interfaces, zones, filtering and audit |
| SVC-057 | Wi-Fi Service | MP-19 | scan/associate/WPA/roam/regulatory/credentials |
| SVC-058 | Bluetooth Service | MP-19 | HCI, pairing, keys, discovery and profile brokers |
| SVC-059 | Connectivity/Captive Portal Service | MP-08 | online state, portal detection and user flow |
| SVC-060 | SSH/Remote Shell Service | MP-15 | authenticated PTY, keys, limits and audit |
| SVC-061 | Remote Desktop Service | MP-18 | authenticated display/input/clipboard and sessions |
| SVC-062 | Time Synchronization Service | MP-08 | NTP/SNTP validation, monotonic-safe clock adjustment |
| SVC-063 | Network Diagnostics Service | MP-08 | ping/trace/DNS/socket/capture with scoped authority |
| SVC-064 | VPN/Tunnel Service | MP-19 | keys, routes, kill switch, reconnect and policy |

## Display, input, shell and accessibility services

| ID | Service | Primary phase | Starting asset / destination responsibility |
|---|---|---|---|
| SVC-070 | Display Server/Compositor | MP-07 | current WM/FB -> isolated surfaces, scanout and atomic commit |
| SVC-071 | Window Manager Policy | MP-07 | placement, focus, stacking, workspaces and recovery |
| SVC-072 | Input Service | MP-07 | devices/events/remap/repeat/acceleration and routing |
| SVC-073 | Seat/Focus/Grab Service | MP-07 | secure ownership of keyboard/pointer/touch and grabs |
| SVC-074 | Login/Lock/Greeter Service | MP-07 | authenticated accessible session entry and secure attention |
| SVC-075 | Shell/Desktop Service | MP-07 | desktop, dock/panel, menu, notifications and workspaces |
| SVC-076 | Application Registry/Launcher | MP-11 | admitted manifests, exact IDs/routes and launch receipts |
| SVC-077 | Appearance/Theme Service | MP-10 | semantic tokens, preview, atomic apply and rollback |
| SVC-078 | Font/Shaping Service | MP-10 | font discovery, shaping, fallback, caches and licensing |
| SVC-079 | Icon/Asset Service | MP-10 | semantic/versioned assets, scale/theme variants |
| SVC-080 | Localization Service | MP-10 | catalogues, locale formats, fallback and live updates |
| SVC-081 | IME/Text Input Service | MP-10 | composition, candidates, secure fields and locale |
| SVC-082 | Accessibility Tree Broker | MP-10 | semantic snapshots/events/actions and privacy |
| SVC-083 | Screen Reader/Speech Service | MP-10 | focus/navigation/speech/braille providers |
| SVC-084 | Magnifier/Visual Assistance | MP-10 | zoom, cursor, filters, large text and multi-display |
| SVC-085 | Captions/Assistive Input Service | MP-10 | captions, switch/dwell, sticky/filter/slow keys |
| SVC-086 | Color/Profile Service | MP-10 | display profiles, contrast, filters and calibration |

## Audio, media and communication services

| ID | Service | Primary phase | Starting asset / destination responsibility |
|---|---|---|---|
| SVC-090 | Audio Server | MP-09 | device ownership, mixing, shared rings, routing and recovery |
| SVC-091 | Media Clock Service | MP-09 | stream timestamps, sync, drift and pause/seek |
| SVC-092 | Codec/Decoder Worker Pool | MP-09 | isolated image/audio/video/font/document parsers |
| SVC-093 | Camera/Capture Service | MP-09 | frames, formats, permission, indicator and revoke |
| SVC-094 | Media Library/Metadata Service | MP-14 | index, tags, playlists, thumbnails and privacy |
| SVC-095 | Speech Recognition Service | MP-17 | opt-in audio, model identity, privacy and cancellation |
| SVC-096 | Text-to-Speech Service | MP-10 | voices, queue, interruption and accessibility binding |
| SVC-097 | Communication Notification Broker | MP-14 | message/call events, privacy and quiet modes |
| SVC-098 | Realtime Call/Conference Service | MP-14 | media negotiation, permissions, network changes and teardown |

## Common application and portal services

| ID | Service | Primary phase | Starting asset / destination responsibility |
|---|---|---|---|
| SVC-100 | Settings Service | MP-11 | schemas, per-user/system scopes, preview and rollback |
| SVC-101 | Clipboard Broker | MP-11 | MIME offers, history policy, consent and owner death |
| SVC-102 | Notification Service | MP-11 | actions, grouping, quiet modes, persistence and a11y |
| SVC-103 | Open/Save/Share Portal | MP-11 | mediated target selection and capability grants |
| SVC-104 | Print Spooler | MP-14 | queue, render, status, cancel, privacy and retry |
| SVC-105 | Scan Service | MP-14 | provider selection, preview, capture and privacy |
| SVC-106 | Document/Autosave Service | MP-11 | revisions, recovery, conflicts and export |
| SVC-107 | Undo/History Service Library | MP-11 | bounded commands, merge, persistent recovery |
| SVC-108 | Spellcheck/Dictionary Service | MP-14 | language packs, privacy and custom dictionaries |
| SVC-109 | Calendar/Alarm Service | MP-14 | wall/deadline separation, recurrence and wake policy |
| SVC-110 | Contacts Service | MP-14 | per-user data, grants, import/export and sync conflicts |
| SVC-111 | Mail Service | MP-14 | accounts, transport, local store, search and security |
| SVC-112 | Messaging/Presence Service | MP-14 | accounts, delivery state, encryption policy and offline queues |
| SVC-113 | Location Portal | MP-19 | consent, precision, lifetime and audit |
| SVC-114 | Background Task Scheduler | MP-11 | quotas, deadlines, power/network policy and cancellation |
| SVC-115 | Application Restore Service | MP-11 | windows/documents, crash/reboot semantics and consent |

## Developer, compatibility, agent and operations services

| ID | Service | Primary phase | Starting asset / destination responsibility |
|---|---|---|---|
| SVC-120 | PTY/Terminal Service | MP-15 | sessions, job control, resize, signals and peer cleanup |
| SVC-121 | Shell/Command Service | MP-15 | parsing, pipelines, redirection, jobs and scripts |
| SVC-122 | Debug Service | MP-15 | process-scoped attach, symbols, breakpoints and audit |
| SVC-123 | Profiler/Trace Service | MP-15 | bounded sampling/events, symbols and export |
| SVC-124 | Build Service | MP-16 | hermetic graphs, cache, sandbox and receipts |
| SVC-125 | Package Build/Port Service | MP-15 | locked recipes, patches, staging and runtime gates |
| SVC-126 | POSIX/Linux Compatibility Service | MP-15 | program-driven ABI and isolation |
| SVC-127 | Dynamic Linker/Loader Service | MP-15 | checked ELF/shared objects/relocations and cache ownership |
| SVC-128 | Virtual Machine Manager | MP-15 | VM lifecycle, devices, images, snapshots and containment |
| SVC-129 | Container/Sandbox Manager | MP-15 | namespaces, capabilities, resources and teardown |
| SVC-130 | Agent Orchestrator | MP-17 | typed plans outside ring 0, lifecycle and budgets |
| SVC-131 | Tool Broker | MP-17 | deterministic authorized operations and receipts |
| SVC-132 | Skill/Plugin Manager | MP-17 | signed transactional install/revoke/uninstall |
| SVC-133 | Automation/Scheduler Service | MP-17 | user-owned workflows, triggers, cancellation and history |
| SVC-134 | Independent Review Service | MP-17 | locked criteria and reject-capable evidence review |
| SVC-135 | Release/Update Orchestrator | MP-18 | build/sign/promote/rollback/revoke generations |
| SVC-136 | Public Demo Lease Manager | MP-18 | atomic allocation, quotas, launch and reclaim |
| SVC-137 | Authenticated VNC/WebSocket Proxy | MP-18 | per-lease data-plane identity and expiration |
| SVC-138 | Telemetry/Health Service | MP-18 | metrics, budgets, retention, privacy and alerts |
| SVC-139 | Support Bundle Service | MP-18 | consent, redaction, reproduction and export |
| SVC-140 | Rescue/Repair Orchestrator | MP-18 | read-only default, target confirmation and repair log |
| SVC-141 | Feature/Provenance Viewer Provider | MP-18 | live registry, receipts, defects and evidence ceiling |

## Service migration rule

Service extraction is not a rewrite switch. For each existing in-kernel path:

1. freeze a deterministic behavior and failure oracle;
2. define the typed service/provider protocol and authority matrix;
3. run old and new paths against the same corpus where possible;
4. ship the new provider behind explicit selection and crash fallback;
5. prove restart, peer death, quota, malformed input and update rollback;
6. remove the privileged policy/parser only after target evidence is equivalent;
7. retain a minimal console/software/recovery path where it is the safety oracle.
