# Freestanding Runtime

This is the no-libc proof lane used by zlOS. It combines C emitted by the boxed
backend with a small startup and kernel runtime.

It is not the normal host runtime and does not claim full language parity. Use
`build.sh` from this directory's documented interface; generated `_gen.c` and
binaries remain ignored.
