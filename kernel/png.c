/* png.c - PNG, and the DEFLATE underneath it, for a kernel with no allocator.
 *
 * A PNG COMES OFF THE NETWORK FROM A MACHINE WE DID NOT CHOOSE. It is the same
 * trust boundary dns.c and css.c sit on, with one difference that makes it
 * worse: a DNS answer is a few hundred bytes of length-prefixed labels, and an
 * image is a compressor. Every length in here expands. The classic holes in
 * this format are all the same shape - a chunk length added to an offset and
 * wrapping, a palette index used without checking it against the palette that
 * was actually sent, a filter reading bpp bytes to the left of a scanline it
 * is not allowed to leave, a match distance pointing further back than
 * anything has been produced. All four are checked here by arithmetic that
 * cannot wrap rather than by a comparison that could.
 *
 * NOTHING IS DECOMPRESSED INTO A BUFFER. The obvious shape - inflate the IDATs
 * into one array, then walk it - needs (1 + 8*width) * height bytes, which for
 * this decoder's own ceiling is 8.4 MiB, and the whole kernel has 438 KiB of
 * BSS left (png.h explains that measurement). So inflate runs as a RESUMABLE
 * state machine that produces bytes into the 32 KiB sliding window it already
 * has to keep, and the scanline reader drains that window one row at a time.
 * The peak cost is the window plus two scanlines, whatever the image size:
 *
 *      win[32768] + two rows of 8192 = 48 KiB, fixed, for any PNG we accept
 *
 * That is also why the state machine has states at all. A stored block may be
 * 65535 bytes, which is twice the window, so "copy the block" cannot be one
 * loop - it has to be able to stop in the middle, hand rows to the caller, and
 * pick up where it was.
 *
 * THE IDATs ARE ONE ZLIB STREAM, NOT SEVERAL. Every encoder splits the
 * compressed data across IDAT chunks, and where it splits is that encoder's
 * business - libpng uses 8192-byte chunks, others use one chunk, others cut on
 * a row boundary. A decoder that inflates each IDAT independently works
 * perfectly against whichever encoder its author tested with and fails on
 * everything else, and the failure looks like a corrupt file rather than like
 * a bug. So the bit reader walks a list of IDAT spans and treats their
 * concatenation as the stream; a zlib header appears exactly once, at the
 * start of the first one.
 *
 * THE ADLER-32 AND EVERY CRC-32 ARE CHECKED. They cost one pass over bytes we
 * are reading anyway and they are the only thing in the format that can tell a
 * decoder it has been handed rubbish before it draws it.
 *
 * FREESTANDING. No libc, no heap, no floating point - the four byte helpers
 * below are the whole of what would otherwise be <string.h>, written here the
 * way crypto.c writes cmemcpy and for the same reason.
 *
 * Everything in this file is asserted by hosttest/pngtest.c, which builds its
 * PNGs byte by byte - including its own three deflate encoders - so that every
 * input is exact and there is no fixture file to go stale.
 */

#include "png.h"

typedef unsigned char u8;
typedef unsigned int  u32;      /* 32 bits on both the i386 and the x86-64
                                   build; `long` is never used, because it is
                                   4 bytes on one of them and 8 on the other */

/* ------------------------------------------------------------------ bytes */

static void pzero(u8 *d, int n)
{
    while (n-- > 0) *d++ = 0;
}

static void pcopy(u8 *d, const u8 *s, int n)
{
    while (n-- > 0) *d++ = *s++;
}

static u32 be32(const u8 *p)
{
    return ((u32)p[0] << 24) | ((u32)p[1] << 16) | ((u32)p[2] << 8) | (u32)p[3];
}

static u32 be16(const u8 *p)
{
    return ((u32)p[0] << 8) | (u32)p[1];
}

/* ------------------------------------------------------------------ CRC-32
 * The CRC every chunk carries, over the type field and the data. A corrupt
 * chunk has to be REPORTED rather than decoded: a flipped bit in IHDR is a
 * width of 2^31, and a flipped bit in a filter byte is a whole page of noise.
 * The table is built once instead of being a 1 KiB const array, because .bss
 * is NOBITS and .rodata is not - this way it costs nothing in the disk image. */

static u32 crc_tab[256];
static int crc_built;

static void crc_build(void)
{
    for (int i = 0; i < 256; i++) {
        u32 c = (u32)i;
        for (int k = 0; k < 8; k++)
            c = (c & 1u) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
        crc_tab[i] = c;
    }
    crc_built = 1;
}

static u32 crc32_of(const u8 *p, int n)
{
    u32 c = 0xFFFFFFFFu;
    for (int i = 0; i < n; i++)
        c = crc_tab[(c ^ p[i]) & 0xFFu] ^ (c >> 8);
    return c ^ 0xFFFFFFFFu;
}

