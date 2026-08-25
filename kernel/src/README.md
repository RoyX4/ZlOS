# zlOS Source

The kernel implementation is grouped by runtime ownership:

- `arch/x86/`: CPU, interrupts, paging, PCI, SMP, and machine primitives
- `core/`: scheduling, execution, memory services, console, and logging
- `drivers/`: display, input, storage, and network devices
- `fs/`: filesystems and block-facing storage logic
- `graphics/`: framebuffer, fonts, icons, UI, and windowing
- `net/`: protocols, cryptography, certificates, and transport
- `runtime/`: kernel-side zl interpreter support
- `web/`: HTML, CSS, layout, JavaScript, and browser integration

`kernel.zl` is the operating system program. Shared C sources compiled into all
boot targets are listed in `../SOURCES`.
