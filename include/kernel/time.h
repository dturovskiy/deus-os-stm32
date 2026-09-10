#ifndef KERNEL_TIME_H
#define KERNEL_TIME_H

#include <stdint.h>

typedef uint32_t kernel_time_ms_t;

/* Monotonic 1 ms kernel time; wraps naturally modulo 2^32. */
kernel_time_ms_t kernel_time_now(void);

/*
 * Wraparound-safe deadline comparison.
 * The ordering is unambiguous when deadline and now differ by less than 2^31 ms.
 */
int kernel_time_reached(kernel_time_ms_t now, kernel_time_ms_t deadline);

/*
 * Wraparound-safe elapsed-time comparison using unsigned modulo subtraction.
 * This detects elapsed durations across a single 32-bit counter wrap.
 */
int kernel_time_elapsed(kernel_time_ms_t start, kernel_time_ms_t duration);

#endif
