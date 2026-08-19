/* pngtest.c - png.c against PNGs this file builds one byte at a time.
 *
 * THERE IS NO FIXTURE FILE, and that is the point. Every input below is
 * assembled here - signature, chunk, CRC, zlib header, deflate blocks, Adler -
 * so each test says exactly what is in the file it is testing, nothing goes
 * stale on disk, and the malformed cases can be built at all. Most of what a
 * decoder has to get right cannot be asked for from an encoder: no encoder
 * will produce a chunk whose length overflows, a palette index past the end of
 * its own palette, a filter byte of 5, or a stream that inflates to one byte
 * more than the image needs.
 *
 * That means this harness carries its OWN deflate: a stored-block writer, a
 * fixed-Huffman writer, and a dynamic-Huffman writer. The dynamic one gives
 * every literal a 9-bit code and the end-of-block symbol a 1-bit code, which
 * is a complete canonical code for any input at all, and it transmits its code
 * lengths using repeat code 16 - so png.c's code-length machinery is exercised
 * rather than merely present. The CRC-32 here is computed bit by bit while
 * png.c uses a table, on purpose: two implementations of the same polynomial
 * can agree by accident far less easily than one can agree with itself.
 *
 * THE ACCEPT CASES CHECK PIXEL VALUES, not "it returned a slot". A decoder
 * that returned a slot full of zeros would pass every "does it decode"
 * assertion ever written.
 *
 * THE REJECT CASES CHECK THE REASON, not merely the refusal - and each one
 * isolates a single fault. Where a test pokes a byte it recomputes the chunk's
 * CRC afterwards, or the CRC check would fire first and the test would pass
 * while proving nothing about the thing it names.
 *
 * The filter vectors in section 3 are computed BY HAND in the comments rather
 * than produced by an encoder written here. An encoder in this file would
 * share a wrong Paeth predictor with the decoder and the round trip would
 * still close.
 *
 *   cd kernel/hosttest && ./build.sh && ./pngtest
 */
#include <stdio.h>
#include "../png.h"

static int passed, failed;

static const char *whyname(int e)
{
    switch (e) {
    case PNG_OK:            return "PNG_OK";
    case PNG_E_SIG:         return "PNG_E_SIG";
    case PNG_E_UNSUPPORTED: return "PNG_E_UNSUPPORTED";
    case PNG_E_TOO_BIG:     return "PNG_E_TOO_BIG";
    case PNG_E_CORRUPT:     return "PNG_E_CORRUPT";
    case PNG_E_NO_ROOM:     return "PNG_E_NO_ROOM";
    case PNG_E_TRUNCATED:   return "PNG_E_TRUNCATED";
    default:                return "?";
    }
}

static void ok(const char *what, int cond)
{
    if (cond) { passed++; printf("  ok   %s\n", what); }
    else      { failed++; printf("  FAIL %s\n", what); }
}

/* the pixel arena, at exactly the budget png.h names */
static unsigned int px[PNG_ARENA_PX];

#define P(a, r, g, b) (((unsigned)(a) << 24) | ((unsigned)(r) << 16) | \
                       ((unsigned)(g) << 8)  | (unsigned)(b))
#define G(a, v)       P(a, v, v, v)

/* ------------------------------------------------------------ checksums */

/* bit by bit, deliberately not png.c's table */
static unsigned int t_crc(const unsigned char *p, int n)
{
    unsigned int c = 0xFFFFFFFFu;
    for (int i = 0; i < n; i++) {
        c ^= (unsigned int)p[i];
        for (int k = 0; k < 8; k++)
            c = (c & 1u) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
    }
    return c ^ 0xFFFFFFFFu;
}

static unsigned int t_adler(const unsigned char *p, int n)
{
    unsigned int a = 1, b = 0;
    for (int i = 0; i < n; i++) {
        a = (a + p[i]) % 65521u;
        b = (b + a) % 65521u;
    }
    return (b << 16) | a;
}

/* ------------------------------------------------------- the deflate side */

static unsigned char zs[1 << 20];
static int zn;
static unsigned int zacc;
static int znb;

static void wbyte(int b) { zs[zn++] = (unsigned char)b; }

static void wbit(unsigned int b)
{
    zacc |= (b & 1u) << znb;
    if (++znb == 8) { zs[zn++] = (unsigned char)zacc; zacc = 0; znb = 0; }
}

static void wbits(unsigned int v, int n)          /* value, low bit first */
{
    for (int i = 0; i < n; i++) wbit(v >> i);
}

static void wcode(unsigned int c, int n)          /* Huffman code, high bit first */
{
    for (int i = n - 1; i >= 0; i--) wbit(c >> i);
}

static void walign(void)
{
    if (znb) { zs[zn++] = (unsigned char)zacc; zacc = 0; znb = 0; }
}

static void zstart(void)
{
    zn = 0; zacc = 0; znb = 0;
    wbyte(0x78); wbyte(0x01);        /* deflate, 32K window, no dictionary */
}

static void zfinish(unsigned int adler)
{
    walign();
    wbyte((int)(adler >> 24) & 0xFF);
    wbyte((int)(adler >> 16) & 0xFF);
    wbyte((int)(adler >> 8) & 0xFF);
    wbyte((int)adler & 0xFF);
}

static void def_stored(const unsigned char *raw, int n, int nblocks)
{
    int per, i = 0;
    zstart();
    if (nblocks < 1) nblocks = 1;
    per = (n + nblocks - 1) / nblocks;
    if (per < 1) per = 1;
    do {
        int m = n - i, last;
        if (m > per) m = per;
        last = (i + m >= n);
        wbits((unsigned)last, 1);
        wbits(0, 2);                          /* stored */
        walign();
        wbyte(m & 0xFF); wbyte((m >> 8) & 0xFF);
        wbyte((~m) & 0xFF); wbyte(((~m) >> 8) & 0xFF);
        for (int k = 0; k < m; k++) wbyte(raw[i + k]);
        i += m;
    } while (i < n);
    zfinish(t_adler(raw, n));
}

/* Fixed Huffman, literals only. 0..143 are 8-bit codes 0x30.., 144..255 are
 * 9-bit codes 0x190.., and the end-of-block symbol 256 is the 7-bit code 0. */
static void def_fixed(const unsigned char *raw, int n)
{
    zstart();
    wbits(1, 1);                              /* BFINAL */
    wbits(1, 2);                              /* fixed  */
    for (int i = 0; i < n; i++) {
        int l = raw[i];
        if (l < 144) wcode(0x30u + (unsigned)l, 8);
        else         wcode(0x190u + (unsigned)(l - 144), 9);
    }
    wcode(0, 7);
    zfinish(t_adler(raw, n));
}

/* Dynamic Huffman over a code chosen so it works for ANY input: 256 literals
 * of length 9 is exactly half the code space and symbol 256 at length 1 is the
 * other half, so the code is complete with no frequency counting. Literal L is
 * then the 9-bit code 0x100|L and end-of-block is the 1-bit code 0.
 *
 * The 258 code lengths are transmitted as one 9 followed by 51 uses of repeat
 * code 16 (five each), then two 1s - which is the part of a dynamic block that
 * a decoder can get wrong without any other test noticing. */
static void def_dynamic(const unsigned char *raw, int n)
{
    /* code lengths for the code-length alphabet, in the order the format
     * transmits them: 16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15.
     * sym16 -> 1 bit, sym9 -> 2 bits, sym1 -> 2 bits. */
    static const int cl[18] = { 1, 0, 0, 0, 0, 0, 2, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0, 2 };
    zstart();
    wbits(1, 1);                              /* BFINAL  */
    wbits(2, 2);                              /* dynamic */
    wbits(0, 5);                              /* HLIT  -> 257 */
    wbits(0, 5);                              /* HDIST -> 1   */
    wbits(14, 4);                             /* HCLEN -> 18  */
    for (int i = 0; i < 18; i++) wbits((unsigned)cl[i], 3);
    /* canonical: sym16 = "0", sym1 = "10", sym9 = "11" */
    wcode(3, 2);                              /* length 9, once      */
    for (int i = 0; i < 51; i++) {            /* ...then 51 x five   */
        wcode(0, 1);
        wbits(2, 2);                          /* repeat count 5      */
    }
    wcode(2, 2);                              /* symbol 256 -> len 1 */
    wcode(2, 2);                              /* the one distance    */
    for (int i = 0; i < n; i++) wcode(0x100u | (unsigned)raw[i], 9);
    wcode(0, 1);                              /* end of block        */
    zfinish(t_adler(raw, n));
}

