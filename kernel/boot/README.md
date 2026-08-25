# zlOS Boot

Architecture entry code, EFI handoff code, AP trampolines, and linker layouts
live here. The root build scripts choose the target-specific files from this
directory; shared shipped C sources remain listed in `../SOURCES`.