/* ------------------------------------------------------- the decode's state
 * ONE DECODE AT A TIME. png_decode is not reentrant and does not need to be -
 * browser.c decodes images one after another while laying a page out, and a
 * second caller would want a second 48 KiB of window and scanlines. Everything
 * below is reset at the top of png_decode. */

static int err;                 /* PNG_E_*, or PNG_OK while nothing is wrong */

static const u8 *pngbuf;        /* the whole file, for the IDAT spans        */
static int img_w, img_h;
static int img_depth, img_colour, img_interlace;
static int img_chans, img_bitspp, img_bpp;

static u8  plte[768];
static int nplte;                       /* entries, not bytes */
static u8  palpha[256];                 /* tRNS for a palette; 255 by default */
static int trns_ok;                     /* tRNS for colour type 0 or 2        */
static u32 trns_v[3];                   /* at the image's own bit depth       */

/* ------------------------------------------------------------- the arena */

static unsigned int *arena;
static int arena_px;
static int arena_used;
static struct { int w, h, off; } slot[PNG_MAX_SLOTS];
static int nslot;

void png_set_arena(unsigned int *px, int npx)
{
    arena = px;
    arena_px = (px && npx > 0) ? npx : 0;
    arena_used = 0;
    nslot = 0;
}

void png_reset(void)          { arena_used = 0; nslot = 0; }
int  png_why(void)            { return err; }
int  png_slots_used(void)     { return nslot; }
int  png_arena_used(void)     { return arena_used; }
int  png_w(int s)             { return (s >= 0 && s < nslot) ? slot[s].w : 0; }
int  png_h(int s)             { return (s >= 0 && s < nslot) ? slot[s].h : 0; }

const unsigned int *png_pixels(int s)
{
    if (s < 0 || s >= nslot || !arena) return 0;
    return arena + slot[s].off;
}

/* ------------------------------------------------------- the IDAT bit source
 * A file large enough to need more spans than this is larger than anything
 * that fits in the arena, so the ceiling refuses work it could not have
 * finished anyway. 1024 spans is 8 MiB of compressed data at libpng's 8 KiB
 * default IDAT size. */
#define MAX_IDAT 1024

static struct { int off, len; } idat[MAX_IDAT];
static int nidat;
static int zseg, zoff;          /* which span, and how far into it */
static u32 bitbuf;
static int bitcnt;

static int zbyte(void)
{
    while (zseg < nidat) {
        if (zoff < idat[zseg].len) return pngbuf[idat[zseg].off + zoff++];
        zseg++;
        zoff = 0;
    }
    return -1;
}

/* n is at most 16 (a stored block's LEN), so bitcnt never exceeds 23 and the
 * accumulator never overflows. Returns -1 and sets err when the input runs
 * out, which is a value bits() can never legitimately produce. */
static int bits(int n)
{
    while (bitcnt < n) {
        int c = zbyte();
        if (c < 0) { err = PNG_E_TRUNCATED; return -1; }
        bitbuf |= (u32)c << bitcnt;
        bitcnt += 8;
    }
    int v = (int)(bitbuf & ((1u << n) - 1u));
    bitbuf >>= n;
    bitcnt -= n;
    return v;
}

/* Discard whatever is left of the current byte. Only whole bytes are ever
 * loaded, so the number of bits still owed to the boundary is bitcnt & 7. */
static void bit_align(void)
{
    int drop = bitcnt & 7;
    bitbuf >>= drop;
    bitcnt -= drop;
}

/* ---------------------------------------------------------- Huffman codes
 * Canonical decoding by counting codes per length rather than by building a
 * lookup table: it needs 620 bytes per code instead of a 32 KiB table, and the
 * images here are small enough that the per-bit loop is not what costs. */

#define MAX_SYMS 288

struct huff {
    short cnt[16];
    short sym[MAX_SYMS];
};

static struct huff lencode, distcode;

/* Returns 0 for a complete code, > 0 for an incomplete one (the number of
 * unused codes), -1 for an over-subscribed one, -2 for one with no codes at
 * all. Which of those the caller tolerates differs between the two tables, so
 * this reports rather than decides. */
static int huff_build(struct huff *h, const u8 *len, int n)
{
    short offs[16];
    int i, left;

    for (i = 0; i < 16; i++) h->cnt[i] = 0;
    for (i = 0; i < n; i++) h->cnt[len[i]]++;
    if (h->cnt[0] == n) return -2;

    left = 1;
    for (i = 1; i < 16; i++) {
        left <<= 1;
        left -= h->cnt[i];
        if (left < 0) return -1;
    }
    offs[1] = 0;
    for (i = 1; i < 15; i++) offs[i + 1] = (short)(offs[i] + h->cnt[i]);
    for (i = 0; i < n; i++)
        if (len[i]) h->sym[offs[len[i]]++] = (short)i;
    return left;
}

