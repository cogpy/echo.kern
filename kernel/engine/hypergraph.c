/*
 * Hypergraph Filesystem Implementation
 * 
 * Prime power series-based memory and storage.
 * Core ENGINE component - NO ADDITION, only prime factorization.
 */

#include "../include/hypergraph.h"
#include "../include/echo_types.h"
#include <stdlib.h>
#include <string.h>

#define HGFS_HASH_TABLE_SIZE 1024
#define HGFS_PRIME_POOL_SIZE 10000

/* External prime functions */
extern prime_t nth_prime(size_t n);
extern prime_power_t prime_power(prime_t prime, exponent_t exponent);

/* Hash function for prime-indexed nodes */
static inline size_t hgfs_hash(prime_t prime) {
    return prime % HGFS_HASH_TABLE_SIZE;
}

/*
 * hgfs_init - Initialize hypergraph filesystem
 * 
 * Returns: 0 on success, negative error code on failure
 */
int hgfs_init(struct hypergraph_fs **hgfs_out) {
    struct hypergraph_fs *hgfs;
    
    if (!hgfs_out)
        return ECHO_EINVAL;
    
    /* Allocate HGFS context */
    hgfs = malloc(sizeof(*hgfs));
    if (!hgfs)
        return ECHO_ENOMEM;
    
    memset(hgfs, 0, sizeof(*hgfs));
    
    /* Allocate hash table */
    hgfs->table = calloc(HGFS_HASH_TABLE_SIZE, sizeof(struct hgfs_node *));
    if (!hgfs->table) {
        free(hgfs);
        return ECHO_ENOMEM;
    }
    hgfs->table_size = HGFS_HASH_TABLE_SIZE;
    
    /* Allocate prime pool */
    hgfs->prime_pool = malloc(HGFS_PRIME_POOL_SIZE * sizeof(prime_t));
    if (!hgfs->prime_pool) {
        free(hgfs->table);
        free(hgfs);
        return ECHO_ENOMEM;
    }
    hgfs->prime_pool_size = HGFS_PRIME_POOL_SIZE;
    hgfs->prime_pool_index = 0;
    
    /* Initialize prime pool with precomputed primes */
    for (size_t i = 0; i < HGFS_PRIME_POOL_SIZE; i++) {
        hgfs->prime_pool[i] = nth_prime(i);
    }
    
    /* Create root node at prime 2 */
    hgfs->root = hgfs_node_create(hgfs, 2, 1);
    if (!hgfs->root) {
        free(hgfs->prime_pool);
        free(hgfs->table);
        free(hgfs);
        return ECHO_ENOMEM;
    }
    
    *hgfs_out = hgfs;
    return ECHO_SUCCESS;
}

/*
 * hgfs_destroy - Destroy hypergraph filesystem
 */
void hgfs_destroy(struct hypergraph_fs *hgfs) {
    if (!hgfs)
        return;
    
    /* Free all nodes in hash table */
    for (size_t i = 0; i < hgfs->table_size; i++) {
        struct hgfs_node *node = hgfs->table[i];
        while (node) {
            struct hgfs_node *next = node->next;
            
            /* Free edges */
            struct hgfs_edge *edge = node->edges;
            while (edge) {
                struct hgfs_edge *next_edge = edge->next;
                free(edge);
                edge = next_edge;
            }
            
            /* Free node data */
            if (node->data)
                free(node->data);
            
            free(node);
            node = next;
        }
    }
    
    free(hgfs->prime_pool);
    free(hgfs->table);
    free(hgfs);
}

/*
 * hgfs_node_create - Create hypergraph node at specific prime
 * 
 * Returns: Pointer to node, or NULL on failure
 */
struct hgfs_node *hgfs_node_create(struct hypergraph_fs *hgfs, prime_t prime, exponent_t exp) {
    struct hgfs_node *node;
    size_t hash;
    
    if (!hgfs || prime == 0)
        return NULL;
    
    /* Allocate node */
    node = malloc(sizeof(*node));
    if (!node)
        return NULL;
    
    memset(node, 0, sizeof(*node));
    node->prime = prime;
    node->exponent = exp;
    
    /* Insert into hash table */
    hash = hgfs_hash(prime);
    node->next = hgfs->table[hash];
    hgfs->table[hash] = node;
    
    hgfs->node_count++;
    
    return node;
}

/*
 * hgfs_node_find - Find node by prime
 * 
 * Returns: Pointer to node, or NULL if not found
 */
