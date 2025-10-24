/*
 * Event Loop Hierarchy
 * 
 * Nested event loops within membrane structure.
 * Drives the ENGINE's computational activity.
 */

#ifndef _EVENT_LOOP_H
#define _EVENT_LOOP_H

#include "echo_types.h"
#include "hypergraph.h"

#define MAX_CHILD_LOOPS 20  /* OEIS A000081 limit */

/* Event types */
typedef enum {
    EVENT_MEMBRANE_EVOLVE,
    EVENT_HYPERGRAPH_PROPAGATE,
    EVENT_TIMER_EXPIRE,
    EVENT_IO_READY,
    EVENT_INTERRUPT
} event_type_t;

/* Event structure */
struct event {
    event_type_t type;
    prime_t source_prime;
    void *data;
    size_t data_size;
    uint64_t timestamp_ns;
    struct event *next;
};

/* Event queue (using hypergraph edges) */
struct event_queue {
    struct hgfs_node *queue_node;   /* Hypergraph node for queue */
    struct event *head;
    struct event *tail;
    uint32_t count;
};

/* Event loop */
struct event_loop {
    prime_t prime_id;                           /* Prime identifier */
    struct event_loop *parent;                  /* Parent loop */
    struct event_loop *children[MAX_CHILD_LOOPS]; /* Child loops */
    uint32_t child_count;
    
    /* Event queue */
    struct event_queue *queue;
    
    /* Associated membrane */
    struct membrane *membrane;
    
    /* Hypergraph context */
    struct hypergraph_fs *hgfs;
    
    /* State */
    volatile int running;
    uint64_t iteration_count;
    
    /* Statistics */
    uint64_t events_processed;
    uint64_t events_dropped;
};

/* Initialize event loop */
int event_loop_init(struct event_loop **loop_out,
                   struct hypergraph_fs *hgfs,
                   prime_t prime_id);

/* Destroy event loop */
void event_loop_destroy(struct event_loop *loop);

/* Add child event loop */
int event_loop_add_child(struct event_loop *parent, struct event_loop *child);

/* Run event loop */
void event_loop_run(struct event_loop *loop);

/* Stop event loop */
void event_loop_stop(struct event_loop *loop);

/* Post event to queue */
int event_post(struct event_loop *loop, event_type_t type,
              prime_t source_prime, void *data, size_t data_size);

/* Process one event */
int event_process(struct event_loop *loop, struct event *event);

/* Propagate event via hypergraph edges */
int event_propagate(struct event_loop *loop, struct event *event);

/* Spawn child event loops recursively (following OEIS A000081) */
int event_loop_spawn_children(struct event_loop *parent, uint32_t depth);

#endif /* _EVENT_LOOP_H */