static int huff_decode(const struct huff *h)
{
    int code = 0, first = 0, index = 0;

    for (int len = 1; len < 16; len++) {
        int b = bits(1);
        if (b < 0) return -1;
        code |= b;
        int count = h->cnt[len];
        if (code - count < first) {
            int s = index + (code - first);
            /* huff_build refuses an over-subscribed code, so s is always in
             * range for a table it built - which is exactly why this check is
             * here rather than trusted away. It is one compare against a
             * stream somebody else wrote. */
            if (s < 0 || s >= MAX_SYMS) { err = PNG_E_CORRUPT; return -1; }
            return h->sym[s];
        }
        index += count;
        first += count;
        first <<= 1;
        code <<= 1;
    }
    err = PNG_E_CORRUPT;                /* 16 bits and no code matched */
    return -1;
}

/* ------------------------------------------------------------------ inflate */

static const unsigned short LBASE[29] = {
    3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51,
    59, 67, 83, 99, 115, 131, 163, 195, 227, 258
};
static const u8 LEXT[29] = {
    0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0
};
static const unsigned short DBASE[30] = {
    1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385,
    513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577
};
static const u8 DEXT[30] = {
    0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13
};

#define WINSZ 32768
#define WINMASK (WINSZ - 1)

static u8  win[WINSZ];
static u32 out_total;           /* bytes the inflater has produced */
static u32 out_drained;         /* bytes the scanline reader has taken */
static u32 out_limit;           /* exactly how many the image needs */
static u32 ad_a, ad_b;

#define ZS_HDR    0
#define ZS_BLOCK  1
#define ZS_STORED 2
#define ZS_CODES  3
#define ZS_TAIL   4
#define ZS_END    5

static int zstate, bfinal;
static u32 stored_left;

/* One byte out. The limit is not a safety net, it is the test for "this stream
 * inflates to MORE than the image needs", which is a corrupt file and not a
 * harmless surplus - the extra bytes would be a second image's worth of rows
 * nobody asked for. */
static int emit(int b)
{
    if (out_total >= out_limit) { err = PNG_E_CORRUPT; return -1; }
    win[out_total & WINMASK] = (u8)b;
    out_total++;
    ad_a += (u32)(b & 0xFF); if (ad_a >= 65521u) ad_a -= 65521u;
    ad_b += ad_a;            if (ad_b >= 65521u) ad_b -= 65521u;
    return 0;
}

static void fixed_tables(void)
{
    u8 l[MAX_SYMS];
    int i;

    for (i = 0;   i < 144; i++) l[i] = 8;
    for (;        i < 256; i++) l[i] = 9;
    for (;        i < 280; i++) l[i] = 7;
    for (;        i < 288; i++) l[i] = 8;
    huff_build(&lencode, l, 288);

    /* 32 distance codes of 5 bits, not 30: the format defines the code over 32
     * symbols and leaves 30 and 31 unused, so building it over 30 would make
     * an incomplete code out of a complete one. The two dead symbols are
     * refused where they are decoded instead. */
    for (i = 0; i < 32; i++) l[i] = 5;
    huff_build(&distcode, l, 32);
}