struct hgfs_node *hgfs_node_find(struct hypergraph_fs *hgfs, prime_t prime) {
    size_t hash;
    struct hgfs_node *node;
    
    if (!hgfs)
        return NULL;
    
    hash = hgfs_hash(prime);
    node = hgfs->table[hash];
    
    while (node) {
        if (node->prime == prime)
            return node;
        node = node->next;
    }
    
    return NULL;
}

/*
 * hgfs_edge_create - Create edge between nodes
 * 
 * Returns: 0 on success, negative error code on failure
 */
int hgfs_edge_create(struct hypergraph_fs *hgfs,
                     struct hgfs_node *from,
                     struct hgfs_node *to,
                     prime_t weight_prime,
                     exponent_t weight_exp,
                     int relation_type) {
    struct hgfs_edge *edge;
    
    if (!hgfs || !from || !to)
        return ECHO_EINVAL;
    
    /* Allocate edge */
    edge = malloc(sizeof(*edge));
    if (!edge)
        return ECHO_ENOMEM;
    
    edge->target = to;
    edge->weight_prime = weight_prime;
    edge->weight_exponent = weight_exp;
    edge->relation_type = relation_type;
    
    /* Add to from node's edge list */
    edge->next = from->edges;
    from->edges = edge;
    from->edge_count++;
    
    hgfs->edge_count++;
    
    return ECHO_SUCCESS;
}

/*
 * hgfs_alloc - Allocate memory via hypergraph (prime-indexed)
 * 
 * Context: Can be called from any context (non-blocking)
 * Timing: Target ≤100ns
 * 
 * Returns: Pointer to allocated memory, or NULL on failure
 */
void *hgfs_alloc(struct hypergraph_fs *hgfs, size_t size, uint32_t depth) {
    struct hgfs_node *node;
    prime_t prime;
    void *data;
    
    if (!hgfs || size == 0)
        return NULL;
    
    /* Get next prime from pool */
    if (hgfs->prime_pool_index >= hgfs->prime_pool_size)
        return NULL;  /* Out of primes */
    
    prime = hgfs->prime_pool[hgfs->prime_pool_index++];
    
    /* Create node at this prime */
    node = hgfs_node_create(hgfs, prime, 1);
    if (!node) {
        hgfs->prime_pool_index--;  /* Return prime to pool */
        return NULL;
    }
    
    /* Allocate actual data */
    data = malloc(size);
    if (!data) {
        /* TODO: Remove node from hash table */
        hgfs->prime_pool_index--;
        return NULL;
    }
    
    node->data = data;
    node->data_size = size;
    
    hgfs->alloc_count++;
    
    return data;
}

/*
 * ptr_to_prime - Map pointer to prime (for freeing)
 * 
 * This is a simplified implementation. In production, we'd maintain
 * a proper mapping table or use the pointer value itself as part of
 * the prime factorization.
 */
prime_t ptr_to_prime(void *ptr) {
    /* Simplified: hash pointer to find associated prime */
    /* In production: use proper reverse lookup table */
    return (prime_t)((uintptr_t)ptr % 65521);  /* Prime modulus */
}

/*
 * hgfs_free - Free memory via prime factorization
 * 
 * Context: Can be called from any context
 * Timing: Target ≤100ns
 */
void hgfs_free(struct hypergraph_fs *hgfs, void *ptr) {
    /* TODO: Implement proper prime factorization-based free */
    /* For now, use standard free */
    if (ptr)
        free(ptr);
    
    hgfs->free_count++;
}

/*
 * hgfs_realloc - Reallocate memory via prime remapping
 * 
 * Timing: Target ≤200ns
 */
void *hgfs_realloc(struct hypergraph_fs *hgfs, void *ptr, size_t new_size) {
    void *new_ptr;
    
    if (!hgfs)
        return NULL;
    
    if (!ptr)
        return hgfs_alloc(hgfs, new_size, 0);
    
    if (new_size == 0) {
        hgfs_free(hgfs, ptr);
        return NULL;
    }
    
    /* Allocate new memory */
    new_ptr = hgfs_alloc(hgfs, new_size, 0);
    if (!new_ptr)
        return NULL;
    
    /* Copy data (TODO: get old size from node) */
    memcpy(new_ptr, ptr, new_size);  /* Simplified */
    
    /* Free old memory */
    hgfs_free(hgfs, ptr);
    
    return new_ptr;
}
