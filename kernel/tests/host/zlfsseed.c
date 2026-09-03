/* zlfsseed.c - put exact fixture bytes into a disposable zlfs disk image. */
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

typedef unsigned int u32;

int fs_mkfs(void);
int fs_mount(void);
int fs_create(const char *name, u32 bytes);
int fs_find(const char *name);
int fs_write(int index, const void *source, u32 bytes);
int fs_read(int index, void *destination, u32 bytes);
int fs_sync(void);

enum {
    DEVICE_BLOCK_BYTES = 512,
    MAX_FIXTURE_BYTES = 16 * 1024 * 1024
};

static int image_fd = -1;
static u32 image_blocks;

void zl_putc_pub(char value)
{
    fputc(value, stderr);
}

int fsdev_read(u32 lba, void *buffer)
{
    if (lba >= image_blocks) return 0;
    off_t offset = (off_t)lba * DEVICE_BLOCK_BYTES;
    return pread(image_fd, buffer, DEVICE_BLOCK_BYTES, offset) ==
           DEVICE_BLOCK_BYTES;
}

int fsdev_write(u32 lba, const void *buffer)
{
    if (lba >= image_blocks) return 0;
    off_t offset = (off_t)lba * DEVICE_BLOCK_BYTES;
    return pwrite(image_fd, buffer, DEVICE_BLOCK_BYTES, offset) ==
           DEVICE_BLOCK_BYTES;
}

int fsdev_sync(void)
{
    return fsync(image_fd) == 0;
}

u32 fsdev_bsize(void) { return DEVICE_BLOCK_BYTES; }
u32 fsdev_blocks(void) { return image_blocks; }

static unsigned char *read_fixture(const char *path, u32 *bytes)
{
    struct stat state;
    if (stat(path, &state) != 0 || !S_ISREG(state.st_mode) ||
        state.st_size <= 0 || state.st_size > MAX_FIXTURE_BYTES) {
        fprintf(stderr, "zlfsseed: fixture must be a 1..%d byte regular file\n",
                MAX_FIXTURE_BYTES);
        return NULL;
    }
    FILE *stream = fopen(path, "rb");
    if (!stream) {
        fprintf(stderr, "zlfsseed: cannot open fixture: %s\n", strerror(errno));
        return NULL;
    }
    unsigned char *buffer = malloc((size_t)state.st_size);
    if (!buffer) {
        fclose(stream);
        fprintf(stderr, "zlfsseed: fixture allocation failed\n");
        return NULL;
    }
    size_t got = fread(buffer, 1, (size_t)state.st_size, stream);
    int close_status = fclose(stream);
    if (got != (size_t)state.st_size || close_status != 0) {
        free(buffer);
        fprintf(stderr, "zlfsseed: fixture read failed\n");
        return NULL;
    }
    *bytes = (u32)state.st_size;
    return buffer;
}

int main(int argc, char **argv)
{
    if (argc != 4) {
        fprintf(stderr, "usage: %s DISK-IMAGE ZLFS-NAME FIXTURE\n", argv[0]);
        return 2;
    }

    u32 fixture_bytes = 0;
    unsigned char *fixture = read_fixture(argv[3], &fixture_bytes);
    if (!fixture) return 2;

    image_fd = open(argv[1], O_RDWR);
    if (image_fd < 0) {
        fprintf(stderr, "zlfsseed: cannot open image: %s\n", strerror(errno));
        free(fixture);
        return 2;
    }
    struct stat state;
    if (fstat(image_fd, &state) != 0 || !S_ISREG(state.st_mode) ||
        state.st_size <= 0 || state.st_size % DEVICE_BLOCK_BYTES != 0 ||
        (uintmax_t)(state.st_size / DEVICE_BLOCK_BYTES) > UINT32_MAX) {
        fprintf(stderr, "zlfsseed: image must be a bounded 512-byte-aligned regular file\n");
        close(image_fd);
        free(fixture);
        return 2;
    }
    image_blocks = (u32)(state.st_size / DEVICE_BLOCK_BYTES);

    unsigned char *readback = malloc(fixture_bytes);
    if (!readback) {
        fprintf(stderr, "zlfsseed: readback allocation failed\n");
        close(image_fd);
        free(fixture);
        return 2;
    }
    int index = -1;
    int readback_index = -1;
    int ok = fs_mkfs() && fs_mount() &&
             (index = fs_create(argv[2], fixture_bytes)) >= 0 &&
             fs_write(index, fixture, fixture_bytes) && fs_sync() &&
             fs_mount() && (readback_index = fs_find(argv[2])) >= 0 &&
             fs_read(readback_index, readback, fixture_bytes) ==
                 (int)fixture_bytes &&
             memcmp(readback, fixture, fixture_bytes) == 0;
    free(readback);
    free(fixture);
    int close_status = close(image_fd);
    image_fd = -1;
    if (!ok || close_status != 0) {
        fprintf(stderr, "zlfsseed: write or readback verification failed\n");
        return 1;
    }
    printf("zlfsseed: wrote %u bytes as %s\n", fixture_bytes, argv[2]);
    return 0;
}