static int dynamic_tables(void)
{
    static const u8 ord[19] = {
        16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
    };
    u8 l[MAX_SYMS + 32];
    int nlen, ndist, ncode, i, v;

    v = bits(5); if (v < 0) return -1; nlen  = v + 257;
    v = bits(5); if (v < 0) return -1; ndist = v + 1;
    v = bits(4); if (v < 0) return -1; ncode = v + 4;
    /* 286 and 30 are the largest counts the format assigns a meaning to. The
     * five-bit fields can say 288 and 32, and a stream that does is claiming
     * symbols with no length or distance table entry. */
    if (nlen > 286 || ndist > 30) { err = PNG_E_CORRUPT; return -1; }

    for (i = 0; i < 19; i++) l[i] = 0;
    for (i = 0; i < ncode; i++) {
        v = bits(3); if (v < 0) return -1;
        l[ord[i]] = (u8)v;
    }
    if (huff_build(&lencode, l, 19) != 0) { err = PNG_E_CORRUPT; return -1; }

    i = 0;
    while (i < nlen + ndist) {
        int sym = huff_decode(&lencode);
        int rep, val = 0;
        if (sym < 0) { if (!err) err = PNG_E_CORRUPT; return -1; }
        if (sym < 16) { l[i++] = (u8)sym; continue; }
        if (sym == 16) {
            /* "repeat the previous length" with nothing before it */
            if (i == 0) { err = PNG_E_CORRUPT; return -1; }
            val = l[i - 1];
            v = bits(2); if (v < 0) return -1;
            rep = 3 + v;
        } else if (sym == 17) {
            v = bits(3); if (v < 0) return -1;
            rep = 3 + v;
        } else {
            v = bits(7); if (v < 0) return -1;
            rep = 11 + v;
        }
        if (i + rep > nlen + ndist) { err = PNG_E_CORRUPT; return -1; }
        while (rep--) l[i++] = (u8)val;
    }
    if (l[256] == 0) { err = PNG_E_CORRUPT; return -1; }  /* no end-of-block */

    if (huff_build(&lencode, l, nlen) != 0) { err = PNG_E_CORRUPT; return -1; }
    {
        int r = huff_build(&distcode, l + nlen, ndist);
        if (r == -1) { err = PNG_E_CORRUPT; return -1; }
        /* An INCOMPLETE distance code is legal and real encoders emit one: a
         * block with no matches in it still has to send a distance table, and
         * zlib sends a single one-bit code. More than one used symbol with
         * codes left over is a table that does not add up, and a decoder that
         * accepted it would resolve some bit patterns to a stale symbol. */
        if (r > 0) {
            int used = 0;
            for (i = 0; i < ndist; i++) if (l[nlen + i]) used++;
            if (used > 1) { err = PNG_E_CORRUPT; return -1; }
        }
    }
    return 0;
}

/* One step of the inflater. `need` is how many bytes the scanline reader is
 * waiting for; a stored block copies at most that many so the window can never
 * run over data the reader has not taken yet. */
static void inf_step(u32 need)
{
    switch (zstate) {

    case ZS_HDR: {
        int cmf = zbyte();
        int flg;
        if (cmf < 0) { err = PNG_E_TRUNCATED; return; }
        flg = zbyte();
        if (flg < 0) { err = PNG_E_TRUNCATED; return; }
        if ((cmf & 15) != 8)  { err = PNG_E_CORRUPT; return; }  /* not deflate */
        if ((cmf >> 4) > 7)   { err = PNG_E_CORRUPT; return; }  /* window > 32K */
        if (((cmf << 8) + flg) % 31) { err = PNG_E_CORRUPT; return; }
        /* A preset dictionary means the first 32 KiB of history came from
         * somewhere this file does not contain. PNG forbids it outright, so it
         * is never a file we merely cannot read - but it is the one zlib
         * feature a decoder can meaningfully be said not to carry. */
        if (flg & 0x20) { err = PNG_E_UNSUPPORTED; return; }
        zstate = ZS_BLOCK;
        return;
    }

    case ZS_BLOCK: {
        int f = bits(1); if (f < 0) return;
        int t = bits(2); if (t < 0) return;
        bfinal = f;
        if (t == 0) {
            int lo, nl;
            bit_align();
            lo = bits(16); if (lo < 0) return;
            nl = bits(16); if (nl < 0) return;
            if ((lo ^ 0xFFFF) != nl) { err = PNG_E_CORRUPT; return; }
            stored_left = (u32)lo;
            zstate = ZS_STORED;
        } else if (t == 1) {
            fixed_tables();
            zstate = ZS_CODES;
        } else if (t == 2) {
            if (dynamic_tables() < 0) return;
            zstate = ZS_CODES;
        } else {
            err = PNG_E_CORRUPT;               /* block type 3 is reserved */
        }
        return;
    }

    case ZS_STORED: {
        /* ROOM MUST NEVER BE ZERO. Written as need - (out_total - out_drained)
         * it is zero exactly when those two are equal, and then this case
         * copies nothing, changes no state and returns - so a caller looping
         * on "not finished yet" spins forever. inf_fill's guard makes that
         * unreachable and the flush after the last scanline happens to make it
         * unreachable too, but both of those are invariants held in ANOTHER
         * function, and the failure mode here is a kernel that stops rather
         * than an image that looks wrong. Found by deliberately breaking the
         * pass loop and watching this hang instead of fail. */
        u32 have = out_total - out_drained;
        u32 room = (need > have) ? need - have : 1;
        while (stored_left && room) {
            int c = zbyte();
            if (c < 0) { err = PNG_E_TRUNCATED; return; }
            if (emit(c)) return;
            stored_left--;
            room--;
        }
        if (!stored_left) zstate = bfinal ? ZS_TAIL : ZS_BLOCK;
        return;
    }

    case ZS_CODES: {
        int sym = huff_decode(&lencode);
        int ds, e;
        u32 length, dist;

        if (sym < 0) { if (!err) err = PNG_E_CORRUPT; return; }
        if (sym < 256) { emit(sym); return; }
        if (sym == 256) { zstate = bfinal ? ZS_TAIL : ZS_BLOCK; return; }

        sym -= 257;
        if (sym >= 29) { err = PNG_E_CORRUPT; return; }   /* 286, 287 */
        e = bits(LEXT[sym]); if (e < 0) return;
        length = (u32)LBASE[sym] + (u32)e;

        ds = huff_decode(&distcode);
        if (ds < 0) { if (!err) err = PNG_E_CORRUPT; return; }
        if (ds >= 30) { err = PNG_E_CORRUPT; return; }    /* the two dead codes */
        e = bits(DEXT[ds]); if (e < 0) return;
        dist = (u32)DBASE[ds] + (u32)e;

        /* "distance too far back" - the copy source has to be inside what has
         * actually been produced. Without this a first-block match reads
         * whatever the previous image left in the window, which decodes
         * silently and shows one page's pixels inside another's. */
        if (dist > out_total) { err = PNG_E_CORRUPT; return; }

        /* At most 258 bytes, and the caller's `need` is at most one scanline,
         * so this cannot overrun the window even when it runs past `need`. The
         * read-then-write order is what makes an overlapping copy correct. */
        while (length--) {
            u8 b = win[(out_total - dist) & WINMASK];
            if (emit(b)) return;
        }
        return;
    }

    case ZS_TAIL: {
        u32 want = 0;
        bit_align();
        for (int i = 0; i < 4; i++) {
            int c = bits(8);
            if (c < 0) return;
            want = (want << 8) | (u32)c;
        }
        if (want != ((ad_b << 16) | ad_a)) { err = PNG_E_CORRUPT; return; }
        zstate = ZS_END;
        return;
    }

    default:
        return;
    }
}

