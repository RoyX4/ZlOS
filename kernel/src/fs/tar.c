/* tar.c - a real ustar archive of the zlfs volume.
 *
 * WHY THIS EXISTS. apps_sys3.zl's Archive Manager drew a primary-styled button
 * labelled "Create archive" whose entire effect was `s3ar_open = 1` - a VIEW
 * flag, switching the pane from its empty state to a grid of the zlfs
 * directory. No ustar header was emitted, no block was written, no file
 * appeared. Beside it the toolbar drew the literal path "/tmp/build.tar",
 * right-flushed where a target file belongs, for a file nothing on the volume
 * was called and nothing in the tree ever wrote.
 *
 * A button with a verb on it that performs no verb is the fault this pane had;
 * the fix is the verb, not a smaller label.
 *
 * WHY THE WHOLE ARCHIVE IS BUILT IN ONE BUFFER. fs_write(idx, src, bytes)
 * takes a pointer and a length and writes the WHOLE file - zlfs has no append
 * and no seek, so a streamed tar is not expressible against this filesystem.
 * The archive is therefore staged contiguously and written once. That makes
 * the staging buffer a hard ceiling, and a ceiling that is silently exceeded
 * is worse than one that refuses: tar_size() is public so the caller can ask
 * BEFORE it commits, and tar_build refuses rather than truncating.
 *
 * THE FORMAT IS POSIX ustar, not GNU tar, and not "close enough". The checksum
 * is the one field that cannot be eyeballed - it is the sum of all 512 header
 * bytes with the checksum field itself read as eight spaces - so the host test
 * writes a real archive out and the shell's own tar reads it back. An archive
 * only this tree can open is not an archive.
 */

typedef unsigned int u32;

/* zlfs's surface. This file deliberately does not include fs.c's private
 * header - it uses the same public calls the rest of the kernel does, so a tar
 * cannot see a file the Files pane cannot. */
int  fs_mounted(void);
int  fs_used(int idx);
u32  fs_size(int idx);
u32  fs_mtime(int idx);
int  fs_maxfiles(void);
int  fs_name_byte(int idx, int i);
int  fs_read(int idx, void *dst, u32 max);
int  fs_create(const char *name, u32 bytes);
int  fs_write(int idx, const void *src, u32 bytes);
int  fs_delete(int idx);
int  fs_sync(void);

#define TAR_BLK 512

/* AN ARCHIVE MUST NOT CONTAIN ITSELF. The Archive Manager writes its output
 * back onto the same volume it just packed, so the SECOND "Create archive"
 * would pack the first archive into the new one, the third would pack the
 * second, and the file would roughly double every press until the volume
 * filled - a runaway that looks like ordinary use. GNU tar has the same hazard
 * and answers it with a warning; there is nobody here to read a warning, so
 * the member is skipped by name.
 *
 * The name is stated once, here, and the pane asks for it rather than
 * restating it - a second copy of this string is a second place for the skip
 * to stop matching the output. */
static const char TAR_SELF[] = "zlfs.tar";

int tar_self_byte(int i)
{
    if (i < 0 || i >= (int)sizeof TAR_SELF) return 0;
    return (unsigned char)TAR_SELF[i];
}

static int is_self(int idx)
{
    int i = 0;
    while (TAR_SELF[i]) {
        if (fs_name_byte(idx, i) != (int)(unsigned char)TAR_SELF[i]) return 0;
        i++;
    }
    return fs_name_byte(idx, i) == 0;
}

/* WHICH SLOT THE ARCHIVE IS IN, or -1.
 *
 * THE PANE MUST NOT ASK THIS THROUGH fs_name_push. zlfs finds a file by name
 * through a SINGLE SHARED staging buffer - fs_name_clear, fs_name_push,
 * fs_find_named - and the Files pane keeps a half-typed new filename in that
 * same buffer while its N-key field is open. A toolbar that staged "zlfs.tar"
 * to look up its own output would erase whatever the user was typing in
 * another window, on every frame it drew. So the search happens here, against
 * the directory directly, and touches nothing shared. */
int tar_slot(void)
{
    if (!fs_mounted()) return -1;
    for (int i = 0; i < fs_maxfiles(); i++)
        if (fs_used(i) && is_self(i)) return i;
    return -1;
}

static u32 blk_round(u32 n) { return (n + TAR_BLK - 1) / TAR_BLK * TAR_BLK; }

/* HOW BIG THE ARCHIVE WILL BE, without building it. One header block plus the
 * data rounded up per member, then the two zero blocks every ustar ends with.
 * The caller prints this figure when it refuses, so the refusal names the
 * number it was measured against rather than saying "too big". */
u32 tar_size(void)
{
    if (!fs_mounted()) return 0;
    u32 total = 0;
    for (int i = 0; i < fs_maxfiles(); i++) {
        if (!fs_used(i)) continue;
        if (is_self(i)) continue;
        total += TAR_BLK + blk_round(fs_size(i));
    }
    return total + 2 * TAR_BLK;
}

/* HOW MANY MEMBERS THE ARCHIVE HAS, AND HOW MANY BYTES OF THEM ARE PAYLOAD.
 *
 * The pane's strip counted both in zl, over every in-use slot - which included
 * the archive itself the moment one existed. The result contradicted itself on
 * screen: PAYLOAD 2564 sat next to ARCHIVE SIZE 2560, a payload larger than the
 * container it is inside, and MEMBERS said 3 for an archive holding 2.
 *
 * Both walk the same directory with the same skip tar_build uses, so the strip
 * cannot describe a different archive from the one the button writes. */
