#ifndef ZLOS_VERSION_H
#define ZLOS_VERSION_H
/* THE VERSION AND THE ARCHITECTURE, IN ONE PLACE.
 *
 * The string "0.3" was written out in six places in kernel.zl and a SEVENTH in
 * settings.c said "zl 0.1" - so the About pane told you the machine was two
 * releases behind the rail immediately to its left, which reads as 0.1 being
 * the truth and everything else being decoration.
 *
 * The architecture was worse than stale, it was BUILD-DEPENDENT and written as
 * a constant: "i386 - ring 0" in a file compiled into both the 32-bit and the
 * 64-bit kernel. The 64-bit build is the one that boots on the ThinkPad and
 * the one probe-shot.py renders, and it was reporting i386 while the same
 * screen's rail read "0.3 x86_64" and the shell offered to run a 64-bit ring 3
 * binary.
 *
 * ZL_64 is the flag build64.sh and buildefi.sh set; the 32-bit build does not.
 * See kernel/tests/host/build.sh, which passes -DZL_64 for the same reason. */

#define ZLOS_VERSION_STR "0.3"

#if defined(ZL_64)
#define ZLOS_ARCH_STR      "x86_64"
#define ZLOS_KERNEL_STR    "x86_64 - ring 0"
#else
#define ZLOS_ARCH_STR      "i386"
#define ZLOS_KERNEL_STR    "i386 - ring 0"
#endif

#endif /* ZLOS_VERSION_H */