/* Every state either consumes input or advances, and the input is finite, so
 * this terminates on a hostile stream as well as on a good one. */
static void inf_fill(u32 need)
{
    while (!err && zstate != ZS_END && (out_total - out_drained) < need)
        inf_step(need);
}

static int need_bytes(u32 n)
{
    inf_fill(n);
    if (err) return -1;
    if ((out_total - out_drained) < n) { err = PNG_E_TRUNCATED; return -1; }
    return 0;
}

static u8 take(void)
{
    return win[out_drained++ & WINMASK];
}

/* ------------------------------------------------------------- the filters
 * These run on BYTES, and `bpp` is max(1, bits_per_pixel / 8) - not the pixel
 * count and not the sample count. At bit depths under 8 a "pixel" is smaller
 * than the unit the filter steps by, which is why bpp is 1 there and why a
 * 4-bit image is the case that catches an off-by-one nobody sees at 8 bits. */

static int paeth(int a, int b, int c)
{
    int p = a + b - c;
    int pa = p - a, pb = p - b, pc = p - c;
    if (pa < 0) pa = -pa;
    if (pb < 0) pb = -pb;
    if (pc < 0) pc = -pc;
    if (pa <= pb && pa <= pc) return a;
    if (pb <= pc) return b;
    return c;
}

static int unfilter(int f, u8 *cur, const u8 *prev, int nb, int bpp)
{
    int i;
    switch (f) {
    case 0:
        break;
    case 1:
        for (i = bpp; i < nb; i++) cur[i] = (u8)(cur[i] + cur[i - bpp]);
        break;
    case 2:
        for (i = 0; i < nb; i++) cur[i] = (u8)(cur[i] + prev[i]);
        break;
    case 3:
        for (i = 0; i < bpp && i < nb; i++) cur[i] = (u8)(cur[i] + (prev[i] >> 1));
        for (; i < nb; i++)
            cur[i] = (u8)(cur[i] + ((cur[i - bpp] + prev[i]) >> 1));
        break;
    case 4:
        for (i = 0; i < bpp && i < nb; i++) cur[i] = (u8)(cur[i] + prev[i]);
        for (; i < nb; i++)
            cur[i] = (u8)(cur[i] + paeth(cur[i - bpp], prev[i], prev[i - bpp]));
        break;
    default:
        err = PNG_E_CORRUPT;                 /* there is no filter 5 */
        return -1;
    }
    return 0;
}

/* --------------------------------------------------------------- samples */

static u32 sample(const u8 *row, int idx, int depth)
{
    if (depth == 8)  return row[idx];
    if (depth == 16) return ((u32)row[idx * 2] << 8) | row[idx * 2 + 1];
    {
        int bit = idx * depth;
        int sh  = 8 - depth - (bit & 7);
        return (u32)((row[bit >> 3] >> sh) & ((1 << depth) - 1));
    }
}

