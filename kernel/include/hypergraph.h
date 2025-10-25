/*
 * Hypergraph Filesystem (HGFS)
 * 
 * Prime power series-based memory and storage system.
 * NO ADDITION - only prime factorization and multiplication.
 */

#ifndef _HYPERGRAPH_H
#define _HYPERGRAPH_H

#include "echo_types.h"

/* Hypergraph node - represents a prime in the power series */
struct hgfs_node {
    prime_t prime;              /* Prime number ID for this node */
    exponent_t exponent;        /* Power in prime factorization */
    
    /* Edges to other nodes (product relationships only) */
    struct hgfs_edge *edges;
    uint32_t edge_count;
    
    /* Memory/data stored at this node */
    void *data;
    size_t data_size;
    
    /* Next node in hash table chain */
    struct hgfs_node *next;
};

/* Hypergraph edge - weighted by prime powers */
struct hgfs_edge {
    struct hgfs_node *target;   /* Target node */
    prime_t weight_prime;       /* Prime for edge weight */
    exponent_t weight_exponent; /* Exponent for edge weight */
    
    /* Edge type (relationship) */
    enum {
        HGFS_EDGE_SUMMATION = 2,      /* Concurrent aggregation (2^n) */
        HGFS_EDGE_PRODUCTION = 3,     /* Concurrent multiplication (3^n) */
        HGFS_EDGE_EXPONENTIATION = 5  /* Concurrent power (5^n) */
    } relation_type;
    
    struct hgfs_edge *next;
};

/* Hypergraph filesystem context */
struct hypergraph_fs {
    struct hgfs_node *root;     /* Root node at prime 2 */
    struct hgfs_node **table;   /* Hash table of nodes */
    size_t table_size;
    
    /* Prime pool for allocation */
    prime_t *prime_pool;
    size_t prime_pool_size;
    size_t prime_pool_index;
    
    /* Statistics */
    uint64_t node_count;
    uint64_t edge_count;
    uint64_t alloc_count;
    uint64_t free_count;
};

/* Initialize hypergraph filesystem */
int hgfs_init(struct hypergraph_fs **hgfs_out);

/* Destroy hypergraph filesystem */
void hgfs_destroy(struct hypergraph_fs *hgfs);

/* Allocate memory via hypergraph (prime-indexed) */
void *hgfs_alloc(struct hypergraph_fs *hgfs, size_t size, uint32_t depth);

/* Free memory via prime factorization */
void hgfs_free(struct hypergraph_fs *hgfs, void *ptr);

/* Reallocate memory via prime remapping */
void *hgfs_realloc(struct hypergraph_fs *hgfs, void *ptr, size_t new_size);

/* Create node at specific prime */
struct hgfs_node *hgfs_node_create(struct hypergraph_fs *hgfs, prime_t prime, exponent_t exp);

/* Find node by prime */
struct hgfs_node *hgfs_node_find(struct hypergraph_fs *hgfs, prime_t prime);

/* Create edge between nodes */
int hgfs_edge_create(struct hypergraph_fs *hgfs,
                     struct hgfs_node *from,
                     struct hgfs_node *to,
                     prime_t weight_prime,
                     exponent_t weight_exp,
                     int relation_type);

/* Compute prime power: prime^exponent */
prime_power_t prime_power(prime_t prime, exponent_t exponent);

/* Get nth prime number */
prime_t nth_prime(size_t n);

/* Prime factorization for pointer-to-prime mapping */
prime_t ptr_to_prime(void *ptr);

#endif /* _HYPERGRAPH_H */
