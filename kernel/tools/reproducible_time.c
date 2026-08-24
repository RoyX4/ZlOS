#define _GNU_SOURCE

#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

/*
 * grub-mkrescue 2.14 still derives an internal hidden UUID filename from
 * gettimeofday() and an EFI FAT serial from time().  xorriso's reproducible
 * build flags run too late to control those values.  Preload this library only
 * for grub-mkrescue and its packaging children so both APIs see the same
 * SOURCE_DATE_EPOCH.  Monotonic clocks are deliberately untouched.
 */

static time_t source_date_epoch(void)
{
    const char *value = getenv("SOURCE_DATE_EPOCH");
    char *end = NULL;
    unsigned long long parsed;

    if (value == NULL || *value == '\0') {
        return (time_t)0;
    }

    errno = 0;
    parsed = strtoull(value, &end, 10);
    if (errno != 0 || end == value || *end != '\0' || parsed > (unsigned long long)LLONG_MAX) {
        return (time_t)0;
    }
    return (time_t)parsed;
}

time_t time(time_t *result)
{
    time_t epoch = source_date_epoch();

    if (result != NULL) {
        *result = epoch;
    }
    return epoch;
}

int gettimeofday(struct timeval *tv, void *timezone_ignored)
{
    (void)timezone_ignored;
    tv->tv_sec = source_date_epoch();
    tv->tv_usec = 0;
    return 0;
}