/* Scale a sample of `depth` bits to 8. The multipliers are exact - 1 bit maps
 * to 0/255, 2 bits to 0/85/170/255, 4 bits to steps of 17 - which is what the
 * specification's "left bit replication" comes out to, without a divide. */
static u32 to8(u32 v, int depth)
{
    if (depth == 16) return v >> 8;
    if (depth == 8)  return v;
    if (depth == 4)  return v * 17u;
    if (depth == 2)  return v * 85u;
    return v * 255u;
}

/* -------------------------------------------------------------- Adam7
 * Seven passes over the same image, each a subsampled grid. Each pass is its
 * own little image with its own scanlines and its own filter history - the
 * filter state RESETS at the start of every pass, which is the bug an
 * interlaced decoder written as "the same loop seven times" always has. A pass
 * with a zero width or height contributes no bytes at all, not even a filter
 * byte, which is the other one. */

static const int A7X0[7] = { 0, 4, 0, 2, 0, 1, 0 };
static const int A7Y0[7] = { 0, 0, 4, 0, 2, 0, 1 };
static const int A7DX[7] = { 8, 8, 4, 4, 2, 2, 1 };
static const int A7DY[7] = { 8, 8, 8, 4, 4, 2, 2 };

static int pass_w(int p) { return (img_w - A7X0[p] + A7DX[p] - 1) / A7DX[p]; }
static int pass_h(int p) { return (img_h - A7Y0[p] + A7DY[p] - 1) / A7DY[p]; }

/* ------------------------------------------------------------- scanlines */

#define MAXROW (PNG_MAX_W * 8)          /* 16-bit RGBA at the widest we take */

static u8 rowa[MAXROW], rowb[MAXROW];

/* Turn one unfiltered scanline into pixels at (x0 + i*dx, y). */
static int expand(const u8 *row, int pw, int base, int x0, int dx, int y)
{
    unsigned int *dst = arena + base + y * img_w;
    int d = img_depth;

    for (int i = 0; i < pw; i++) {
        u32 r, g, b, a = 255;
        switch (img_colour) {
        case 0: {
            u32 v = sample(row, i, d);
            if (trns_ok && v == trns_v[0]) a = 0;
            r = g = b = to8(v, d);
            break;
        }
        case 2: {
            u32 sr = sample(row, i * 3 + 0, d);
            u32 sg = sample(row, i * 3 + 1, d);
            u32 sb = sample(row, i * 3 + 2, d);
            if (trns_ok && sr == trns_v[0] && sg == trns_v[1] && sb == trns_v[2])
                a = 0;
            r = to8(sr, d); g = to8(sg, d); b = to8(sb, d);
            break;
        }
        case 3: {
            u32 idx = sample(row, i, d);
            /* THE index check. A palette image may name entry 200 while
             * sending a PLTE of three colours, and there is nothing in the
             * format that stops it. */
            if (idx >= (u32)nplte) { err = PNG_E_CORRUPT; return -1; }
            r = plte[idx * 3 + 0];
            g = plte[idx * 3 + 1];
            b = plte[idx * 3 + 2];
            a = palpha[idx];
            break;
        }
        case 4: {
            u32 v  = sample(row, i * 2 + 0, d);
            u32 sa = sample(row, i * 2 + 1, d);
            r = g = b = to8(v, d);
            a = to8(sa, d);
            break;
        }
        default: {                                   /* colour type 6 */
            u32 sr = sample(row, i * 4 + 0, d);
            u32 sg = sample(row, i * 4 + 1, d);
            u32 sb = sample(row, i * 4 + 2, d);
            u32 sa = sample(row, i * 4 + 3, d);
            r = to8(sr, d); g = to8(sg, d); b = to8(sb, d); a = to8(sa, d);
            break;
        }
        }
        dst[x0 + i * dx] = (a << 24) | (r << 16) | (g << 8) | b;
    }
    return 0;
}

