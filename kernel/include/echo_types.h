/*
 * Echo.Kern Core Types
 * 
 * Fundamental type definitions for the Echo.Kern ENGINE.
 * All types follow OEIS A000081 enumeration principles.
 */

#ifndef _ECHO_TYPES_H
#define _ECHO_TYPES_H

#include <stdint.h>
#include <stddef.h>

/* OEIS A000081 magic number */
#define ECHO_MAGIC 0xECH0A00081ULL

/* Prime power series types */
typedef uint64_t prime_t;
typedef uint32_t exponent_t;
typedef uint64_t prime_power_t;

/* Security levels (OEIS A000081 partitioning) */
typedef enum {
    ECHO_LEVEL_FIRMWARE = -3,    /* 1 context */
    ECHO_LEVEL_DEVICE   = -2,    /* 2 contexts */
    ECHO_LEVEL_HYPERVISOR = -1,  /* 4 contexts */
    ECHO_LEVEL_KERNEL   = 0,     /* 9 contexts */
    ECHO_LEVEL_SERVICE  = 1,     /* 20 contexts */
    ECHO_LEVEL_APP      = 2,     /* 48 contexts */
    ECHO_LEVEL_THREAD   = 3      /* 115 contexts */
} echo_security_level_t;

/* OEIS A000081 enumeration values (precomputed) */
extern const uint32_t oeis_a000081[];
extern const size_t oeis_a000081_length;

/* Get OEIS A000081 value for depth */
static inline uint32_t oeis_value(uint32_t depth) {
    extern const uint32_t oeis_a000081[];
    extern const size_t oeis_a000081_length;
    return (depth < oeis_a000081_length) ? oeis_a000081[depth] : 0;
}

/* Error codes */
#define ECHO_SUCCESS      0
#define ECHO_EINVAL      -1  /* Invalid argument */
#define ECHO_ENOMEM      -2  /* Out of memory */
#define ECHO_EMATH       -3  /* Mathematical constraint violation */
#define ECHO_ENOTIME     -4  /* Real-time constraint violated */
#define ECHO_ESECURITY   -5  /* Security violation */

#endif /* _ECHO_TYPES_H */
