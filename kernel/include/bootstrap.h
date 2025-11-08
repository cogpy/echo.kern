/*
 * Echo.Kern Bootstrap Chain
 * 
 * Stage0 → Stage1 → Stage2 → Stage3
 * Each stage follows OEIS A000081 enumeration.
 */

#ifndef _BOOTSTRAP_H
#define _BOOTSTRAP_H

#include "echo_types.h"
#include "hypergraph.h"
#include "membrane.h"
#include "event_loop.h"
#include "ggml_backend.h"

/* Stage0: Firmware Bootstrap (Level -3) */
struct stage0_bootstrap {
    uint64_t magic;                    /* 0xECH0A00081 */
    uint8_t attestation[32];           /* SHA-256 of Stage1 */
    void (*stage1_entry)(void);        /* Jump to Stage1 */
    prime_t prime_index;               /* First prime (2) */
} __attribute__((packed));

/* Stage1: Hypervisor Bootstrap (Level -1) */
struct stage1_bootstrap {
    struct membrane_topology *root;    /* Root membrane (prime 2) */
    struct hypergraph_fs *hgfs;       /* Prime power series filesystem */
    struct echo_ggml_context *ggml;   /* GGML tensor backend */
    struct event_loop *engine_loop;   /* Master event loop */
    uint32_t context_count;           /* 4 contexts (OEIS A000081) */
};

/* Stage2: Kernel Partitions (Level 0) */
struct stage2_bootstrap {
    struct dtesn_partition {
        prime_t prime_id;
        struct membrane *partition_membrane;
        void *esn_reservoir;
        void *bseries_engine;
        echo_security_level_t security_level;
    } partitions[9];  /* 9 partitions (OEIS) */
    struct esn_reservoir *reservoirs;
    struct bseries_engine *differential_engines;
};

/* Stage3: User Space (Levels +1, +2, +3) */
struct stage3_bootstrap {
    uint32_t service_partitions;      /* 20 (OEIS) */
    uint32_t app_containers;          /* 48 (OEIS) */
    uint32_t thread_contexts;         /* 115 (OEIS) */
};

/* DTESN partition access helpers */
#define dtesn_partition struct dtesn_partition

/* Bootstrap functions */

/* Stage0: Entry point from firmware */
void stage0_bootstrap(void) __attribute__((noreturn));

/* Stage0: Verify Stage1 attestation */
int stage0_attestation_verify(void *stage1_addr, const uint8_t *expected_hash);

/* Stage0: Jump to Stage1 */
void stage0_jump_to_stage1(void (*stage1_entry)(void)) __attribute__((noreturn));

/* Stage1: Initialize membrane hierarchy */
int stage1_init_membranes(struct stage1_bootstrap *stage1);

/* Stage1: Initialize GGML tensor backend */
int stage1_init_ggml_backend(struct stage1_bootstrap *stage1);

/* Stage1: Initialize hypergraph filesystem */
int stage1_init_hypergraph_fs(struct stage1_bootstrap *stage1);

/* Stage1: Spawn kernel partitions */
int stage1_spawn_kernel_partitions(struct stage1_bootstrap *stage1,
                                   struct stage2_bootstrap *stage2);

/* Stage2: Load DTESN core */
int stage2_load_dtesn_core(struct stage2_bootstrap *stage2);

/* Stage2: Initialize ESN reservoirs */
int stage2_init_esn_reservoirs(struct stage2_bootstrap *stage2);

/* Stage2: Start B-Series engines */
int stage2_start_bseries_engines(struct stage2_bootstrap *stage2);

/* Stage3: Load services */
int stage3_load_services(struct stage3_bootstrap *stage3);

/* Stage3: Spawn applications */
int stage3_spawn_applications(struct stage3_bootstrap *stage3);

/* Stage3: Initialize threads */
int stage3_init_threads(struct stage3_bootstrap *stage3);

#endif /* _BOOTSTRAP_H */
