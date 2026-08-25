# `build_request` overflows `req[]` by 9 bytes — the one memory-safety hit in an otherwise clean sweep

**2026-08-19 · worktree `zl-linux-fleet` · tree `3f00366` · arithmetic re-derived by hand**

From the browser wave's `memory-safety` lens. **Confirmed.**

The context matters as much as the bug. That lens enumerated **all 33 fixed buffers**
across `html.c`, `layout.c`, `browser.c`, `http.c`, `tcp.c`, `net.c` and `dns.c` and
traced every write whose length derives from network bytes, reporting:

> **THE HEADLINE IS NEGATIVE AND IT IS THE REAL RESULT:** there is no unclamped
> network-derived write anywhere […] Every one is clamped, and I verified each clamp
> arithmetically rather than by reading its comment.

So this is not one of many. It is the exception, and it is an *outbound* buffer — which
is likely why it escaped a review focused on hostile input.

---

## The arithmetic

```c
/* kernel/src/net/http.c:40,43 */
#define REQ_MAX  512
static u8   req[REQ_MAX];

/* kernel/src/net/http.c:89-104 */
static int build_request(void)
{
    int n = 0;
    const char *g = "GET ";
    while (*g) req[n++] = (u8)*g++;                                        /* n ≤ 4   */
    for (int i = 0; path[i] && n < REQ_MAX - 64; i++) req[n++] = (u8)path[i];   /* n ≤ 448 */
    const char *v = " HTTP/1.0\r\nHost: ";
    while (*v) req[n++] = (u8)*v++;                                        /* +17     */
    for (int i = 0; host[i] && n < REQ_MAX - 32; i++) req[n++] = (u8)host[i];   /* n ≤ 480 */
    const char *t = "\r\nUser-Agent: zlOS\r\nConnection: close\r\n\r\n";
    while (*t) req[n++] = (u8)*t++;                                        /* UNGUARDED */
    return n;
}
```

The tail string, counted byte by byte:

| piece | bytes |
|---|---|
| `\r\n` | 2 |
| `User-Agent: zlOS` | 16 |
| `\r\n` | 2 |
| `Connection: close` | 17 |
| `\r\n` | 2 |
| `\r\n` | 2 |
| **total** | **41** |

The host loop's guard is `n < REQ_MAX - 32`, i.e. `n < 480`. The guard is tested
*before* each write, so the final write occurs at `n == 479` and leaves **`n == 480`**.

Then 41 unguarded bytes: `480 + 41 = 521` into `req[512]`.

**A 9-byte out-of-bounds write.** The reserve is `32`; the tail needs `41`.

Both other segments are fine — the path loop's `REQ_MAX - 64` reserve comfortably covers
the 17-byte `" HTTP/1.0\r\nHost: "` that follows it. Only the last reserve is wrong, and
only the last write is unguarded.

## Reachability

`host` is filled by `parse_url` (`browser.c:294-331`) from the URL — the address bar or
an `href` on a fetched page. Reaching `n == 480` needs the path and host together to
push past the boundary, which a crafted or merely long URL does.

`req` is a **static** `u8` array, so the 9 bytes land in whatever static object the
linker placed next — not in a guard page, and with no MMU protection in ring 0. This
repo's own rule applies: *an out-of-bounds write that does not fault landed in the next
mapping.* Nothing will crash; something adjacent will quietly change.

## Fix

Two lines, and prefer both:

```c
/* 1. make the reserve match the tail */
for (int i = 0; host[i] && n < REQ_MAX - 48; i++) req[n++] = (u8)host[i];

/* 2. and stop trusting a hand-counted constant */
static const char tail[] = "\r\nUser-Agent: zlOS\r\nConnection: close\r\n\r\n";
_Static_assert(REQ_MAX - 48 + sizeof(tail) - 1 <= REQ_MAX,
               "build_request's host reserve no longer covers the request tail");
for (const char *t = tail; *t; t++) { if (n >= REQ_MAX) return 0; req[n++] = (u8)*t; }
```

The `_Static_assert` is the part worth insisting on. The defect is a hand-counted reserve
drifting out of step with a string literal someone later edited — exactly the class a
compile-time check removes permanently, and exactly the shape of check this repo already
uses well (`fb.c:203-215`, `memmap.h`).

## Watch for the sibling

`build_request`'s structure — *"guarded, guarded, then an unguarded literal"* — is worth
grepping for elsewhere. The two guarded loops make the function *look* careful, which is
why the third write reads as safe.

---

## Three more from the same wave, not hand-verified

| file:line | claim |
|---|---|
| `http.c:227` | **critical** — any response over ~49 KB deadlocks the fetch permanently: `tcp_recv(resp, 0)` is a no-op, so a full buffer stops draining TCP and nothing ever progresses |
| `css.c:124` | `trim_cs`'s trailing-comment recovery is **quadratic** — a 100 KB run of `/*` freezes the machine. A denial of service from a stylesheet, though `css.c` is not currently compiled |
| `browser.c:103` | `sset` over-reads `html.c`'s deliberately NUL-free arena — a link click copies 127 bytes of whatever follows the href |

`http.c:227` is the one to check next: it is a **critical**, it needs no crafted input
(any largish page), and the `memory-safety` and `http` lenses reported it independently.
