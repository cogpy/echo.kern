/*
 * Event Loop Implementation
 * 
 * Nested event loops within membrane hierarchy.
 * Drives the ENGINE's computational activity.
 */

#include "../include/event_loop.h"
#include "../include/echo_types.h"
#include <stdlib.h>
#include <string.h>

extern struct hgfs_node *hgfs_node_create(struct hypergraph_fs *hgfs, prime_t prime, exponent_t exp);

/*
 * event_loop_init - Initialize event loop
 */
int event_loop_init(struct event_loop **loop_out,
                   struct hypergraph_fs *hgfs,
                   prime_t prime_id) {
    struct event_loop *loop;
    
    if (!loop_out || !hgfs)
        return ECHO_EINVAL;
    
    loop = malloc(sizeof(*loop));
    if (!loop)
        return ECHO_ENOMEM;
    
    memset(loop, 0, sizeof(*loop));
    
    loop->prime_id = prime_id;
    loop->hgfs = hgfs;
    loop->running = 0;
    
    /* Create event queue */
    loop->queue = malloc(sizeof(*loop->queue));
    if (!loop->queue) {
        free(loop);
        return ECHO_ENOMEM;
    }
    
    memset(loop->queue, 0, sizeof(*loop->queue));
    
    /* Create hypergraph node for queue */
    loop->queue->queue_node = hgfs_node_create(hgfs, prime_id, 1);
    if (!loop->queue->queue_node) {
        free(loop->queue);
        free(loop);
        return ECHO_ENOMEM;
    }
    
    *loop_out = loop;
    return ECHO_SUCCESS;
}

/*
 * event_loop_destroy - Destroy event loop
 */
void event_loop_destroy(struct event_loop *loop) {
    if (!loop)
        return;
    
    /* Free events in queue */
    if (loop->queue) {
        struct event *event = loop->queue->head;
        while (event) {
            struct event *next = event->next;
            if (event->data)
                free(event->data);
            free(event);
            event = next;
        }
        free(loop->queue);
    }
    
    /* Free child loops */
    for (uint32_t i = 0; i < loop->child_count; i++) {
        if (loop->children[i])
            event_loop_destroy(loop->children[i]);
    }
    
    free(loop);
}

/*
 * event_loop_add_child - Add child event loop
 */
int event_loop_add_child(struct event_loop *parent, struct event_loop *child) {
    if (!parent || !child)
        return ECHO_EINVAL;
    
    if (parent->child_count >= MAX_CHILD_LOOPS)
        return ECHO_EINVAL;  /* OEIS A000081 limit */
    
    parent->children[parent->child_count++] = child;
    child->parent = parent;
    
    return ECHO_SUCCESS;
}

/*
 * event_post - Post event to queue
 */
int event_post(struct event_loop *loop, event_type_t type,
              prime_t source_prime, void *data, size_t data_size) {
    struct event *event;
    
    if (!loop || !loop->queue)
        return ECHO_EINVAL;
    
    /* Allocate event */
    event = malloc(sizeof(*event));
    if (!event)
        return ECHO_ENOMEM;
    
    event->type = type;
    event->source_prime = source_prime;
    event->data = data;
    event->data_size = data_size;
    event->timestamp_ns = 0;  /* TODO: Get real timestamp */
    event->next = NULL;
    
    /* Add to queue tail */
    if (loop->queue->tail) {
        loop->queue->tail->next = event;
    } else {
        loop->queue->head = event;
    }
    loop->queue->tail = event;
    loop->queue->count++;
    
    return ECHO_SUCCESS;
}

/*
 * event_process - Process one event
 */
int event_process(struct event_loop *loop, struct event *event) {
    if (!loop || !event)
        return ECHO_EINVAL;
    
    switch (event->type) {
    case EVENT_MEMBRANE_EVOLVE:
        /* Evolve associated membrane */
        if (loop->membrane) {
            /* TODO: Call membrane_evolve */
        }
        break;
        
    case EVENT_HYPERGRAPH_PROPAGATE:
        /* Propagate event via hypergraph edges */
        event_propagate(loop, event);
        break;
        
    case EVENT_TIMER_EXPIRE:
        /* Handle timer expiration */
        break;
        
    case EVENT_IO_READY:
        /* Handle I/O ready */
        break;
        
    case EVENT_INTERRUPT:
        /* Handle interrupt */
        break;
    }
    
    loop->events_processed++;
    return ECHO_SUCCESS;
}

/*
 * event_propagate - Propagate event via hypergraph edges
 */
int event_propagate(struct event_loop *loop, struct event *event) {
    if (!loop || !event)
        return ECHO_EINVAL;
    
    /* Propagate to child loops */
    for (uint32_t i = 0; i < loop->child_count; i++) {
        if (loop->children[i]) {
            event_post(loop->children[i], event->type,
                      event->source_prime, event->data, event->data_size);
        }
    }
    
    return ECHO_SUCCESS;
}

/*
 * event_loop_run - Run event loop
 * 
 * Main ENGINE loop. Processes events from queue.
 */
void event_loop_run(struct event_loop *loop) {
    if (!loop)
        return;
    
    loop->running = 1;
    
    while (loop->running) {
        struct event *event;
        
        /* Get next event from queue */
        if (!loop->queue->head) {
            /* No events - idle */
            continue;
        }
        
        event = loop->queue->head;
        loop->queue->head = event->next;
        if (!loop->queue->head)
            loop->queue->tail = NULL;
        loop->queue->count--;
        
        /* Process event */
        event_process(loop, event);
        
        /* Free event */
        if (event->data)
            free(event->data);
        free(event);
        
        loop->iteration_count++;
    }
}

/*
 * event_loop_stop - Stop event loop
 */
void event_loop_stop(struct event_loop *loop) {
    if (loop)
        loop->running = 0;
}

/*
 * event_loop_spawn_children - Spawn child event loops recursively
 * 
 * Follows OEIS A000081 enumeration to determine child count.
 */
int event_loop_spawn_children(struct event_loop *parent, uint32_t depth) {
    extern prime_t nth_prime(size_t n);
    uint32_t child_count;
    
    if (!parent || !parent->hgfs)
        return ECHO_EINVAL;
    
    if (depth >= 8)  /* Max depth limit */
        return ECHO_SUCCESS;
    
    /* Get child count from OEIS A000081 */
    child_count = oeis_value(depth);
    if (child_count > MAX_CHILD_LOOPS)
        child_count = MAX_CHILD_LOOPS;
    
    /* Create child loops */
    for (uint32_t i = 0; i < child_count; i++) {
        struct event_loop *child;
        prime_t child_prime = nth_prime(i + depth * 10);  /* Spread primes */
        
        if (event_loop_init(&child, parent->hgfs, child_prime) != ECHO_SUCCESS)
            return ECHO_ENOMEM;
        
        if (event_loop_add_child(parent, child) != ECHO_SUCCESS) {
            event_loop_destroy(child);
            return ECHO_EINVAL;
        }
        
        /* Recursively spawn grandchildren */
        event_loop_spawn_children(child, depth + 1);
    }
    
    return ECHO_SUCCESS;
}
