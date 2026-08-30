# zlOS Tools

- `checks/`: static checks and boot verification gates
- `generators/`: committed font, icon, cursor, and GPU asset generators
- `images/`: ISO, raw disk, and ESP image builders
- `probes/`: QEMU and runtime observation scripts
- `run/`: convenience VM and trace launchers

Checks, image builders, probes, runners, and direct-write generators resolve the
kernel root from their own location. The primary build entry points remain at
the kernel root.

Generators that write files directly resolve their destination under `src/`.
`mkfont.py` writes in the current directory, and the three GPU extractors emit
to stdout. Run those four from `kernel/` or give an explicit redirection path;
their headers show the destination under `src/`.
