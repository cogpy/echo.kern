/*
 * GGML Backend Integration for Echo.Kern
 * 
 * Tensor-based memory and computation using GGML.
 * Maps Echo.Kern primitives to GGML tensor operations.
 */

#ifndef _GGML_BACKEND_H
#define _GGML_BACKEND_H

#include "echo_types.h"
#include "hypergraph.h"

/* Forward declarations for GGML types */
struct ggml_context;
struct ggml_tensor;
struct ggml_cgraph;

/* GGML backend configuration */
struct ggml_backend_config {
    size_t mem_size;           /* Memory pool size */
    size_t tensor_alignment;   /* Tensor alignment (64 bytes default) */
    int use_f16;              /* Use FP16 for weights */
    int use_quantized;        /* Use quantized tensors (Q4_K, Q8_0) */
    int num_threads;          /* Thread count for computation */
};

/* GGML context for Echo.Kern */
struct echo_ggml_context {
    struct ggml_context *ctx;        /* GGML context */
    struct ggml_cgraph *graph;       /* Computation graph */
    void *mem_buffer;                /* Memory buffer */
    size_t mem_size;                 /* Memory size */
    
    /* Statistics */
    uint64_t tensor_alloc_count;
    uint64_t tensor_free_count;
    uint64_t compute_count;
};

/* Initialize GGML backend for Echo.Kern */
int ggml_backend_init(struct echo_ggml_context **ggml_out,
                     const struct ggml_backend_config *config);

/* Destroy GGML backend */
void ggml_backend_destroy(struct echo_ggml_context *ggml);

/* Allocate tensor-backed memory (replaces standard malloc) */
void *ggml_backend_alloc(struct echo_ggml_context *ggml,
                        size_t size,
                        prime_t prime_id);

/* Free tensor-backed memory */
void ggml_backend_free(struct echo_ggml_context *ggml, void *ptr);

/* Create GGML tensor from hypergraph node */
struct ggml_tensor *ggml_backend_node_to_tensor(struct echo_ggml_context *ggml,
                                               struct hgfs_node *node);

/* Tensor operations for ESN reservoir */

/**
 * ggml_backend_esn_update - Update ESN reservoir state
 * @ggml: GGML context
 * @state: Current reservoir state tensor (N x 1)
 * @input: Input vector tensor (M x 1)
 * @weights: Weight matrix tensor (N x (N+M))
 * @leak_rate: Leak rate for reservoir
 * 
 * Implements: state_new = (1-leak)*state + leak*tanh(W*[state; input])
 * 
 * Return: New state tensor, or NULL on failure
 */
struct ggml_tensor *ggml_backend_esn_update(struct echo_ggml_context *ggml,
                                           struct ggml_tensor *state,
                                           struct ggml_tensor *input,
                                           struct ggml_tensor *weights,
                                           float leak_rate);

/**
 * ggml_backend_esn_readout - Compute ESN readout
 * @ggml: GGML context
 * @state: Reservoir state tensor
 * @readout_weights: Readout weight matrix
 * 
 * Return: Output tensor
 */
struct ggml_tensor *ggml_backend_esn_readout(struct echo_ggml_context *ggml,
                                            struct ggml_tensor *state,
                                            struct ggml_tensor *readout_weights);

/* Tensor operations for B-Series computation */

/**
 * ggml_backend_bseries_eval - Evaluate B-Series differential
 * @ggml: GGML context
 * @tree_structure: Tree structure tensor (encoded as prime factorization)
 * @coefficient: B-Series coefficient
 * 
 * Return: Differential value tensor
 */
struct ggml_tensor *ggml_backend_bseries_eval(struct echo_ggml_context *ggml,
                                             struct ggml_tensor *tree_structure,
                                             float coefficient);

/* Tensor operations for membrane computing */

/**
 * ggml_backend_membrane_evolve - Evolve membrane state
 * @ggml: GGML context
 * @membrane_state: Current membrane multiset tensor
 * @rules: Evolution rules tensor
 * 
 * Return: New membrane state tensor
 */
struct ggml_tensor *ggml_backend_membrane_evolve(struct echo_ggml_context *ggml,
                                                struct ggml_tensor *membrane_state,
                                                struct ggml_tensor *rules);

/* Tensor operations for probabilistic logic (PLN) */

/**
 * ggml_backend_pln_infer - Probabilistic logic inference
 * @ggml: GGML context
 * @premises: Premise truth values tensor
 * @rule_weights: Inference rule weights tensor
 * 
 * Return: Conclusion truth values tensor
 */
struct ggml_tensor *ggml_backend_pln_infer(struct echo_ggml_context *ggml,
                                          struct ggml_tensor *premises,
                                          struct ggml_tensor *rule_weights);

/* Computation graph operations */

/**
 * ggml_backend_build_graph - Build computation graph
 * @ggml: GGML context
 * 
 * Finalizes the computation graph for execution
 */
int ggml_backend_build_graph(struct echo_ggml_context *ggml);

/**
 * ggml_backend_compute - Execute computation graph
 * @ggml: GGML context
 * @num_threads: Number of threads for parallel execution
 * 
 * Return: 0 on success, negative error code on failure
 */
int ggml_backend_compute(struct echo_ggml_context *ggml, int num_threads);

/* Quantization support */

/**
 * ggml_backend_quantize_tensor - Quantize tensor for efficiency
 * @ggml: GGML context
 * @tensor: Input tensor (FP32)
 * @quant_type: Quantization type (Q4_K, Q8_0, etc.)
 * 
 * Return: Quantized tensor
 */
struct ggml_tensor *ggml_backend_quantize_tensor(struct echo_ggml_context *ggml,
                                                struct ggml_tensor *tensor,
                                                int quant_type);

/* Integration with hypergraph filesystem */

/**
 * hgfs_alloc_tensor - Allocate hypergraph node as GGML tensor
 * @hgfs: Hypergraph filesystem context
 * @ggml: GGML backend context
 * @size: Size in bytes
 * @depth: Membrane depth (OEIS A000081)
 * 
 * Allocates memory via hypergraph and backs it with a GGML tensor
 * for differentiable operations.
 * 
 * Return: Pointer to allocated tensor data
 */
void *hgfs_alloc_tensor(struct hypergraph_fs *hgfs,
                       struct echo_ggml_context *ggml,
                       size_t size,
                       uint32_t depth);

/* Performance monitoring */

/**
 * ggml_backend_get_stats - Get GGML backend statistics
 * @ggml: GGML context
 * @stats_out: Output statistics structure
 */
struct ggml_backend_stats {
    uint64_t tensor_alloc_count;
    uint64_t tensor_free_count;
    uint64_t compute_count;
    uint64_t total_compute_time_ns;
    uint64_t avg_compute_time_ns;
};

int ggml_backend_get_stats(struct echo_ggml_context *ggml,
                          struct ggml_backend_stats *stats_out);

#endif /* _GGML_BACKEND_H */
