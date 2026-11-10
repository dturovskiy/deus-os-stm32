/* ESP01-010 freestanding C11 support for frozen Node Link codec.
 * Firmware links -nostdlib -fno-builtin, so libc memcpy/memmove/memset
 * cannot be assumed. No heap or hardware I/O.
 */
#include <stddef.h>
#include <stdint.h>
#include <string.h>

void *memcpy(void *restrict dst, const void *restrict src, size_t count)
{
    unsigned char *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;
    size_t i;
    for (i = 0u; i < count; ++i) d[i] = s[i];
    return dst;
}

void *memmove(void *dst, const void *src, size_t count)
{
    unsigned char *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;
    size_t i;
    if (d == s || count == 0u) return dst;
    if ((uintptr_t)d <= (uintptr_t)s) {
        for (i = 0u; i < count; ++i) d[i] = s[i];
    } else {
        for (i = count; i != 0u; --i) d[i - 1u] = s[i - 1u];
    }
    return dst;
}

void *memset(void *dst, int value, size_t count)
{
    unsigned char *d = (unsigned char *)dst;
    size_t i;
    for (i = 0u; i < count; ++i) d[i] = (unsigned char)value;
    return dst;
}