int tar_members(void)
{
    if (!fs_mounted()) return 0;
    int n = 0;
    for (int i = 0; i < fs_maxfiles(); i++) {
        if (!fs_used(i) || is_self(i)) continue;
        n++;
    }
    return n;
}

u32 tar_payload(void)
{
    if (!fs_mounted()) return 0;
    u32 n = 0;
    for (int i = 0; i < fs_maxfiles(); i++) {
        if (!fs_used(i) || is_self(i)) continue;
        n += fs_size(i);
    }
    return n;
}

static void put_str(unsigned char *p, const char *s, int n)
{
    int i = 0;
    while (i < n && s[i]) { p[i] = (unsigned char)s[i]; i++; }
    while (i < n) { p[i] = 0; i++; }
}

/* ustar's numbers are ZERO-PADDED OCTAL followed by a NUL, written right to
 * left into a field of n bytes - so an 8-byte field carries 7 digits. Getting
 * this wrong produces an archive that lists but extracts the wrong length,
 * which is exactly the failure the host test's `tar -tvf` catches. */
static void put_oct(unsigned char *p, u32 v, int n)
{
    int i = n - 1;
    p[i--] = 0;
    while (i >= 0) { p[i--] = (unsigned char)('0' + (v & 7)); v >>= 3; }
}

/* tar_build - the whole volume, into dst, or 0.
 *
 * Returns the byte count written, or 0 when the archive does not fit in cap or
 * nothing is mounted. It never writes past cap and never writes a partial
 * member: the size is computed first and checked once. */
u32 tar_build(void *dst, u32 cap)
{
    if (!fs_mounted() || !dst) return 0;
    u32 need = tar_size();
    if (need == 0 || need > cap) return 0;

    unsigned char *out = (unsigned char *)dst;
    u32 off = 0;

    for (int i = 0; i < fs_maxfiles(); i++) {
        if (!fs_used(i)) continue;
        if (is_self(i)) continue;
        unsigned char *h = out + off;
        for (int k = 0; k < TAR_BLK; k++) h[k] = 0;

        /* the name, byte by byte - fs.c has no way to hand out a pointer to
         * one, and the 100-byte field is far longer than FS_NAME_MAX */
        int n = 0;
        while (n < 99) {
            int c = fs_name_byte(i, n);
            if (c == 0) break;
            h[n] = (unsigned char)c;
            n++;
        }

        put_oct(h + 100, 0644, 8);          /* mode  */
        put_oct(h + 108, 0, 8);             /* uid   */
        put_oct(h + 116, 0, 8);             /* gid   */
        put_oct(h + 124, fs_size(i), 12);   /* size  */
        put_oct(h + 136, fs_mtime(i), 12);  /* mtime */
        h[156] = '0';                       /* typeflag: a regular file */
        put_str(h + 257, "ustar", 6);       /* magic, NUL-terminated    */
        h[263] = '0'; h[264] = '0';         /* version "00", NOT NUL    */
        put_str(h + 265, "root", 32);
        put_str(h + 297, "root", 32);

        /* THE CHECKSUM IS COMPUTED WITH ITS OWN FIELD READ AS SPACES. That is
         * the rule, and it is why the field is filled with spaces first and
         * overwritten after: summing the zeros that were there instead gives a
         * number that is wrong by 8 * ' ' and every tar refuses it. */
        for (int k = 148; k < 156; k++) h[k] = ' ';
        u32 sum = 0;
        for (int k = 0; k < TAR_BLK; k++) sum += h[k];
        put_oct(h + 148, sum, 7);           /* 6 octal digits then NUL  */
        h[154] = 0;
        h[155] = ' ';                       /* ...then a space          */
        off += TAR_BLK;

        u32 len = fs_size(i);
        if (len) {
            fs_read(i, out + off, len);
            u32 pad = blk_round(len) - len;
            for (u32 k = 0; k < pad; k++) out[off + len + k] = 0;
            off += blk_round(len);
        }
    }

    for (u32 k = 0; k < 2 * TAR_BLK; k++) out[off + k] = 0;
    off += 2 * TAR_BLK;
    return off;
}

/* tar_commit - stage the archive and put it on the volume, or refuse.
 *
 * Returns the bytes written, or 0. Every refusal is a refusal BEFORE anything
 * is destroyed except the previous archive, which is the one thing that has to
 * go first: its blocks are the space the new one needs, and leaving it there
 * makes a volume with room for one archive look like a volume with room for
 * none.
 *
 * fs_create takes the name directly, so this path never touches the shared
 * staging buffer either - see tar_slot above for why that matters.
 *
 * ORDER: delete, stage, create, write, sync. Staging before creating is what
 * keeps the archive's contents from including the empty file it is about to be
 * written into - and is why the size handed to fs_create is measured from the
 * staged bytes rather than predicted. */
u32 tar_commit(void *stage, u32 cap)
{
    if (!fs_mounted() || !stage) return 0;

    int old = tar_slot();
    if (old >= 0) fs_delete(old);

    u32 n = tar_build(stage, cap);
    if (n == 0) return 0;

    int idx = fs_create(TAR_SELF, n);
    if (idx < 0) return 0;
    if (!fs_write(idx, stage, n)) { fs_delete(idx); return 0; }
    fs_sync();
    return n;
}
