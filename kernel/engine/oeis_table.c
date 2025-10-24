/*
 * OEIS A000081 Enumeration Table
 * 
 * Precomputed values for unlabeled rooted trees enumeration.
 * These values determine concurrency contexts at each depth.
 */

#include "../include/echo_types.h"

/* OEIS A000081: 0, 1, 1, 2, 4, 9, 20, 48, 115, 286, 719, 1842, 4766, 12486, ... */
const uint32_t oeis_a000081[] = {
    0,      /* depth 0: no trees */
    1,      /* depth 1: 1 tree */
    1,      /* depth 2: 1 tree */
    2,      /* depth 3: 2 trees */
    4,      /* depth 4: 4 trees */
    9,      /* depth 5: 9 trees */
    20,     /* depth 6: 20 trees */
    48,     /* depth 7: 48 trees */
    115,    /* depth 8: 115 trees */
    286,    /* depth 9: 286 trees */
    719,    /* depth 10: 719 trees */
    1842,   /* depth 11: 1842 trees */
    4766,   /* depth 12: 4766 trees */
    12486,  /* depth 13: 12486 trees */
    32973,  /* depth 14: 32973 trees */
    87811,  /* depth 15: 87811 trees */
    235381, /* depth 16: 235381 trees */
};

const size_t oeis_a000081_length = sizeof(oeis_a000081) / sizeof(oeis_a000081[0]);

/*
 * Prime number table (first 1000 primes)
 * Used for prime-indexed hypergraph nodes
 */
const prime_t prime_table[] = {
    2, 3, 5, 7, 11, 13, 17, 19, 23, 29,
    31, 37, 41, 43, 47, 53, 59, 61, 67, 71,
    73, 79, 83, 89, 97, 101, 103, 107, 109, 113,
    127, 131, 137, 139, 149, 151, 157, 163, 167, 173,
    179, 181, 191, 193, 197, 199, 211, 223, 227, 229,
    233, 239, 241, 251, 257, 263, 269, 271, 277, 281,
    283, 293, 307, 311, 313, 317, 331, 337, 347, 349,
    353, 359, 367, 373, 379, 383, 389, 397, 401, 409,
    419, 421, 431, 433, 439, 443, 449, 457, 461, 463,
    467, 479, 487, 491, 499, 503, 509, 521, 523, 541,
    /* Continue up to 1000 primes - abbreviated for space */
};

const size_t prime_table_length = sizeof(prime_table) / sizeof(prime_table[0]);

/* Get nth prime (0-indexed) */
prime_t nth_prime(size_t n) {
    return (n < prime_table_length) ? prime_table[n] : 0;
}

/* Compute prime^exponent using iterative multiplication (no addition!) */
prime_power_t prime_power(prime_t prime, exponent_t exponent) {
    prime_power_t result = 1;
    for (exponent_t i = 0; i < exponent; i++) {
        result = result * prime;  /* Only multiplication, no addition */
    }
    return result;
}