/* ---------------------------------------------------------- the PNG side */

static unsigned char img[1 << 20];

static void be(unsigned char *p, unsigned int v)
{
    p[0] = (unsigned char)(v >> 24); p[1] = (unsigned char)(v >> 16);
    p[2] = (unsigned char)(v >> 8);  p[3] = (unsigned char)v;
}

static unsigned int rd32(const unsigned char *p)
{
    return ((unsigned)p[0] << 24) | ((unsigned)p[1] << 16) |
           ((unsigned)p[2] << 8) | (unsigned)p[3];
}

static int chunk(unsigned char *out, int off, const char *type,
                 const unsigned char *data, int n)
{
    be(out + off, (unsigned int)n);
    for (int i = 0; i < 4; i++) out[off + 4 + i] = (unsigned char)type[i];
    for (int i = 0; i < n; i++) out[off + 8 + i] = data[i];
    be(out + off + 8 + n, t_crc(out + off + 4, 4 + n));
    return off + 12 + n;
}

#define M_STORED  0
#define M_FIXED   1
#define M_DYNAMIC 2
#define M_PREBUILT 3        /* zs[0..zn) was filled by the caller */

struct spec {
    int w, h, depth, colour, interlace;
    const unsigned char *plte; int nplte;      /* bytes, not entries */
    const unsigned char *trns; int ntrns;
    const unsigned char *raw;  int nraw;
    int method;
    int nidat;                                 /* split the stream over this many */
    int nblocks;                               /* deflate blocks (stored only)    */
    int extra;                                 /* insert an unknown chunk         */
    int no_ihdr, no_idat, no_iend;
};

static int build(const struct spec *s, unsigned char *out)
{
    static const unsigned char sig[8] = { 137, 80, 78, 71, 13, 10, 26, 10 };
    unsigned char ihdr[13];
    int off, k, per, p;

    for (int i = 0; i < 8; i++) out[i] = sig[i];
    off = 8;

    if (!s->no_ihdr) {
        be(ihdr, (unsigned)s->w);
        be(ihdr + 4, (unsigned)s->h);
        ihdr[8] = (unsigned char)s->depth;
        ihdr[9] = (unsigned char)s->colour;
        ihdr[10] = 0; ihdr[11] = 0;
        ihdr[12] = (unsigned char)s->interlace;
        off = chunk(out, off, "IHDR", ihdr, 13);
    }
    if (s->extra)
        off = chunk(out, off, "tEXt", (const unsigned char *)"zl\0x", 4);
    if (s->nplte) off = chunk(out, off, "PLTE", s->plte, s->nplte);
    if (s->ntrns) off = chunk(out, off, "tRNS", s->trns, s->ntrns);

    if (s->method == M_FIXED)        def_fixed(s->raw, s->nraw);
    else if (s->method == M_DYNAMIC) def_dynamic(s->raw, s->nraw);
    else if (s->method == M_STORED)  def_stored(s->raw, s->nraw, s->nblocks);

    if (!s->no_idat) {
        k = s->nidat ? s->nidat : 1;
        per = (zn + k - 1) / k;
        if (per < 1) per = 1;
        p = 0;
        for (int j = 0; j < k; j++) {
            int m = zn - p;
            if (m > per) m = per;
            if (m < 0) m = 0;
            off = chunk(out, off, "IDAT", zs + p, m);
            p += m;
        }
        if (p < zn) off = chunk(out, off, "IDAT", zs + p, zn - p);
    }
    if (!s->no_iend) off = chunk(out, off, "IEND", zs, 0);
    return off;
}

/* find the nth chunk of a type; returns the offset of its length field */
static int find(unsigned char *b, int len, const char *type, int nth)
{
    int off = 8, seen = 0;
    while (off + 8 <= len) {
        int cl = (int)rd32(b + off);
        if (b[off+4] == (unsigned char)type[0] && b[off+5] == (unsigned char)type[1] &&
            b[off+6] == (unsigned char)type[2] && b[off+7] == (unsigned char)type[3]) {
            if (seen == nth) return off;
            seen++;
        }
        if (cl < 0 || off + 12 + cl > len) return -1;
        off += 12 + cl;
    }
    return -1;
}

/* recompute a chunk's CRC after poking at its data, so the test that follows
 * fails for the reason it names instead of failing the CRC check */
static void refix(unsigned char *b, int coff)
{
    int n = (int)rd32(b + coff);
    be(b + coff + 8 + n, t_crc(b + coff + 4, 4 + n));
}

/* --------------------------------------------------------------- checks */

static void accept(const char *what, const struct spec *s, const unsigned int *want)
{
    int n = build(s, img);
    int sl = png_decode(img, n);
    const unsigned int *got;

    if (sl < 0) {
        failed++;
        printf("  FAIL %s   (refused: %s)\n", what, whyname(png_why()));
        return;
    }
    if (png_w(sl) != s->w || png_h(sl) != s->h) {
        failed++;
        printf("  FAIL %s   (size %dx%d, wanted %dx%d)\n",
               what, png_w(sl), png_h(sl), s->w, s->h);
        return;
    }
    got = png_pixels(sl);
    for (int i = 0; i < s->w * s->h; i++) {
        if (got[i] != want[i]) {
            failed++;
            printf("  FAIL %s   pixel %d (%d,%d): want %08X got %08X\n",
                   what, i, i % s->w, i / s->w, want[i], got[i]);
            return;
        }
    }
    passed++;
    printf("  ok   %s\n", what);
}

static void reject(const char *what, const unsigned char *b, int len, int want)
{
    int sl = png_decode(b, len);
    if (sl >= 0) {
        failed++;
        printf("  FAIL %s   (ACCEPTED, slot %d)\n", what, sl);
        return;
    }
    if (png_why() != want) {
        failed++;
        printf("  FAIL %s   want %s, got %s\n", what, whyname(want),
               whyname(png_why()));
        return;
    }
    passed++;
    printf("  ok   %s\n", what);
}

/* ====================================================================== */

