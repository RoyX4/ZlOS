/* png.h - decoded images, and the one thing this decoder cannot decide for
 * itself.
 *
 * SCOPE, stated up front the way css.h states its own, because "decode images"
 * is another of those tasks with no natural end. This one decodes the whole
 * of the PNG still-image surface and stops exactly there:
 *
 *   colour types 0 grey, 2 RGB, 3 palette, 4 grey+alpha, 6 RGBA
 *   bit depths   1, 2, 4, 8 and 16 (16 is downsampled to 8, see below)
 *   filters      None, Sub, Up, Average, Paeth
 *   interlace    none, and Adam7
 *   ancillary    tRNS, for a palette and for the one transparent colour of
 *                types 0 and 2. Every other ancillary chunk is skipped.
 *
 * What it deliberately does NOT do, and these are decisions rather than a
 * to-do list:
 *
 *   - NO APNG. An animated PNG decodes as its first frame, because acTL/fcTL/
 *     fdAT are skipped like any other unknown chunk. A browser that showed one
 *     frame of an animation is right; one that showed nothing is not.
 *   - NO COLOUR MANAGEMENT. gAMA, sRGB, iCCP and cHRM are skipped and samples
 *     are used as stored. Getting this wrong is a slightly wrong colour;
 *     carrying it means a colour transform with no floating point, and this
 *     kernel has paths with no FPU state saved.
 *   - NO 16-BIT OUTPUT. A 16-bit sample keeps its high byte. fb.c blits 8 bits
 *     per channel, so the low byte has nowhere to go.
 *   - NO PARTIAL RESULT. A decode either produces every pixel or produces
 *     nothing and says why. A half-decoded image is worse than a placeholder
 *     box, because nobody can tell it apart from a correct one.
 *
 * WHERE THE PIXELS LIVE IS THE CALLER'S DECISION, not this file's, and that is
 * the one place this header differs from what you might expect. Measured on
 * this branch: the kernel image already ends at 0x00592720 (5.573 MiB) and
 * link.ld asserts it stays under 0x00600000, so there are 438 KiB of BSS left
 * in the whole kernel. A 2 MiB pixel arena declared here would not link - the
 * build would stop with "the kernel image has grown into the raw-boot stack at
 * 6 MiB". So png.c owns the slot bookkeeping and the caller owns the storage.
 *
 * That storage is `HI_IMG` in memmap.h, which is where the address lives and
 * the only place it should be written down - a second copy here would be a
 * number that can drift out of step with the asserts guarding it, which is the
 * whole reason memmap.h exists. browser.c makes the one call. This file names
 * only the SIZE, below, because the size is a budget png.c enforces and the
 * address is a map decision png.c has no business making.
 *
 * UNTIL png_set_arena IS CALLED EVERY DECODE FAILS WITH PNG_E_NO_ROOM. That is
 * deliberate and loud: a decoder that silently fell back to a small built-in
 * buffer would work on icons, fail on photographs, and give nobody a reason.
 */
#ifndef PNG_H
#define PNG_H

#define PNG_MAX_W 1024
#define PNG_MAX_H 1024

/* The budget, not the geometry - the same argument arena.c makes. 524288
 * pixels is 2 MiB of 0xAARRGGBB, which is a page's worth of logos plus one
 * photograph. A ceiling set to "whatever was left" tells you nothing when you
 * hit it; this one says the page asked for more images than a page should. */
#define PNG_ARENA_PX  524288
#define PNG_MAX_SLOTS 12

/* Why a decode stopped, so a caller can say which of several things went wrong
 * rather than "broken image".
 *
 * The split between CORRUPT and UNSUPPORTED is worth stating because it is not
 * the obvious one: UNSUPPORTED means the file is WELL FORMED and uses
 * something this decoder does not carry (a zlib preset dictionary, a
 * compression/filter/interlace method the format reserves for later). Anything
 * the PNG specification forbids outright - a bit depth of 3, a colour type of
 * 7, a chunk length past 2^31 - is CORRUPT, because such a file was not
 * produced by an encoder, it was produced by an attacker or by a disk. */
#define PNG_OK            0
#define PNG_E_SIG         1   /* not a PNG at all                            */
#define PNG_E_UNSUPPORTED 2   /* well formed, uses something we do not carry */
#define PNG_E_TOO_BIG     3   /* wider or taller than PNG_MAX_W/H            */
#define PNG_E_CORRUPT     4   /* the file contradicts itself                 */
#define PNG_E_NO_ROOM     5   /* the pixel arena, or the slot table, is full */
#define PNG_E_TRUNCATED   6   /* it ran out - of file, or of deflate output  */

/* Hand png.c the pixel storage. Resets every slot, because the old ones point
 * into memory that is no longer this arena's. Call once at boot. */
void png_set_arena(unsigned int *px, int npx);

/* Decode into the arena. Returns a slot >= 0, or -1 with png_why() set.
 * Pixels are 0xAARRGGBB, one unsigned int per pixel, row-major, no padding -
 * the same format fb.c blits. `src` is only read during this call. */
int png_decode(const unsigned char *src, int len);

/* Why the last png_decode failed. PNG_OK after one that succeeded. */
int png_why(void);

int png_w(int slot);
int png_h(int slot);
const unsigned int *png_pixels(int slot);

void png_reset(void);          /* drop every slot; a new page */
int  png_slots_used(void);
int  png_arena_used(void);     /* in pixels, not bytes */

#endif
