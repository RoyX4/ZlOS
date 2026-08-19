/* runtest.c - `run hello.zl` actually executes, not just declines.
 *
 * exectest.c asserts every refusal, and keeps EX_LOADED by leaving lex_text
 * and fs_read unbound. This binary is the other half of that seam: it links
 * the real lexer/parser/interp and a filesystem that holds one file, so the
 * path exec.c takes in the shipping kernel is reached here without QEMU.
 *
 * hello.zl is `print(40 + 2)` - the same bytes fs_seed_hello() writes.
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdint.h>

typedef unsigned int u32;

int  exec_run(void);
int  exec_state(void);

#define EX_RAN  8
#define EX_FAIL 9

static char typed[256] = "hello.zl";
static char out[8192];
static size_t outn;

const char *term_argstr(void) { return typed; }

void term_say(const char *s)
{
    while (*s && outn < sizeof out - 1) out[outn++] = *s++;
    out[outn] = 0;
}

static unsigned char arena[1 << 20];
static unsigned long used;

unsigned long arena_capacity(void)  { return sizeof arena; }
unsigned long arena_available(void) { return sizeof arena - used; }
unsigned long arena_base_addr(void) { return (unsigned long)(uintptr_t)arena; }

void *arena_alloc(unsigned long n)
{
    if (n == 0) n = 1;
    if (used + n > sizeof arena) return 0;
    void *p = arena + used;
    used = (used + n + 7UL) & ~7UL;
    return p;
}

void arena_reset(void) { used = 0; }

void fb_text_prop(int x, int y, const char *s, unsigned int rgb)
{
    (void)x; (void)y; (void)s; (void)rgb;
}
int fb_text_prop_w(const char *s) { return s ? (int)strlen(s) * 8 : 0; }
int fb_text_prop_h(void) { return 16; }

static const char hello[] = "print(40 + 2)\n";

int fs_mounted(void) { return 1; }
int fs_find(const char *n) { return (n && strcmp(n, "hello.zl") == 0) ? 1 : -1; }
u32 fs_size(int idx) { return idx == 1 ? (u32)(sizeof hello - 1) : 0; }
int fs_read(int idx, void *dst, u32 max)
{
    if (idx != 1) return 0;
    u32 n = (u32)(sizeof hello - 1);
    if (n > max) n = max;
    memcpy(dst, hello, n);
    return (int)n;
}

int main(void)
{
    printf("runtest - kernel interpreter, hello.zl\n\n");
    arena_reset();
    int st = exec_run();
    printf("%s", out);
    if (st != EX_RAN) {
        printf("FAIL: state %d, want EX_RAN=%d\n", st, EX_RAN);
        return 1;
    }
    if (!strstr(out, "42") && !strstr(out, "finished")) {
        printf("FAIL: ran, but neither 42 nor 'finished' appeared\n");
        return 1;
    }
    printf("ok    run hello.zl executed (state EX_RAN)\n");
    return 0;
}