static int decode_pixels(int base)
{
    int npass = img_interlace ? 7 : 1;

    for (int p = 0; p < npass; p++) {
        int pw = img_interlace ? pass_w(p) : img_w;
        int ph = img_interlace ? pass_h(p) : img_h;
        int x0 = img_interlace ? A7X0[p] : 0;
        int y0 = img_interlace ? A7Y0[p] : 0;
        int dx = img_interlace ? A7DX[p] : 1;
        int dy = img_interlace ? A7DY[p] : 1;
        int nb = (pw * img_bitspp + 7) / 8;
        u8 *cur = rowa, *prev = rowb;

        if (pw == 0 || ph == 0) continue;
        pzero(prev, nb);

        for (int y = 0; y < ph; y++) {
            u8 *tmp;
            int f;
            if (need_bytes((u32)nb + 1u)) return -1;
            f = take();
            for (int i = 0; i < nb; i++) cur[i] = take();
            if (unfilter(f, cur, prev, nb, img_bpp)) return -1;
            if (expand(cur, pw, base, x0, dx, y0 + y * dy)) return -1;
            tmp = cur; cur = prev; prev = tmp;
        }
    }
    return 0;
}

/* --------------------------------------------------------- the chunk walk */

static const u8 SIG[8] = { 137, 80, 78, 71, 13, 10, 26, 10 };

static int is_type(const u8 *t, const char *s)
{
    return t[0] == (u8)s[0] && t[1] == (u8)s[1] &&
           t[2] == (u8)s[2] && t[3] == (u8)s[3];
}

static int depth_ok(int colour, int d)
{
    switch (colour) {
    case 0: return d == 1 || d == 2 || d == 4 || d == 8 || d == 16;
    case 2: return d == 8 || d == 16;
    case 3: return d == 1 || d == 2 || d == 4 || d == 8;
    case 4: return d == 8 || d == 16;
    case 6: return d == 8 || d == 16;
    default: return 0;
    }
}

static int chans_of(int colour)
{
    switch (colour) {
    case 0: return 1;
    case 2: return 3;
    case 3: return 1;
    case 4: return 2;
    default: return 4;                                /* colour type 6 */
    }
}

static int read_ihdr(const u8 *d, u32 clen)
{
    u32 w, h;

    if (clen != 13) { err = PNG_E_CORRUPT; return -1; }
    w = be32(d);
    h = be32(d + 4);
    img_depth  = d[8];
    img_colour = d[9];
    /* Compression and filter method are single defined values with room for
     * more; interlace has two. A file using a method that does not exist yet
     * is one we cannot read, not one that is broken. */
    if (d[10] != 0 || d[11] != 0 || d[12] > 1) { err = PNG_E_UNSUPPORTED; return -1; }
    img_interlace = d[12];

    if (w == 0 || h == 0) { err = PNG_E_CORRUPT; return -1; }
    if (w > (u32)PNG_MAX_W || h > (u32)PNG_MAX_H) { err = PNG_E_TOO_BIG; return -1; }
    if (!depth_ok(img_colour, img_depth)) { err = PNG_E_CORRUPT; return -1; }

    img_w = (int)w;
    img_h = (int)h;
    img_chans  = chans_of(img_colour);
    img_bitspp = img_chans * img_depth;
    img_bpp    = (img_bitspp + 7) / 8;
    return 0;
}

