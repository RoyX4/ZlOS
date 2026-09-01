# Security policy

## Supported state

zlOS is active experimental systems software. There is no production-supported
release channel and no claim of complete isolation, hardening, secure boot,
update security, or physical-device safety.

Security reports are still valuable, especially for:

- memory corruption or privilege-boundary failures;
- malformed input that escapes a documented bound;
- cryptographic verification mistakes;
- unsafe device programming or panel-power sequences;
- build, evidence, or release gates that report a false green;
- accidental credential, identity, or private-data exposure.

## Reporting a vulnerability

Use the repository's **Security → Report a vulnerability** flow. Do not place
exploit details, private data, or hardware-damage instructions in a public
issue.

Include the affected commit, boot/build route, hardware or QEMU configuration,
the smallest reproduction you can safely provide, and whether the result was
source-only, host-tested, booted, or physically observed.

## Disclosure and evidence

Reports will be assessed against the exact affected artifact and route. A fix
will not be called complete until the relevant reproduction fails safely and
the smallest regression gate can observe the original defect.