int main(void)
{
    png_set_arena(px, PNG_ARENA_PX);

    printf("pngtest: png.c against PNGs built byte by byte\n");
    printf("\n1. colour types and bit depths, checked as pixel VALUES\n");

    {
        static const unsigned char raw[] = { 0, 0x00, 0xFF, 0, 0x40, 0x80 };
        static const unsigned int want[] = { G(255,0x00), G(255,0xFF),
                                             G(255,0x40), G(255,0x80) };
        struct spec s = { .w=2, .h=2, .depth=8, .colour=0,
                          .raw=raw, .nraw=sizeof raw };
        accept("grey 8-bit", &s, want);
    }
    {   /* 0xB1 = 1011 0001 */
        static const unsigned char raw[] = { 0, 0xB1 };
        static const unsigned int want[] = { G(255,255), G(255,0), G(255,255),
                                             G(255,255), G(255,0), G(255,0),
                                             G(255,0), G(255,255) };
        struct spec s = { .w=8, .h=1, .depth=1, .colour=0,
                          .raw=raw, .nraw=sizeof raw };
        accept("grey 1-bit, eight pixels in one byte", &s, want);
    }
    {   /* 0x1B = 00 01 10 11 -> 0, 85, 170, 255 */
        static const unsigned char raw[] = { 0, 0x1B };
        static const unsigned int want[] = { G(255,0), G(255,85),
                                             G(255,170), G(255,255) };
        struct spec s = { .w=4, .h=1, .depth=2, .colour=0,
                          .raw=raw, .nraw=sizeof raw };
        accept("grey 2-bit", &s, want);
    }
    {   /* nibbles 0,15,3,7 -> 0, 255, 51, 119 */
        static const unsigned char raw[] = { 0, 0x0F, 0x37 };
        static const unsigned int want[] = { G(255,0), G(255,255),
                                             G(255,51), G(255,119) };
        struct spec s = { .w=4, .h=1, .depth=4, .colour=0,
                          .raw=raw, .nraw=sizeof raw };
        accept("grey 4-bit", &s, want);
    }
    {   /* 16-bit keeps the high byte and drops the low one */
        static const unsigned char raw[] = { 0, 0x12, 0x34, 0xAB, 0xCD };
        static const unsigned int want[] = { G(255,0x12), G(255,0xAB) };
        struct spec s = { .w=2, .h=1, .depth=16, .colour=0,
                          .raw=raw, .nraw=sizeof raw };
        accept("grey 16-bit downsamples to 8", &s, want);
    }
    {
        static const unsigned char raw[] = { 0, 10,20,30, 40,50,60 };
        static const unsigned int want[] = { P(255,10,20,30), P(255,40,50,60) };
        struct spec s = { .w=2, .h=1, .depth=8, .colour=2,
                          .raw=raw, .nraw=sizeof raw };
        accept("RGB 8-bit", &s, want);
    }
    {
        static const unsigned char raw[] = { 0, 0x11,0x22, 0x33,0x44, 0x55,0x66,
                                                0x77,0x88, 0x99,0xAA, 0xBB,0xCC };
        static const unsigned int want[] = { P(255,0x11,0x33,0x55),
                                             P(255,0x77,0x99,0xBB) };
        struct spec s = { .w=2, .h=1, .depth=16, .colour=2,
                          .raw=raw, .nraw=sizeof raw };
        accept("RGB 16-bit", &s, want);
    }
    {
        static const unsigned char pal[] = { 255,0,0, 0,255,0, 0,0,255 };
        static const unsigned char raw[] = { 0, 0, 1, 2 };
        static const unsigned int want[] = { P(255,255,0,0), P(255,0,255,0),
                                             P(255,0,0,255) };
        struct spec s = { .w=3, .h=1, .depth=8, .colour=3,
                          .plte=pal, .nplte=sizeof pal,
                          .raw=raw, .nraw=sizeof raw };
        accept("palette 8-bit", &s, want);
    }
    {   /* 0x40 = 0100 0000 -> indices 0, 1 */
        static const unsigned char pal[] = { 1,2,3, 4,5,6 };
        static const unsigned char raw[] = { 0, 0x40 };
        static const unsigned int want[] = { P(255,1,2,3), P(255,4,5,6) };
        struct spec s = { .w=2, .h=1, .depth=1, .colour=3,
                          .plte=pal, .nplte=sizeof pal,
                          .raw=raw, .nraw=sizeof raw };
        accept("palette 1-bit", &s, want);
    }
    {   /* 0x1B -> indices 0,1,2,3 */
        static const unsigned char pal[] = { 0x10,0x11,0x12, 0x20,0x21,0x22,
                                             0x30,0x31,0x32, 0x40,0x41,0x42 };
        static const unsigned char raw[] = { 0, 0x1B };
        static const unsigned int want[] = { P(255,0x10,0x11,0x12),
                                             P(255,0x20,0x21,0x22),
                                             P(255,0x30,0x31,0x32),
                                             P(255,0x40,0x41,0x42) };
        struct spec s = { .w=4, .h=1, .depth=2, .colour=3,
                          .plte=pal, .nplte=sizeof pal,
                          .raw=raw, .nraw=sizeof raw };
        accept("palette 2-bit", &s, want);
    }
    {   /* nibbles 0,1,2 and a pad nibble the width must stop it reading */
        static const unsigned char pal[] = { 255,0,0, 0,255,0, 0,0,255 };
        static const unsigned char raw[] = { 0, 0x01, 0x20 };
        static const unsigned int want[] = { P(255,255,0,0), P(255,0,255,0),
                                             P(255,0,0,255) };
        struct spec s = { .w=3, .h=1, .depth=4, .colour=3,
                          .plte=pal, .nplte=sizeof pal,
                          .raw=raw, .nraw=sizeof raw };
        accept("palette 4-bit, odd width", &s, want);
    }
    /* twelve slots is the whole table, and this section wants sixteen images */
    png_reset();
    {
        static const unsigned char raw[] = { 0, 100,255, 200,128 };
        static const unsigned int want[] = { G(255,100), G(128,200) };
        struct spec s = { .w=2, .h=1, .depth=8, .colour=4,
                          .raw=raw, .nraw=sizeof raw };
        accept("grey+alpha 8-bit", &s, want);
    }
    {
        static const unsigned char raw[] = { 0, 0x11,0x22, 0x80,0x00 };
        static const unsigned int want[] = { G(0x80,0x11) };
        struct spec s = { .w=1, .h=1, .depth=16, .colour=4,
                          .raw=raw, .nraw=sizeof raw };
        accept("grey+alpha 16-bit", &s, want);
    }
    {
        static const unsigned char raw[] = { 0, 1,2,3,4, 5,6,7,8 };
        static const unsigned int want[] = { P(4,1,2,3), P(8,5,6,7) };
        struct spec s = { .w=2, .h=1, .depth=8, .colour=6,
                          .raw=raw, .nraw=sizeof raw };
        accept("RGBA 8-bit", &s, want);
    }
    {
        static const unsigned char raw[] = { 0, 0x11,0x01, 0x22,0x02,
                                                0x33,0x03, 0x44,0x04 };
        static const unsigned int want[] = { P(0x44,0x11,0x22,0x33) };
        struct spec s = { .w=1, .h=1, .depth=16, .colour=6,
                          .raw=raw, .nraw=sizeof raw };
        accept("RGBA 16-bit", &s, want);
    }
    {
        static const unsigned char raw[] = { 0, 7 };
        static const unsigned int want[] = { G(255,7) };
        struct spec s = { .w=1, .h=1, .depth=8, .colour=0,
                          .raw=raw, .nraw=sizeof raw };
        accept("a one pixel image", &s, want);
    }

    printf("\n2. tRNS, which is compared at the image's OWN bit depth\n");
    png_reset();
    {
        static const unsigned char pal[] = { 255,0,0, 0,255,0, 0,0,255 };
        static const unsigned char tr[]  = { 0x00, 0x80 };
        static const unsigned char raw[] = { 0, 0, 1, 2 };
        static const unsigned int want[] = { P(0,255,0,0), P(0x80,0,255,0),
                                             P(255,0,0,255) };
        struct spec s = { .w=3, .h=1, .depth=8, .colour=3,
                          .plte=pal, .nplte=sizeof pal,
                          .trns=tr, .ntrns=sizeof tr,
                          .raw=raw, .nraw=sizeof raw };
        accept("tRNS gives a palette per-entry alpha, short list defaults to 255",
               &s, want);
    }
    {
        static const unsigned char tr[]  = { 0x00, 0x40 };    /* grey 64 */
        static const unsigned char raw[] = { 0, 0x40, 0x41 };
        static const unsigned int want[] = { G(0,0x40), G(255,0x41) };
        struct spec s = { .w=2, .h=1, .depth=8, .colour=0,
                          .trns=tr, .ntrns=sizeof tr,
                          .raw=raw, .nraw=sizeof raw };
        accept("tRNS makes one grey value transparent", &s, want);
    }
    {
        static const unsigned char tr[]  = { 0,10, 0,20, 0,30 };
        static const unsigned char raw[] = { 0, 10,20,30, 10,20,31 };
        static const unsigned int want[] = { P(0,10,20,30), P(255,10,20,31) };
        struct spec s = { .w=2, .h=1, .depth=8, .colour=2,
                          .trns=tr, .ntrns=sizeof tr,
                          .raw=raw, .nraw=sizeof raw };
        accept("tRNS makes one RGB triple transparent, and only that one",
               &s, want);
    }
    {   /* the value is 3 at four bits, NOT 3 after scaling to 51 - a decoder
         * that compares the scaled sample matches nothing here */
        static const unsigned char tr[]  = { 0x00, 0x03 };
        static const unsigned char raw[] = { 0, 0x3F };
        static const unsigned int want[] = { G(0,51), G(255,255) };
        struct spec s = { .w=2, .h=1, .depth=4, .colour=0,
                          .trns=tr, .ntrns=sizeof tr,
                          .raw=raw, .nraw=sizeof raw };
        accept("tRNS at 4 bits compares the raw sample, not the scaled one",
               &s, want);
    }

    printf("\n3. the five filters, against hand-computed vectors\n");
    png_reset();
    {
        /* w=3, bpp=1. Each row's filter bytes were worked out by hand:
         *   row0 None    10 20 30                     -> 10  20  30
         *   row1 Sub     40 10 10                     -> 40  50  60
         *   row2 Up      30 30 30  over  40 50 60     -> 70  80  90
         *   row3 Average 65 20 20  over  70 80 90     -> 100 110 120
         *        i0: a=0   b=70  -> 35, 100-35 = 65
         *        i1: a=100 b=80  -> 90, 110-90 = 20
         *        i2: a=110 b=90  -> 100, 120-100 = 20
         *   row4 Paeth   30 10 10 over 100 110 120    -> 130 140 150
         *        i0: a=0 b=100 c=0 -> b=100, 130-100 = 30
         *        i1: a=130 b=110 c=100, p=140, pa=10 pb=30 pc=40 -> a, 10
         *        i2: a=140 b=120 c=110, p=150, pa=10 pb=30 pc=40 -> a, 10 */
        static const unsigned char raw[] = {
            0, 10, 20, 30,
            1, 40, 10, 10,
            2, 30, 30, 30,
            3, 65, 20, 20,
            4, 30, 10, 10
        };
        static const unsigned int want[] = {
            G(255,10),  G(255,20),  G(255,30),
            G(255,40),  G(255,50),  G(255,60),
            G(255,70),  G(255,80),  G(255,90),
            G(255,100), G(255,110), G(255,120),
            G(255,130), G(255,140), G(255,150)
        };
        struct spec s = { .w=3, .h=5, .depth=8, .colour=0,
                          .raw=raw, .nraw=sizeof raw };
        accept("None, Sub, Up, Average and Paeth on consecutive rows", &s, want);
    }
    {
        /* THE bpp TEST. RGB 8-bit means bpp is 3, so Sub subtracts the byte
         * three to the left. With bpp taken as 1 the second row decodes to
         * 70 150 240 ... instead. */
        static const unsigned char raw[] = {
            0, 10,20,30, 40,50,60,
            1, 70,80,90, 30,30,30
        };
        static const unsigned int want[] = {
            P(255,10,20,30),  P(255,40,50,60),
            P(255,70,80,90),  P(255,100,110,120)
        };
        struct spec s = { .w=2, .h=2, .depth=8, .colour=2,
                          .raw=raw, .nraw=sizeof raw };
        accept("Sub steps by bpp=3 on RGB, not by one byte", &s, want);
    }
    {
        /* bpp is 1 below eight bits even though a pixel is half a byte */
        static const unsigned char raw[] = { 0, 0x12, 0x34,
                                             1, 0x56, 0x22 };
        static const unsigned int want[] = {
            G(255,17),  G(255,34),  G(255,51),  G(255,68),
            G(255,85),  G(255,102), G(255,119), G(255,136)
        };
        struct spec s = { .w=4, .h=2, .depth=4, .colour=0,
                          .raw=raw, .nraw=sizeof raw };
        accept("Sub at 4 bits steps by one byte (bpp = max(1, bits/8))",
               &s, want);
    }
    {
        /* bpp is 6 for RGB at 16 bits. Paeth row worked out by hand:
         *   prev = 10 00 20 00 30 00 40 00 50 00 60 00
         *   want = 70 00 80 00 90 00 A0 00 B0 00 C0 00
         *   i<6  predictor is prev[i]:      60 00 60 00 60 00
         *   i=6  a=70 b=40 c=10, p=A0, pa=30 pb=60 pc=90 -> a=70, A0-70=30
         *   i=8  a=80 b=50 c=20, p=B0, pa=30 -> a=80, B0-80=30
         *   i=10 a=90 b=60 c=30, p=C0, pa=30 -> a=90, C0-90=30 */
        static const unsigned char raw[] = {
            0, 0x10,0, 0x20,0, 0x30,0,  0x40,0, 0x50,0, 0x60,0,
            4, 0x60,0, 0x60,0, 0x60,0,  0x30,0, 0x30,0, 0x30,0
        };
        static const unsigned int want[] = {
            P(255,0x10,0x20,0x30), P(255,0x40,0x50,0x60),
            P(255,0x70,0x80,0x90), P(255,0xA0,0xB0,0xC0)
        };
        struct spec s = { .w=2, .h=2, .depth=16, .colour=2,
                          .raw=raw, .nraw=sizeof raw };
        accept("Paeth steps by bpp=6 on 16-bit RGB", &s, want);
    }
    {
        /* THE PAETH TIE. Every other Paeth vector above has one strictly
         * smallest distance, so all of them pass with the comparisons written
         * as < instead of <=, and that is the whole content of the rule.
         *
         *   prev = 2 3, and we want cur = 0 100
         *   i=0  below bpp, predictor is prev[0]=2, so 0-2 = 254
         *   i=1  a=0 b=3 c=2, p=1, pa=1 pb=2 pc=1
         *        pa ties with pc and beats pb, so the predictor is a=0 and
         *        the byte is 100. With < in place of <= the first test fails
         *        on pa<pc, pb<=pc fails too, and the predictor becomes c=2 -
         *        so that decoder reads 102 here. Two apart, on purpose: a
         *        difference of one could be an off-by-one anywhere. */
        static const unsigned char raw[] = { 0, 2, 3,
                                             4, 254, 100 };
        static const unsigned int want[] = { G(255,2), G(255,3),
                                             G(255,0), G(255,100) };
        struct spec s = { .w=2, .h=2, .depth=8, .colour=0,
                          .raw=raw, .nraw=sizeof raw };
        accept("Paeth breaks a tie towards the left pixel, not upward",
               &s, want);
    }

    printf("\n4. the deflate underneath: three block types, and many IDATs\n");
    png_reset();
    {
        /* one 16x16 RGB image, put through every path that can carry it */
        static unsigned char raw[16 * (1 + 48)];
        static unsigned int want[16 * 16];
        int n = 0;
        for (int y = 0; y < 16; y++) {
            raw[n++] = 0;
            for (int x = 0; x < 16; x++) {
                unsigned char r = (unsigned char)(x * 7 + 1);
                unsigned char g = (unsigned char)(y * 5 + 2);
                unsigned char b = (unsigned char)(x * 3 + y);
                raw[n++] = r; raw[n++] = g; raw[n++] = b;
                want[y * 16 + x] = P(255, r, g, b);
            }
        }
        {
            struct spec s = { .w=16, .h=16, .depth=8, .colour=2,
                              .raw=raw, .nraw=n };
            accept("one stored block", &s, want);
            s.nblocks = 7;
            accept("seven stored blocks", &s, want);
            s.nblocks = 0; s.method = M_FIXED;
            accept("fixed Huffman", &s, want);
            s.method = M_DYNAMIC;
            accept("dynamic Huffman, code lengths sent with repeat code 16",
                   &s, want);
            /* THE ONE THAT SEPARATES ENCODERS. Every encoder splits its zlib
             * stream over IDATs differently, and a decoder that inflates each
             * chunk on its own works against exactly one of them. */
            s.method = M_STORED; s.nidat = 3;
            accept("the zlib stream split over 3 IDATs", &s, want);
            s.nidat = 11;
            accept("...over 11 IDATs, cutting mid-block", &s, want);
            s.nidat = 400;
            accept("...over 400 IDATs, most of them empty", &s, want);
            s.nidat = 0; s.extra = 1;
            accept("an unknown chunk between IHDR and IDAT is skipped",
                   &s, want);
        }
    }
    {
        /* A LENGTH/DISTANCE PAIR, which none of the encoders above emit.
         * literal 0, literal 65, then match length 7 distance 1 - an
         * overlapping copy, which is the case a naive memmove gets wrong. */
        static const unsigned char raw[] = { 0, 65,65,65,65,65,65,65,65 };
        static const unsigned int want[] = { G(255,65), G(255,65), G(255,65),
                                             G(255,65), G(255,65), G(255,65),
                                             G(255,65), G(255,65) };
        struct spec s = { .w=8, .h=1, .depth=8, .colour=0, .method=M_PREBUILT };
        zstart();
        wbits(1, 1); wbits(1, 2);          /* final, fixed */
        wcode(0x30u + 0, 8);               /* literal 0  (the filter byte) */
        wcode(0x30u + 65, 8);              /* literal 65 */
        wcode(5, 7);                       /* symbol 261 -> length 7 */
        wcode(0, 5);                       /* distance symbol 0 -> 1 */
        wcode(0, 7);                       /* end of block */
        zfinish(t_adler(raw, (int)sizeof raw));
        accept("a match with distance 1 copies as it writes", &s, want);
    }

    printf("\n5. Adam7, generated from the specification's own pass grid\n");
    png_reset();
    {
        /* The 8x8 grid of pass numbers, written out rather than derived, so
         * this is not the same arithmetic png.c uses checking itself. */
        static const int grid[8][8] = {
            { 1,6,4,6,2,6,4,6 },
            { 7,7,7,7,7,7,7,7 },
            { 5,6,5,6,5,6,5,6 },
            { 7,7,7,7,7,7,7,7 },
            { 3,6,4,6,3,6,4,6 },
            { 7,7,7,7,7,7,7,7 },
            { 5,6,5,6,5,6,5,6 },
            { 7,7,7,7,7,7,7,7 }
        };
        static unsigned char raw[4096];
        static unsigned int want[64 * 64];
        /* Sizes chosen so passes come out empty and so rows end mid-byte at
         * the sub-byte depths - a pass's row length rounds up on its OWN
         * width, and a decoder that rounded on the image width instead walks
         * out of step from the second pass onwards. */
        const int cases[7][3] = {
            { 8, 8, 8 }, { 5, 3, 8 }, { 17, 9, 8 },
            { 9, 9, 1 }, { 8, 8, 1 }, { 11, 5, 2 }, { 13, 6, 4 }
        };

        for (int c = 0; c < 7; c++) {
            int w = cases[c][0], h = cases[c][1], d = cases[c][2];
            int maxv = (1 << d) - 1, n = 0;
            char name[64];
            for (int y = 0; y < h; y++)
                for (int x = 0; x < w; x++) {
                    int v = (x * 13 + y * 7) & maxv;
                    unsigned s8 = (d == 8) ? (unsigned)v
                                : (d == 4) ? (unsigned)v * 17u
                                : (d == 2) ? (unsigned)v * 85u
                                           : (unsigned)v * 255u;
                    want[y * w + x] = G(255, s8);
                }
            for (int p = 1; p <= 7; p++) {
                int first = 1;
                for (int y = 0; y < h; y++) {
                    int cnt = 0, at, bit;
                    for (int x = 0; x < w; x++)
                        if (grid[y & 7][x & 7] == p) cnt++;
                    if (!cnt) continue;
                    /* THE FIRST ROW OF EVERY PASS USES Up. With the previous
                     * row taken as zeros - which is what "the filter history
                     * resets at the start of a pass" means - Up is the
                     * identity, so the bytes below are the samples unchanged.
                     * A decoder that carried the last row of the previous pass
                     * across adds it here and every pass after the first
                     * decodes wrong. Later rows use None, so this asserts the
                     * reset and nothing else. */
                    raw[n++] = first ? 2 : 0;
                    first = 0;
                    at = n; bit = 0;
                    for (int k = 0; k < (cnt * d + 7) / 8; k++) raw[at + k] = 0;
                    for (int x = 0; x < w; x++)
                        if (grid[y & 7][x & 7] == p) {
                            int v = (x * 13 + y * 7) & maxv;
                            raw[at + (bit >> 3)] |=
                                (unsigned char)(v << (8 - d - (bit & 7)));
                            bit += d;
                        }
                    n = at + (cnt * d + 7) / 8;
                }
            }
            {
                struct spec s = { .w=w, .h=h, .depth=d, .colour=0,
                                  .interlace=1, .raw=raw, .nraw=n };
                int m;
                snprintf(name, sizeof name, "Adam7 %dx%d at %d bits, every pixel",
                         w, h, d);
                accept(name, &s, want);

                /* One byte either side of exactly right, for every geometry.
                 * The expected total is a sum over seven passes of each
                 * pass's OWN rounded row length; a decoder that rounded on the
                 * image width instead gets a number that is too large, and
                 * then stops noticing surplus data entirely. Nothing in the
                 * accept case above can see that. */
                raw[n] = 0;
                s.nraw = n + 1;
                m = build(&s, img);
                snprintf(name, sizeof name, "...%dx%d/%d with one byte too many",
                         w, h, d);
                reject(name, img, m, PNG_E_CORRUPT);

                s.nraw = n - 1;
                m = build(&s, img);
                snprintf(name, sizeof name, "...%dx%d/%d with one byte too few",
                         w, h, d);
                reject(name, img, m, PNG_E_TRUNCATED);
            }
        }
    }

    printf("\n6. the arena and the slots\n");
    png_reset();
    ok("a fresh page has no slots", png_slots_used() == 0 && png_arena_used() == 0);
    {
        static const unsigned char raw[] = { 0, 1,2,3, 4,5,6, 0, 7,8,9, 10,11,12 };
        static const unsigned int want[] = { P(255,1,2,3), P(255,4,5,6),
                                             P(255,7,8,9), P(255,10,11,12) };
        struct spec s = { .w=2, .h=2, .depth=8, .colour=2,
                          .raw=raw, .nraw=sizeof raw };
        accept("a 2x2 image", &s, want);
        ok("one slot used, four pixels of arena",
           png_slots_used() == 1 && png_arena_used() == 4);
        accept("a second copy gets its own slot", &s, want);
        ok("two slots, eight pixels", png_slots_used() == 2 && png_arena_used() == 8);
        ok("slot 0's pixels are still slot 0's",
           png_pixels(0)[0] == P(255,1,2,3) && png_pixels(1)[0] == P(255,1,2,3));
        ok("slot 0 and slot 1 are different memory",
           png_pixels(0) != png_pixels(1));
        ok("an out of range slot has no pixels and no size",
           png_pixels(2) == 0 && png_pixels(-1) == 0 &&
           png_w(2) == 0 && png_h(-1) == 0);
        png_reset();
        ok("png_reset drops every slot",
           png_slots_used() == 0 && png_arena_used() == 0);
    }

    printf("\n7. not a PNG, and files that run out\n");
    png_reset();
    {
        static const unsigned char raw[] = { 0, 1,2,3, 4,5,6,
                                             0, 7,8,9, 10,11,12 };
        struct spec s = { .w=2, .h=2, .depth=8, .colour=2,
                          .raw=raw, .nraw=sizeof raw };
        static unsigned char good[4096];
        int n = build(&s, good);

        ok("the file this section corrupts is itself accepted",
           png_decode(good, n) >= 0);
        png_reset();

        for (int i = 0; i < 8; i++) {
            static unsigned char b[4096];
            char name[64];
            for (int k = 0; k < n; k++) b[k] = good[k];
            b[i] ^= 0x01;
            snprintf(name, sizeof name, "a flipped bit in signature byte %d", i);
            reject(name, b, n, PNG_E_SIG);
        }
        reject("an empty buffer", good, 0, PNG_E_SIG);
        reject("four bytes", good, 4, PNG_E_SIG);
        reject("the signature and nothing else", good, 8, PNG_E_TRUNCATED);

        /* every truncation, one byte at a time: none may be accepted and none
         * may read past the end - the sanitiser build is what proves the
         * second half */
        {
            int accepted = 0, walked = 0;
            for (int cut = 8; cut < n; cut++) {
                if (png_decode(good, cut) >= 0) accepted++;
                walked++;
            }
            ok("every truncation of a good file is refused",
               accepted == 0 && walked > 40);
        }
        reject("one byte short of the final CRC", good, n - 1, PNG_E_TRUNCATED);
        {
            int iend = find(good, n, "IEND", 0);
            reject("everything up to but not including IEND",
                   good, iend, PNG_E_TRUNCATED);
        }
    }

    printf("\n8. chunk lengths and CRCs, which are what an attacker writes\n");
    png_reset();
    {
        static const unsigned char raw[] = { 0, 1,2,3, 4,5,6,
                                             0, 7,8,9, 10,11,12 };
        struct spec s = { .w=2, .h=2, .depth=8, .colour=2,
                          .raw=raw, .nraw=sizeof raw };
        static unsigned char good[4096], b[4096];
        int n = build(&s, good), c;
        #define FRESH() do { for (int k = 0; k < n; k++) b[k] = good[k]; } while (0)

        FRESH(); c = find(b, n, "IDAT", 0);
        be(b + c, 0x00100000u);
        reject("a chunk length that runs past the end of the file",
               b, n, PNG_E_TRUNCATED);

        FRESH(); c = find(b, n, "IDAT", 0);
        be(b + c, 0xFFFFFFF0u);
        reject("a chunk length that would overflow when added to the offset",
               b, n, PNG_E_CORRUPT);

        FRESH(); c = find(b, n, "IDAT", 0);
        be(b + c, 0x80000000u);
        reject("a chunk length past the format's own 2^31 ceiling",
               b, n, PNG_E_CORRUPT);

        FRESH(); c = find(b, n, "IDAT", 0);
        b[c + 8] ^= 0x01;
        reject("one flipped bit in IDAT, with the CRC left alone",
               b, n, PNG_E_CORRUPT);

        FRESH(); c = find(b, n, "IHDR", 0);
        b[c + 8 + 12] ^= 0x01;
        reject("one flipped bit in IHDR, with the CRC left alone",
               b, n, PNG_E_CORRUPT);

        FRESH(); c = find(b, n, "IEND", 0);
        b[c + 8] ^= 0xFF;
        reject("a corrupt IEND CRC", b, n, PNG_E_CORRUPT);

        /* built with a byte of data rather than poked, so the file is the
         * right length for what it claims and the only thing wrong with it is
         * that IEND is defined to be empty */
        FRESH(); c = find(b, n, "IEND", 0);
        chunk(b, c, "IEND", b, 1);
        reject("an IEND carrying data", b, c + 13, PNG_E_CORRUPT);
        #undef FRESH
    }

    printf("\n9. an IHDR that says something impossible\n");
    png_reset();
    {
        static const unsigned char raw[] = { 0, 1,2,3, 4,5,6,
                                             0, 7,8,9, 10,11,12 };
        struct spec s = { .w=2, .h=2, .depth=8, .colour=2,
                          .raw=raw, .nraw=sizeof raw };
        static unsigned char good[4096], b[4096];
        int n = build(&s, good), c;
        #define POKE(off, val) do { \
            for (int k = 0; k < n; k++) b[k] = good[k]; \
            c = find(b, n, "IHDR", 0); b[c + 8 + (off)] = (unsigned char)(val); \
            refix(b, c); } while (0)

        for (int k = 0; k < n; k++) b[k] = good[k];
        c = find(b, n, "IHDR", 0);
        be(b + c + 8, 0u); refix(b, c);
        reject("a width of zero", b, n, PNG_E_CORRUPT);

        for (int k = 0; k < n; k++) b[k] = good[k];
        c = find(b, n, "IHDR", 0);
        be(b + c + 12, 0u); refix(b, c);
        reject("a height of zero", b, n, PNG_E_CORRUPT);

        for (int k = 0; k < n; k++) b[k] = good[k];
        c = find(b, n, "IHDR", 0);
        be(b + c + 8, (unsigned)PNG_MAX_W + 1u); refix(b, c);
        reject("one pixel wider than PNG_MAX_W", b, n, PNG_E_TOO_BIG);

        for (int k = 0; k < n; k++) b[k] = good[k];
        c = find(b, n, "IHDR", 0);
        be(b + c + 12, (unsigned)PNG_MAX_H + 1u); refix(b, c);
        reject("one pixel taller than PNG_MAX_H", b, n, PNG_E_TOO_BIG);

        for (int k = 0; k < n; k++) b[k] = good[k];
        c = find(b, n, "IHDR", 0);
        be(b + c + 8, 0x7FFFFFFFu); refix(b, c);
        reject("a width of 2^31-1, which the format allows and we do not",
               b, n, PNG_E_TOO_BIG);

        POKE(8, 3);  reject("a bit depth of 3", b, n, PNG_E_CORRUPT);
        POKE(8, 0);  reject("a bit depth of 0", b, n, PNG_E_CORRUPT);
        POKE(8, 32); reject("a bit depth of 32", b, n, PNG_E_CORRUPT);
        POKE(9, 7);  reject("a colour type of 7", b, n, PNG_E_CORRUPT);
        POKE(9, 1);  reject("a colour type of 1", b, n, PNG_E_CORRUPT);
        POKE(9, 3);  reject("depth 8 palette with no PLTE", b, n, PNG_E_CORRUPT);
        POKE(10, 1); reject("compression method 1", b, n, PNG_E_UNSUPPORTED);
        POKE(11, 1); reject("filter method 1", b, n, PNG_E_UNSUPPORTED);
        POKE(12, 2); reject("interlace method 2", b, n, PNG_E_UNSUPPORTED);
        #undef POKE

        {   /* 16-bit palette is not a combination the format defines */
            static const unsigned char pal[] = { 1,2,3, 4,5,6 };
            static const unsigned char r2[] = { 0, 0,0, 0,1 };
            struct spec bad = { .w=2, .h=1, .depth=16, .colour=3,
                                .plte=pal, .nplte=sizeof pal,
                                .raw=r2, .nraw=sizeof r2 };
            int m = build(&bad, b);
            reject("a 16-bit palette image", b, m, PNG_E_CORRUPT);
        }
        {
            static const unsigned char r2[] = { 0, 0x40 };
            struct spec bad = { .w=2, .h=1, .depth=1, .colour=2,
                                .raw=r2, .nraw=sizeof r2 };
            int m = build(&bad, b);
            reject("a 1-bit RGB image", b, m, PNG_E_CORRUPT);
        }
    }

    printf("\n10. chunks in the wrong place, or missing\n");
    png_reset();
    {
        static unsigned char b[4096];
        static const unsigned char raw[] = { 0, 1,2,3, 4,5,6,
                                             0, 7,8,9, 10,11,12 };
        {
            struct spec s = { .w=2, .h=2, .depth=8, .colour=2,
                              .raw=raw, .nraw=sizeof raw, .no_ihdr=1 };
            int n = build(&s, b);
            reject("an IDAT with no IHDR before it", b, n, PNG_E_CORRUPT);
        }
        {
            struct spec s = { .w=2, .h=2, .depth=8, .colour=2,
                              .raw=raw, .nraw=sizeof raw, .no_idat=1 };
            int n = build(&s, b);
            reject("no IDAT at all", b, n, PNG_E_CORRUPT);
        }
        {
            struct spec s = { .w=2, .h=2, .depth=8, .colour=2,
                              .raw=raw, .nraw=sizeof raw, .no_iend=1 };
            int n = build(&s, b);
            reject("no IEND", b, n, PNG_E_TRUNCATED);
        }
        {   /* two IHDRs: the second could redefine the geometry under a
             * decoder that had already sized its rows off the first */
            struct spec s = { .w=2, .h=2, .depth=8, .colour=2,
                              .raw=raw, .nraw=sizeof raw };
            int n = build(&s, b);
            int c = find(b, n, "IDAT", 0);
            for (int k = n - 1; k >= c; k--) b[k + 25] = b[k];
            for (int k = 0; k < 25; k++) b[c + k] = b[8 + k];
            reject("a second IHDR", b, n + 25, PNG_E_CORRUPT);
        }
        {   /* a PLTE after the pixel data has already started: built by
             * putting the palette where IEND was, rather than by moving bytes
             * around, so the file is otherwise perfectly ordinary */
            static const unsigned char pal[] = { 1,2,3, 4,5,6 };
            static const unsigned char r2[] = { 0, 0, 1 };
            struct spec s = { .w=2, .h=1, .depth=8, .colour=3,
                              .raw=r2, .nraw=sizeof r2 };
            int m = build(&s, b);
            int e = find(b, m, "IEND", 0);
            int add = chunk(b, e, "PLTE", pal, (int)sizeof pal);
            add = chunk(b, add, "IEND", pal, 0);
            reject("a PLTE after IDAT", b, add, PNG_E_CORRUPT);
        }
        {   /* IDATs separated by another chunk */
            struct spec s = { .w=2, .h=2, .depth=8, .colour=2,
                              .raw=raw, .nraw=sizeof raw, .nidat=2 };
            int n = build(&s, b);
            int c = find(b, n, "IDAT", 1);
            static unsigned char tail[4096];
            int tlen = n - c;
            for (int k = 0; k < tlen; k++) tail[k] = b[c + k];
            {
                int at = chunk(b, c, "tEXt", (const unsigned char *)"ab", 2);
                for (int k = 0; k < tlen; k++) b[at + k] = tail[k];
                reject("two IDATs with another chunk between them",
                       b, at + tlen, PNG_E_CORRUPT);
            }
        }
        {   /* PLTE with a length that is not a whole number of colours */
            static const unsigned char pal[] = { 1,2,3, 4,5 };
            static const unsigned char r2[] = { 0, 0, 1 };
            struct spec s = { .w=2, .h=1, .depth=8, .colour=3,
                              .plte=pal, .nplte=5,
                              .raw=r2, .nraw=sizeof r2 };
            int n = build(&s, b);
            reject("a PLTE of five bytes", b, n, PNG_E_CORRUPT);
        }
        {   /* tRNS where there is already an alpha channel */
            static const unsigned char tr[] = { 0, 1 };
            static const unsigned char r2[] = { 0, 1,2,3,4 };
            struct spec s = { .w=1, .h=1, .depth=8, .colour=6,
                              .trns=tr, .ntrns=sizeof tr,
                              .raw=r2, .nraw=sizeof r2 };
            int n = build(&s, b);
            reject("a tRNS on an image that already has alpha",
                   b, n, PNG_E_CORRUPT);
        }
        {   /* tRNS of the wrong length for a greyscale image */
            static const unsigned char tr[] = { 0, 1, 2 };
            static const unsigned char r2[] = { 0, 5 };
            struct spec s = { .w=1, .h=1, .depth=8, .colour=0,
                              .trns=tr, .ntrns=sizeof tr,
                              .raw=r2, .nraw=sizeof r2 };
            int n = build(&s, b);
            reject("a three byte tRNS on a grey image", b, n, PNG_E_CORRUPT);
        }
        {   /* more tRNS entries than the palette has */
            static const unsigned char pal[] = { 1,2,3, 4,5,6 };
            static const unsigned char tr[] = { 0, 1, 2, 3 };
            static const unsigned char r2[] = { 0, 0, 1 };
            struct spec s = { .w=2, .h=1, .depth=8, .colour=3,
                              .plte=pal, .nplte=sizeof pal,
                              .trns=tr, .ntrns=sizeof tr,
                              .raw=r2, .nraw=sizeof r2 };
            int n = build(&s, b);
            reject("a tRNS longer than its palette", b, n, PNG_E_CORRUPT);
        }
    }

    printf("\n11. the pixels contradict the header\n");
    png_reset();
    {
        static unsigned char b[4096];
        {   /* an index nothing sent a colour for */
            static const unsigned char pal[] = { 1,2,3, 4,5,6 };
            static const unsigned char r2[] = { 0, 0, 5 };
            struct spec s = { .w=2, .h=1, .depth=8, .colour=3,
                              .plte=pal, .nplte=sizeof pal,
                              .raw=r2, .nraw=sizeof r2 };
            int n = build(&s, b);
            reject("a palette index past the end of PLTE", b, n, PNG_E_CORRUPT);
        }
        {   /* and the same at four bits, where the index is half a byte */
            static const unsigned char pal[] = { 1,2,3, 4,5,6 };
            static const unsigned char r2[] = { 0, 0x0F };
            struct spec s = { .w=2, .h=1, .depth=4, .colour=3,
                              .plte=pal, .nplte=sizeof pal,
                              .raw=r2, .nraw=sizeof r2 };
            int n = build(&s, b);
            reject("a 4-bit palette index past the end of PLTE",
                   b, n, PNG_E_CORRUPT);
        }
        {
            static const unsigned char r2[] = { 5, 1, 2, 3 };
            struct spec s = { .w=3, .h=1, .depth=8, .colour=0,
                              .raw=r2, .nraw=sizeof r2 };
            int n = build(&s, b);
            reject("a filter byte of 5", b, n, PNG_E_CORRUPT);
        }
        {
            static const unsigned char r2[] = { 200, 1, 2, 3 };
            struct spec s = { .w=3, .h=1, .depth=8, .colour=0,
                              .raw=r2, .nraw=sizeof r2 };
            int n = build(&s, b);
            reject("a filter byte of 200", b, n, PNG_E_CORRUPT);
        }
        {   /* a good first row and a bad second one - the failure has to
             * happen even after some pixels are already in the arena */
            static const unsigned char r2[] = { 0, 1, 2, 3, 5, 4, 5, 6 };
            struct spec s = { .w=3, .h=2, .depth=8, .colour=0,
                              .raw=r2, .nraw=sizeof r2 };
            int n = build(&s, b);
            int before = png_arena_used(), slots = png_slots_used();
            reject("a bad filter byte on the second row", b, n, PNG_E_CORRUPT);
            ok("...and the failed decode took no slot and no arena",
               png_arena_used() == before && png_slots_used() == slots);
        }
        {   /* one byte short of a whole image */
            static const unsigned char r2[] = { 0, 1, 2 };
            struct spec s = { .w=3, .h=1, .depth=8, .colour=0,
                              .raw=r2, .nraw=sizeof r2 };
            int n = build(&s, b);
            reject("a stream that inflates to fewer bytes than the image needs",
                   b, n, PNG_E_TRUNCATED);
        }
        {   /* one byte more */
            static const unsigned char r2[] = { 0, 1, 2, 3, 4 };
            struct spec s = { .w=3, .h=1, .depth=8, .colour=0,
                              .raw=r2, .nraw=sizeof r2 };
            int n = build(&s, b);
            reject("a stream that inflates to more bytes than the image needs",
                   b, n, PNG_E_CORRUPT);
        }
        {   /* an interlaced image missing its last pass */
            static const unsigned char r2[] = { 0, 1 };
            struct spec s = { .w=4, .h=4, .depth=8, .colour=0, .interlace=1,
                              .raw=r2, .nraw=sizeof r2 };
            int n = build(&s, b);
            reject("an interlaced image with only one pass of data",
                   b, n, PNG_E_TRUNCATED);
        }
    }

    printf("\n12. the zlib and deflate layers\n");
    png_reset();
    {
        static unsigned char b[4096];
        static const unsigned char raw[] = { 0, 1, 2, 3 };
        struct spec s = { .w=3, .h=1, .depth=8, .colour=0,
                          .raw=raw, .nraw=sizeof raw };
        int n, c;

        /* Each of these patches the two zlib header bytes and then fixes the
         * IDAT CRC, so nothing but the header is wrong. FCHECK is chosen so
         * (CMF*256+FLG) stays divisible by 31 except where that is the point. */
        #define ZHDR(cmf, flg, what, want) do {                      \
            n = build(&s, b); c = find(b, n, "IDAT", 0);             \
            b[c + 8] = (unsigned char)(cmf);                         \
            b[c + 9] = (unsigned char)(flg);                         \
            refix(b, c);                                             \
            reject(what, b, n, want);                                \
        } while (0)

        ZHDR(0x78, 0x20, "a zlib stream claiming a preset dictionary",
             PNG_E_UNSUPPORTED);
        ZHDR(0x79, 24, "a compression method that is not deflate",
             PNG_E_CORRUPT);
        ZHDR(0x88, 28, "a window size larger than 32K", PNG_E_CORRUPT);
        ZHDR(0x78, 0x02, "a zlib header whose check bits do not divide by 31",
             PNG_E_CORRUPT);
        #undef ZHDR

        {   /* the Adler-32 is the only thing that notices data which
             * decompressed cleanly and is still wrong */
            n = build(&s, b);
            c = find(b, n, "IDAT", 0);
            b[c + 8 + (int)rd32(b + c) - 1] ^= 0x01;
            refix(b, c);
            reject("an Adler-32 that does not match the data", b, n,
                   PNG_E_CORRUPT);
        }
        {   /* block type 3 is reserved and means the stream is not deflate */
            struct spec t = { .w=3, .h=1, .depth=8, .colour=0,
                              .method=M_PREBUILT };
            zstart();
            wbits(1, 1); wbits(3, 2);
            walign();
            wbyte(0); wbyte(0); wbyte(0); wbyte(0);
            n = build(&t, b);
            reject("a deflate block of the reserved type 3", b, n,
                   PNG_E_CORRUPT);
        }
        {   /* a stored block whose NLEN is not the complement of LEN */
            struct spec t = { .w=3, .h=1, .depth=8, .colour=0,
                              .method=M_PREBUILT };
            zstart();
            wbits(1, 1); wbits(0, 2);
            walign();
            wbyte(4); wbyte(0);
            wbyte(0xFF); wbyte(0xFF);           /* should be 0xFB 0xFF */
            wbyte(0); wbyte(1); wbyte(2); wbyte(3);
            zfinish(t_adler(raw, 4));
            n = build(&t, b);
            reject("a stored block whose LEN and NLEN disagree", b, n,
                   PNG_E_CORRUPT);
        }
        {   /* an over-subscribed code-length code: four one-bit codes in a
             * space that holds two */
            struct spec t = { .w=3, .h=1, .depth=8, .colour=0,
                              .method=M_PREBUILT };
            zstart();
            wbits(1, 1); wbits(2, 2);
            wbits(0, 5); wbits(0, 5); wbits(0, 4);   /* HCLEN -> 4 */
            for (int i = 0; i < 4; i++) wbits(1, 3);
            walign();
            n = build(&t, b);
            reject("a dynamic block whose code-length code is over-subscribed",
                   b, n, PNG_E_CORRUPT);
        }
        {   /* "repeat the previous length" as the very first length */
            struct spec t = { .w=3, .h=1, .depth=8, .colour=0,
                              .method=M_PREBUILT };
            static const int cl[18] = { 1,0,0, 2, 0,0,0,0,0,0,0,0,0,0,0,0,0, 2 };
            zstart();
            wbits(1, 1); wbits(2, 2);
            wbits(0, 5); wbits(0, 5); wbits(14, 4);
            for (int i = 0; i < 18; i++) wbits((unsigned)cl[i], 3);
            wcode(0, 1);                     /* sym16, with nothing before it */
            wbits(0, 2);
            walign();
            n = build(&t, b);
            reject("a dynamic block that repeats a length that does not exist",
                   b, n, PNG_E_CORRUPT);
        }
        {   /* a zero run that walks off the end of the length table */
            struct spec t = { .w=3, .h=1, .depth=8, .colour=0,
                              .method=M_PREBUILT };
            static const int cl[18] = { 0,0,1, 2, 0,0,0,0,0,0,0,0,0,0,0,0,0, 2 };
            zstart();
            wbits(1, 1); wbits(2, 2);
            wbits(0, 5); wbits(0, 5); wbits(14, 4);
            for (int i = 0; i < 18; i++) wbits((unsigned)cl[i], 3);
            wcode(0, 1); wbits(127, 7);      /* sym18 -> 138 zeros */
            wcode(0, 1); wbits(127, 7);      /* ...and 138 more, past 258 */
            walign();
            n = build(&t, b);
            reject("a dynamic block whose zero run runs past the table",
                   b, n, PNG_E_CORRUPT);
        }
        {   /* every length zero, so there is no end-of-block symbol */
            struct spec t = { .w=3, .h=1, .depth=8, .colour=0,
                              .method=M_PREBUILT };
            static const int cl[18] = { 0,0,1, 2, 0,0,0,0,0,0,0,0,0,0,0,0,0, 2 };
            zstart();
            wbits(1, 1); wbits(2, 2);
            wbits(0, 5); wbits(0, 5); wbits(14, 4);
            for (int i = 0; i < 18; i++) wbits((unsigned)cl[i], 3);
            wcode(0, 1); wbits(127, 7);      /* 138 zeros */
            wcode(0, 1); wbits(109, 7);      /* 120 more -> exactly 258 */
            walign();
            n = build(&t, b);
            reject("a dynamic block with no end-of-block code", b, n,
                   PNG_E_CORRUPT);
        }
        {   /* A MATCH REACHING BACK FURTHER THAN ANYTHING PRODUCED, and its
             * twin, because on its own this test proves nothing.
             *
             * A decoder with no distance check does not fault here - it copies
             * whatever is sitting in the sliding window from an earlier image,
             * and then the Adler-32 rejects the result. Same verdict, same
             * reason code, so the test passes either way and the missing check
             * is invisible. (Confirmed: deleting the check from png.c leaves
             * all 126 other assertions green.)
             *
             * So the streams below are a matched pair. Both inflate to four
             * zero bytes and both carry the Adler-32 of four zero bytes; the
             * ONLY difference is the distance symbol. The legal one must be
             * accepted, which proves everything around the distance is right,
             * and then the illegal one must be refused. */
            static const unsigned char zero4[4] = { 0, 0, 0, 0 };
            static const unsigned int black[] = { G(255,0), G(255,0), G(255,0) };
            struct spec t = { .w=3, .h=1, .depth=8, .colour=0,
                              .method=M_PREBUILT };
            zstart();
            wbits(1, 1); wbits(1, 2);
            wcode(0x30u + 0, 8);             /* one literal, so out_total is 1 */
            wcode(1, 7);                     /* symbol 257 -> length 3 */
            wcode(0, 5);                     /* distance symbol 0 -> 1, legal */
            wcode(0, 7);
            zfinish(t_adler(zero4, 4));
            accept("...the same stream with a legal distance of 1", &t, black);

            zstart();
            wbits(1, 1); wbits(1, 2);
            wcode(0x30u + 0, 8);
            wcode(1, 7);                     /* length 3 */
            wcode(2, 5);                     /* distance symbol 2 -> 3 */
            wcode(0, 7);
            zfinish(t_adler(zero4, 4));
            n = build(&t, b);
            reject("a match whose distance points before the start of the stream",
                   b, n, PNG_E_CORRUPT);
        }
        {   /* the two distance codes the fixed table defines and the format
             * gives no meaning to */
            struct spec t = { .w=3, .h=1, .depth=8, .colour=0,
                              .method=M_PREBUILT };
            zstart();
            wbits(1, 1); wbits(1, 2);
            wcode(0x30u + 0, 8);
            wcode(0x30u + 1, 8);
            wcode(0x30u + 2, 8);
            wcode(0x30u + 3, 8);
            wcode(5, 7);                     /* length 7 */
            wcode(30, 5);                    /* distance symbol 30 */
            wcode(0, 7);
            walign();
            wbyte(0); wbyte(0); wbyte(0); wbyte(0);
            n = build(&t, b);
            reject("a distance symbol of 30, which has no distance",
                   b, n, PNG_E_CORRUPT);
        }
        {   /* An IDAT three bytes shorter than the stream it carries: the
             * chunk is well formed, its CRC is right, the file walks to IEND,
             * and the deflate stream inside it simply stops. Nothing but the
             * inflater can notice this one. */
            int cl;
            n = build(&s, b);
            c = find(b, n, "IDAT", 0);
            cl = (int)rd32(b + c);
            be(b + c, (unsigned)(cl - 3));
            for (int k = c + 8 + cl - 3; k < n - 3; k++) b[k] = b[k + 3];
            refix(b, c);
            reject("an IDAT cut off inside the deflate stream",
                   b, n - 3, PNG_E_TRUNCATED);
        }
    }

    printf("\n13. the arena and the slot table filling up\n");
    {
        static const unsigned char raw[] = { 0, 9 };
        static const unsigned int want[] = { G(255,9) };
        struct spec s = { .w=1, .h=1, .depth=8, .colour=0,
                          .raw=raw, .nraw=sizeof raw };
        static unsigned char b[512];
        int n = build(&s, b);
        int i, taken = 0;

        png_set_arena(px, PNG_ARENA_PX);
        for (i = 0; i < PNG_MAX_SLOTS; i++)
            if (png_decode(b, n) >= 0) taken++;
        ok("PNG_MAX_SLOTS images fit", taken == PNG_MAX_SLOTS);
        reject("one more image than there are slots", b, n, PNG_E_NO_ROOM);

        /* and the pixel budget, independently of the slot count */
        png_set_arena(px, 8);
        ok("a fresh arena has nothing in it",
           png_slots_used() == 0 && png_arena_used() == 0);
        {
            static const unsigned char big[] = { 0, 1,2,3, 4,5,6,
                                                 0, 7,8,9, 10,11,12 };
            struct spec t = { .w=2, .h=2, .depth=8, .colour=2,
                              .raw=big, .nraw=sizeof big };
            static unsigned char c2[512];
            int m = build(&t, c2);
            ok("two 2x2 images fit in eight pixels",
               png_decode(c2, m) >= 0 && png_decode(c2, m) >= 0);
            reject("a third does not", c2, m, PNG_E_NO_ROOM);
        }
        png_set_arena(0, 0);
        reject("no arena at all is PNG_E_NO_ROOM, loudly", b, n, PNG_E_NO_ROOM);

        png_set_arena(px, PNG_ARENA_PX);
        accept("...and a real arena works again immediately", &s, want);
        ok("png_why is PNG_OK after a decode that succeeded",
           png_why() == PNG_OK);
    }

    printf("\n14. a bigger image, so the row machinery runs more than twice\n");
    png_reset();
    {
        static unsigned char raw[64 * (1 + 64 * 4)];
        static unsigned int want[64 * 64];
        int n = 0;
        for (int y = 0; y < 64; y++) {
            raw[n++] = 0;
            for (int x = 0; x < 64; x++) {
                unsigned char r = (unsigned char)(x * 4);
                unsigned char g = (unsigned char)(y * 4);
                unsigned char bl = (unsigned char)((x ^ y) * 3);
                unsigned char a = (unsigned char)(255 - x);
                raw[n++] = r; raw[n++] = g; raw[n++] = bl; raw[n++] = a;
                want[y * 64 + x] = P(a, r, g, bl);
            }
        }
        {
            struct spec s = { .w=64, .h=64, .depth=8, .colour=6,
                              .raw=raw, .nraw=n, .method=M_DYNAMIC };
            accept("64x64 RGBA through dynamic Huffman", &s, want);
            s.method = M_STORED; s.nblocks = 3; s.nidat = 5;
            accept("...and through three stored blocks over five IDATs",
                   &s, want);
        }
    }

    printf("\n%d checks, %d failed\n", passed + failed, failed);
    return failed ? 1 : 0;
}
