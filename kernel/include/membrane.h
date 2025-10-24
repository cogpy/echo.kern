/*
 * P-System Membrane Computing
 * 
 * Hierarchical membranes following OEIS A000081 topology.
 */

#ifndef _MEMBRANE_H
#define _MEMBRANE_H

#include "echo_types.h"
#include "hypergraph.h"

#define MAX_MEMBRANE_CHILDREN 20  /* OEIS A000081: max at depth 5 */

/* P-System membrane structure */
struct membrane {
    prime_t prime_id;              /* Prime identifier */
    uint32_t depth;                /* Depth in OEIS tree */
    
    /* Parent-child relationships */
    struct membrane *parent;
    struct membrane *children[MAX_MEMBRANE_CHILDREN];
    uint32_t child_count;
    
    /* Object multiset (P-System objects) */
    void **objects;
    uint32_t *object_counts;
    uint32_t object_type_count;
    
    /* P-lingua evolution rules */
    struct plingua_rule *rules;
    uint32_t rule_count;
    
    /* Hypergraph node for this membrane */
    struct hgfs_node *hgfs_node;
    
    /* Event loop for this membrane */
    struct event_loop *loop;
    
    /* State */
    enum {
        MEMBRANE_ACTIVE,
        MEMBRANE_DORMANT,
        MEMBRANE_DISSOLVED
    } state;
};

/* Membrane topology (rooted tree) */
struct membrane_topology {
    struct membrane *root;          /* Root membrane (prime 2) */
    uint32_t max_depth;             /* Maximum depth */
    uint32_t *depth_counts;         /* OEIS A000081 counts per depth */
};

/* P-lingua evolution rule */
struct plingua_rule {
    uint32_t priority;
    
    /* Left-hand side (consumed objects) */
    uint32_t *lhs_objects;
    uint32_t lhs_count;
    
    /* Right-hand side (produced objects) */
    uint32_t *rhs_objects;
    uint32_t rhs_count;
    
    /* Membrane operations */
    enum {
        RULE_NONE,
        RULE_DISSOLVE,
        RULE_DIVIDE,
        RULE_SEND_IN,
        RULE_SEND_OUT
    } operation;
};

/* Initialize membrane topology */
int membrane_topology_init(struct membrane_topology **topo_out, uint32_t max_depth);

/* Create membrane at specific depth */
struct membrane *membrane_create(struct membrane_topology *topo,
                                struct membrane *parent,
                                prime_t prime_id,
                                uint32_t depth);

/* Destroy membrane */
void membrane_destroy(struct membrane *membrane);

/* Evolve membrane (apply P-System rules) */
int membrane_evolve(struct membrane *membrane, uint64_t timestamp_ns);

/* Membrane communication */
int membrane_send(struct membrane *src, struct membrane *dst,
                 const void *data, size_t size);

int membrane_receive(struct membrane *membrane, void *buf, size_t size);

/* Add object to membrane multiset */
int membrane_add_object(struct membrane *membrane, uint32_t object_type, uint32_t count);

/* Remove object from membrane multiset */
int membrane_remove_object(struct membrane *membrane, uint32_t object_type, uint32_t count);

/* Add evolution rule */
int membrane_add_rule(struct membrane *membrane, const struct plingua_rule *rule);

#endif /* _MEMBRANE_H */