int png_decode(const unsigned char *src, int len)
{
    int off, base, need;
    int seen_ihdr = 0, seen_idat = 0, seen_iend = 0, idat_closed = 0;
    int i;

    err = PNG_OK;
    if (!crc_built) crc_build();

    if (!src || len < 8) { err = PNG_E_SIG; return -1; }
    for (i = 0; i < 8; i++)
        if (src[i] != SIG[i]) { err = PNG_E_SIG; return -1; }

    pngbuf = src;
    img_w = img_h = 0;
    img_interlace = 0;
    nplte = 0;
    trns_ok = 0;
    nidat = 0;
    for (i = 0; i < 256; i++) palpha[i] = 255;

    off = 8;
    while (off + 8 <= len) {
        u32 clen = be32(src + off);
        const u8 *type = src + off + 4;
        const u8 *data = src + off + 8;

        /* The format caps a chunk at 2^31-1. Refusing anything larger before
         * doing arithmetic with it is what makes the next line safe: len, off
         * and clen can then be combined without any of them wrapping. */
        if (clen > 0x7FFFFFFFu) { err = PNG_E_CORRUPT; return -1; }
        if ((u32)(len - off - 8) < clen + 4u) { err = PNG_E_TRUNCATED; return -1; }
        if (crc32_of(type, 4 + (int)clen) != be32(data + clen)) {
            err = PNG_E_CORRUPT;
            return -1;
        }

        if (is_type(type, "IHDR")) {
            if (seen_ihdr) { err = PNG_E_CORRUPT; return -1; }
            if (read_ihdr(data, clen)) return -1;
            seen_ihdr = 1;
        } else if (is_type(type, "PLTE")) {
            if (!seen_ihdr || seen_idat || nplte) { err = PNG_E_CORRUPT; return -1; }
            /* PLTE is illegal for the two greyscale types, and a suggested
             * palette for the truecolour ones - so it is taken and then only
             * consulted for colour type 3. */
            if (img_colour == 0 || img_colour == 4) { err = PNG_E_CORRUPT; return -1; }
            if (clen == 0 || clen % 3u || clen > 768u) { err = PNG_E_CORRUPT; return -1; }
            pcopy(plte, data, (int)clen);
            nplte = (int)clen / 3;
        } else if (is_type(type, "tRNS")) {
            if (!seen_ihdr || seen_idat) { err = PNG_E_CORRUPT; return -1; }
            if (img_colour == 3) {
                if (clen > (u32)nplte) { err = PNG_E_CORRUPT; return -1; }
                for (i = 0; i < (int)clen; i++) palpha[i] = data[i];
            } else if (img_colour == 0) {
                if (clen != 2) { err = PNG_E_CORRUPT; return -1; }
                trns_ok = 1;
                trns_v[0] = be16(data);
            } else if (img_colour == 2) {
                if (clen != 6) { err = PNG_E_CORRUPT; return -1; }
                trns_ok = 1;
                trns_v[0] = be16(data);
                trns_v[1] = be16(data + 2);
                trns_v[2] = be16(data + 4);
            } else {
                /* an alpha channel and a transparent colour at once */
                err = PNG_E_CORRUPT;
                return -1;
            }
        } else if (is_type(type, "IDAT")) {
            if (!seen_ihdr) { err = PNG_E_CORRUPT; return -1; }
            /* The IDATs have to be consecutive. If they were not, "the
             * concatenation" would depend on what a decoder chose to do with
             * the chunk in between, and two decoders would disagree. */
            if (idat_closed) { err = PNG_E_CORRUPT; return -1; }
            if (nidat >= MAX_IDAT) { err = PNG_E_UNSUPPORTED; return -1; }
            idat[nidat].off = off + 8;
            idat[nidat].len = (int)clen;
            nidat++;
            seen_idat = 1;
        } else if (is_type(type, "IEND")) {
            if (clen != 0) { err = PNG_E_CORRUPT; return -1; }
            seen_iend = 1;
            break;
        } else {
            if (seen_idat) idat_closed = 1;      /* anything else ends them */
        }

        off += 12 + (int)clen;
    }

    /* "it ran out" is asked FIRST, because it is the more useful answer and
     * because the other two are its consequences: a file cut off after the
     * signature has no IHDR either, and saying so would send whoever reads the
     * log looking for a malformed header that was never sent. A file that does
     * reach IEND and is still missing one of these really is malformed. */
    if (!seen_iend) { err = PNG_E_TRUNCATED; return -1; }
    if (!seen_ihdr) { err = PNG_E_CORRUPT; return -1; }
    if (!seen_idat) { err = PNG_E_CORRUPT; return -1; }
    if (img_colour == 3 && nplte == 0) { err = PNG_E_CORRUPT; return -1; }

    /* Room, before a byte is inflated. img_w and img_h are both at most 1024,
     * so the product is at most 2^20 and cannot overflow an int; the
     * subtraction is written that way round so it cannot either. */
    need = img_w * img_h;
    if (nslot >= PNG_MAX_SLOTS) { err = PNG_E_NO_ROOM; return -1; }
    if (!arena || need > arena_px - arena_used) { err = PNG_E_NO_ROOM; return -1; }
    base = arena_used;

    out_limit = 0;
    {
        int npass = img_interlace ? 7 : 1;
        for (i = 0; i < npass; i++) {
            int pw = img_interlace ? pass_w(i) : img_w;
            int ph = img_interlace ? pass_h(i) : img_h;
            u32 rb;
            if (pw == 0 || ph == 0) continue;
            rb = ((u32)pw * (u32)img_bitspp + 7u) / 8u;
            out_limit += (rb + 1u) * (u32)ph;
        }
    }

    zseg = zoff = 0;
    bitbuf = 0; bitcnt = 0;
    out_total = out_drained = 0;
    ad_a = 1; ad_b = 0;
    zstate = ZS_HDR;
    bfinal = 0;
    stored_left = 0;

    if (decode_pixels(base)) return -1;

    /* The stream has to END where the image does. Running it out proves three
     * separate things at once: that the last block was marked final, that
     * nothing follows the pixels we wanted, and that the Adler-32 over
     * everything produced matches what the encoder wrote. A decoder that
     * stopped at the last scanline would accept all three of those being
     * wrong. */
    while (!err && zstate != ZS_END) inf_step(1);
    if (err) return -1;

    slot[nslot].w = img_w;
    slot[nslot].h = img_h;
    slot[nslot].off = base;
    arena_used += need;
    return nslot++;
}
