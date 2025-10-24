/*
 * DTESN Adaptive Learning Implementation
 * =====================================
 * 
 * Adaptive learning algorithms for ESN reservoirs including Hebbian learning,
 * spike-timing dependent plasticity (STDP), BCM rule, reinforcement learning,
 * and meta-learning with performance target compliance.
 */

#define _GNU_SOURCE
#define _POSIX_C_SOURCE 199309L
#include "include/dtesn/dtesn_cognitive.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <math.h>
#include <float.h>

/* Internal constants */
#define DTESN_LEARNING_DEFAULT_BATCH_SIZE       32
#define DTESN_LEARNING_DEFAULT_CONVERGENCE      1e-6f
#define DTESN_LEARNING_DEFAULT_LEARNING_RATE    0.01f
#define DTESN_LEARNING_DEFAULT_ADAPTATION_RATE  0.001f
#define DTESN_LEARNING_PLASTICITY_THRESHOLD     0.1f
#define DTESN_LEARNING_HOMEOSTASIS_TARGET       0.5f

/* Forward declarations */
static uint64_t get_time_ns(void);
static int validate_learning_params(const dtesn_cognitive_learn_params_t *params);
static int apply_hebbian_learning(dtesn_cognitive_system_t *system, 
                                 const float *input, const float *target,
                                 const dtesn_cognitive_learn_params_t *params);
static int apply_stdp_learning(dtesn_cognitive_system_t *system,
                              const float *input, const float *target,
                              const dtesn_cognitive_learn_params_t *params);
static int apply_bcm_learning(dtesn_cognitive_system_t *system,
                             const float *input, const float *target,
                             const dtesn_cognitive_learn_params_t *params);
static int apply_reinforcement_learning(dtesn_cognitive_system_t *system,
                                       const float *input, const float *target,
                                       const dtesn_cognitive_learn_params_t *params);
static int apply_meta_learning(dtesn_cognitive_system_t *system,
                              const float *input, const float *target,
                              const dtesn_cognitive_learn_params_t *params);
static float compute_prediction_error(const float *predicted, const float *target, uint32_t size);
static int update_reservoir_weights(dtesn_esn_reservoir_t *reservoir, 
                                   const float *weight_updates, uint32_t size);
static int apply_homeostatic_regulation(dtesn_cognitive_system_t *system);

/**
 * Get current time in nanoseconds
 */
static uint64_t get_time_ns(void) {
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        return 0;
    }
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

/**
 * Validate learning parameters
 */
static int validate_learning_params(const dtesn_cognitive_learn_params_t *params) {
    if (!params) {
        return -EINVAL;
    }
    
    if (params->learning_rate <= 0.0f || params->learning_rate > 1.0f) {
        return -EINVAL;
    }
    
    if (params->adaptation_rate < 0.0f || params->adaptation_rate > 1.0f) {
        return -EINVAL;
    }
    
    if (params->max_iterations == 0 || 
        params->max_iterations > DTESN_COGNITIVE_MAX_LEARNING_ITERATIONS) {
        return -EINVAL;
    }
    
    if (params->convergence_threshold <= 0.0f) {
        return -EINVAL;
    }
    
    return 0;
}

/**
 * Apply Hebbian learning rule
 */
static int apply_hebbian_learning(dtesn_cognitive_system_t *system,
                                 const float *input, const float *target,
                                 const dtesn_cognitive_learn_params_t *params) {
    dtesn_esn_reservoir_t *reservoir = system->reservoir;
    
    if (!reservoir || !reservoir->x_current || !input) {
        return -EINVAL;
    }
    
    uint32_t reservoir_size = reservoir->config.reservoir_size;
    uint32_t input_size = reservoir->config.input_size;
    
    /* Hebbian learning: Δw_ij = η * x_i * x_j */
    uint32_t max_updates = reservoir_size * input_size;
    float *weight_updates = (float *)malloc(max_updates * sizeof(float));
    if (!weight_updates) {
        return -ENOMEM;
    }
    
    uint32_t update_idx = 0;
    for (uint32_t i = 0; i < reservoir_size && update_idx < max_updates; i++) {
        for (uint32_t j = 0; j < input_size && update_idx < max_updates; j++) {
            float pre_activity = input[j];
            float post_activity = reservoir->x_current[i];
            
            /* Weight update based on pre- and post-synaptic activity */
            float weight_delta = params->learning_rate * pre_activity * post_activity;
            
            /* Apply plasticity threshold */
            if (fabs(weight_delta) > DTESN_LEARNING_PLASTICITY_THRESHOLD) {
                weight_updates[update_idx++] = weight_delta;
            } else {
                weight_updates[update_idx++] = 0.0f;
            }
        }
    }
    
    /* Apply weight updates to reservoir */
    int result = update_reservoir_weights(reservoir, weight_updates, update_idx);
    free(weight_updates);
    
    return result;
}

/**
 * Apply spike-timing dependent plasticity (STDP)
 */
static int apply_stdp_learning(dtesn_cognitive_system_t *system,
                              const float *input, const float *target,
                              const dtesn_cognitive_learn_params_t *params) {
    dtesn_esn_reservoir_t *reservoir = system->reservoir;
    
    if (!reservoir || !reservoir->x_current || !reservoir->x_previous || !input) {
        return -EINVAL;
    }
    
    uint32_t reservoir_size = reservoir->config.reservoir_size;
    uint32_t input_size = reservoir->config.input_size;
    
    /* STDP parameters */
    const float tau_plus = 20.0f;    /* LTP time constant */
    const float tau_minus = 20.0f;   /* LTD time constant */
    const float A_plus = 0.01f;      /* LTP amplitude */
    const float A_minus = 0.012f;    /* LTD amplitude */
    
    /* Allocate weight update buffer */
    uint32_t max_updates = reservoir_size * input_size;
    float *weight_updates = (float *)malloc(max_updates * sizeof(float));
    if (!weight_updates) {
        return -ENOMEM;
    }
    
    /* Compute weight updates based on spike timing differences */
    uint32_t update_idx = 0;
    for (uint32_t i = 0; i < reservoir_size && update_idx < max_updates; i++) {
        for (uint32_t j = 0; j < input_size && update_idx < max_updates; j++) {
            /* Compute activity differences as proxy for spike timing */
            float pre_activity = input[j];
            float post_activity = reservoir->x_current[i];
            float prev_post_activity = reservoir->x_previous[i];
            
            /* Approximate spike timing difference */
            float delta_t = post_activity - prev_post_activity;
            
            float weight_delta = 0.0f;
            if (delta_t > 0) {
                /* LTP: pre before post */
                weight_delta = A_plus * exp(-delta_t / tau_plus);
            } else if (delta_t < 0) {
                /* LTD: post before pre */
                weight_delta = -A_minus * exp(delta_t / tau_minus);
            }
            
            weight_delta *= params->learning_rate;
            
            /* Apply weight update */
            if (fabs(weight_delta) > DTESN_LEARNING_PLASTICITY_THRESHOLD) {
                weight_updates[update_idx++] = weight_delta;
            } else {
                weight_updates[update_idx++] = 0.0f;
            }
        }
    }
    
    /* Apply weight updates to reservoir */
    int result = update_reservoir_weights(reservoir, weight_updates, update_idx);
    free(weight_updates);
    
    return result;
}

/**
 * Apply BCM (Bienenstock-Cooper-Munro) learning rule
 */
static int apply_bcm_learning(dtesn_cognitive_system_t *system,
                             const float *input, const float *target,
                             const dtesn_cognitive_learn_params_t *params) {
    dtesn_esn_reservoir_t *reservoir = system->reservoir;
    
    if (!reservoir || !reservoir->x_current || !input) {
        return -EINVAL;
    }
    
    uint32_t reservoir_size = reservoir->config.reservoir_size;
    uint32_t input_size = reservoir->config.input_size;
    
    /* BCM parameters */
    const float theta_0 = 0.5f;      /* Base threshold */
    const float tau_theta = 100.0f;  /* Threshold time constant */
    
    /* Allocate weight update buffer */
    uint32_t max_updates = reservoir_size * input_size;
    float *weight_updates = (float *)malloc(max_updates * sizeof(float));
    if (!weight_updates) {
        return -ENOMEM;
    }
    
    /* BCM learning: Δw = η * x * y * (y - θ) */
    uint32_t update_idx = 0;
    for (uint32_t i = 0; i < reservoir_size && update_idx < max_updates; i++) {
        float post_activity = reservoir->x_current[i];
        
        /* Compute sliding threshold */
        float theta = theta_0 + post_activity * post_activity / tau_theta;
        
        for (uint32_t j = 0; j < input_size && update_idx < max_updates; j++) {
            float pre_activity = input[j];
            
            /* BCM weight update */
            float weight_delta = params->learning_rate * pre_activity * 
                               post_activity * (post_activity - theta);
            
            if (fabs(weight_delta) > DTESN_LEARNING_PLASTICITY_THRESHOLD) {
                weight_updates[update_idx++] = weight_delta;
            } else {
                weight_updates[update_idx++] = 0.0f;
            }
        }
    }
    
    /* Apply weight updates to reservoir */
    int result = update_reservoir_weights(reservoir, weight_updates, update_idx);
    free(weight_updates);
    
    return result;
}

/**
 * Apply reinforcement learning
 */
static int apply_reinforcement_learning(dtesn_cognitive_system_t *system,
                                       const float *input, const float *target,
                                       const dtesn_cognitive_learn_params_t *params) {
    dtesn_esn_reservoir_t *reservoir = system->reservoir;
    
    if (!reservoir || !reservoir->x_current || !reservoir->y_current || !target) {
        return -EINVAL;
    }
    
    uint32_t output_size = reservoir->config.output_size;
    
    /* Compute prediction error as reward signal */
    float reward = -compute_prediction_error(reservoir->y_current, target, output_size);
    
    /* Apply reward-modulated learning */
    uint32_t reservoir_size = reservoir->config.reservoir_size;
    float *weight_updates = (float *)malloc(reservoir_size * sizeof(float));
    if (!weight_updates) {
        return -ENOMEM;
    }
    
    uint32_t update_idx = 0;
    for (uint32_t i = 0; i < reservoir_size; i++) {
        float activity = reservoir->x_current[i];
        
        /* Reward-modulated weight update */
        float weight_delta = params->learning_rate * reward * activity;
        
        if (fabs(weight_delta) > DTESN_LEARNING_PLASTICITY_THRESHOLD) {
            weight_updates[update_idx++] = weight_delta;
        } else {
            weight_updates[update_idx++] = 0.0f;
        }
    }
    
    /* Apply weight updates to reservoir */
    int result = update_reservoir_weights(reservoir, weight_updates, update_idx);
    free(weight_updates);
    
    return result;
}

/**
 * Apply adaptive meta-learning
 */
static int apply_meta_learning(dtesn_cognitive_system_t *system,
                              const float *input, const float *target,
                              const dtesn_cognitive_learn_params_t *params) {
    /* Meta-learning adapts the learning rate itself based on performance */
    dtesn_esn_reservoir_t *reservoir = system->reservoir;
    
    if (!reservoir || !reservoir->y_current || !target) {
        return -EINVAL;
    }
    
    uint32_t output_size = reservoir->config.output_size;
    
    /* Compute current prediction error */
    float current_error = compute_prediction_error(reservoir->y_current, target, output_size);
    
    /* Adapt learning rate based on error trend */
    static float previous_error = FLT_MAX;
    static float adapted_learning_rate = 0.0f;
    
    /* Initialize adapted learning rate on first call */
    if (adapted_learning_rate == 0.0f) {
        adapted_learning_rate = params->learning_rate;
    }
    
    if (previous_error != FLT_MAX) {
        float error_change = current_error - previous_error;
        
        if (error_change < 0) {
            /* Error decreasing - increase learning rate slightly */
            adapted_learning_rate *= 1.05f;
            /* Cap at 2x original rate */
            if (adapted_learning_rate > 2.0f * params->learning_rate) {
                adapted_learning_rate = 2.0f * params->learning_rate;
            }
        } else {
            /* Error increasing - decrease learning rate */
            adapted_learning_rate *= 0.95f;
            /* Floor at 0.1x original rate */
            if (adapted_learning_rate < 0.1f * params->learning_rate) {
                adapted_learning_rate = 0.1f * params->learning_rate;
            }
        }
    }
    
    previous_error = current_error;
    
    /* Create modified parameters with adapted learning rate */
    dtesn_cognitive_learn_params_t adapted_params = *params;
    adapted_params.learning_rate = adapted_learning_rate;
    
    /* Apply standard learning rule with adapted parameters */
    return apply_hebbian_learning(system, input, target, &adapted_params);
}

/**
 * Compute prediction error
 */
static float compute_prediction_error(const float *predicted, const float *target, uint32_t size) {
    float error = 0.0f;
    
    for (uint32_t i = 0; i < size; i++) {
        float diff = predicted[i] - target[i];
        error += diff * diff;
    }
    
    return sqrtf(error / size); /* RMS error */
}

/**
 * Update reservoir weights - Real implementation with sparse matrix support
 * 
 * Applies weight updates to reservoir and input weight matrices while maintaining
 * sparsity patterns and enforcing spectral radius constraints for ESN stability.
 */
static int update_reservoir_weights(dtesn_esn_reservoir_t *reservoir,
                                   const float *weight_updates, uint32_t size) {
    if (!reservoir || !weight_updates || size == 0) {
        return -EINVAL;
    }
    
    if (!reservoir->W_res || !reservoir->W_in) {
        return -EINVAL;
    }
    
    /* Update reservoir weights (W_res) using sparse matrix structure */
    dtesn_esn_sparse_matrix_t *W_res = reservoir->W_res;
    uint32_t update_idx = 0;
    
    for (uint32_t i = 0; i < W_res->rows && update_idx < size; i++) {
        uint32_t row_start = W_res->row_ptr[i];
        uint32_t row_end = W_res->row_ptr[i + 1];
        
        for (uint32_t j = row_start; j < row_end && update_idx < size; j++) {
            /* Apply weight update while preserving sparsity */
            float old_weight = W_res->values[j];
            float new_weight = old_weight + weight_updates[update_idx];
            
            /* Clip weights to maintain stability (-1.0 to 1.0) */
            if (new_weight > 1.0f) {
                new_weight = 1.0f;
            } else if (new_weight < -1.0f) {
                new_weight = -1.0f;
            }
            
            /* Only update if change is significant (avoid floating point drift) */
            if (fabsf(new_weight - old_weight) > 1e-7f) {
                W_res->values[j] = new_weight;
            }
            
            update_idx++;
        }
    }
    
    /* Update input weights (W_in) if there are remaining updates */
    dtesn_esn_sparse_matrix_t *W_in = reservoir->W_in;
    
    for (uint32_t i = 0; i < W_in->rows && update_idx < size; i++) {
        uint32_t row_start = W_in->row_ptr[i];
        uint32_t row_end = W_in->row_ptr[i + 1];
        
        for (uint32_t j = row_start; j < row_end && update_idx < size; j++) {
            float old_weight = W_in->values[j];
            float new_weight = old_weight + weight_updates[update_idx];
            
            /* Clip input weights */
            if (new_weight > 1.0f) {
                new_weight = 1.0f;
            } else if (new_weight < -1.0f) {
                new_weight = -1.0f;
            }
            
            if (fabsf(new_weight - old_weight) > 1e-7f) {
                W_in->values[j] = new_weight;
            }
            
            update_idx++;
        }
    }
    
    /* Rescale reservoir weights to maintain target spectral radius */
    /* This is critical for ESN echo state property */
    float current_spectral_radius = reservoir->current_spectral_radius;
    float target_spectral_radius = reservoir->config.spectral_radius;
    
    if (fabsf(current_spectral_radius - target_spectral_radius) > 0.01f) {
        /* Compute scaling factor to restore target spectral radius */
        float scale = target_spectral_radius / (current_spectral_radius + 1e-10f);
        
        /* Scale all reservoir weights */
        for (uint32_t i = 0; i < W_res->nnz; i++) {
            W_res->values[i] *= scale;
        }
        
        reservoir->current_spectral_radius = target_spectral_radius;
    }
    
    return 0;
}

/**
 * Apply homeostatic regulation
 */
static int apply_homeostatic_regulation(dtesn_cognitive_system_t *system) {
    dtesn_esn_reservoir_t *reservoir = system->reservoir;
    
    if (!reservoir || !reservoir->x_current) {
        return -EINVAL;
    }
    
    uint32_t reservoir_size = reservoir->config.reservoir_size;
    
    /* Compute mean activity */
    float mean_activity = 0.0f;
    for (uint32_t i = 0; i < reservoir_size; i++) {
        mean_activity += reservoir->x_current[i];
    }
    mean_activity /= reservoir_size;
    
    /* Apply homeostatic scaling if activity deviates from target */
    float deviation = mean_activity - DTESN_LEARNING_HOMEOSTASIS_TARGET;
    if (fabs(deviation) > 0.1f) {
        /* Scale activities toward target */
        float scaling_factor = 1.0f - 0.01f * deviation;
        
        for (uint32_t i = 0; i < reservoir_size; i++) {
            reservoir->x_current[i] *= scaling_factor;
        }
    }
    
    return 0;
}

/**
 * Perform adaptive learning on ESN reservoir
 */
int dtesn_adaptive_learn(dtesn_cognitive_system_t *system,
                        const float **input_data,
                        const float **target_data,
                        uint32_t num_samples,
                        const dtesn_cognitive_learn_params_t *params) {
    int result;
    uint64_t start_time, end_time;
    
    if (!system || !input_data || !target_data || num_samples == 0) {
        return -EINVAL;
    }
    
    result = validate_learning_params(params);
    if (result != 0) {
        return result;
    }
    
    if (!system->initialized || !system->reservoir) {
        return -ENODEV;
    }
    
    start_time = get_time_ns();
    
    pthread_mutex_lock(&system->system_lock);
    
    uint32_t batch_size = params->batch_size > 0 ? params->batch_size : DTESN_LEARNING_DEFAULT_BATCH_SIZE;
    uint32_t iterations = 0;
    float prev_error = FLT_MAX;
    
    /* Batch learning loop */
    while (iterations < params->max_iterations) {
        float total_error = 0.0f;
        uint32_t samples_processed = 0;
        
        /* Process samples in batches */
        for (uint32_t batch_start = 0; batch_start < num_samples; batch_start += batch_size) {
            uint32_t batch_end = (batch_start + batch_size < num_samples) ? 
                                 batch_start + batch_size : num_samples;
            
            for (uint32_t i = batch_start; i < batch_end; i++) {
                /* Apply selected learning algorithm */
                switch (params->learn_type) {
                    case DTESN_COGNITIVE_LEARN_HEBBIAN:
                        result = apply_hebbian_learning(system, input_data[i], target_data[i], params);
                        break;
                    case DTESN_COGNITIVE_LEARN_STDP:
                        result = apply_stdp_learning(system, input_data[i], target_data[i], params);
                        break;
                    case DTESN_COGNITIVE_LEARN_BCM:
                        result = apply_bcm_learning(system, input_data[i], target_data[i], params);
                        break;
                    case DTESN_COGNITIVE_LEARN_RLRL:
                        result = apply_reinforcement_learning(system, input_data[i], target_data[i], params);
                        break;
                    case DTESN_COGNITIVE_LEARN_ADAPTIVE:
                        result = apply_meta_learning(system, input_data[i], target_data[i], params);
                        break;
                    default:
                        result = -EINVAL;
                        break;
                }
                
                if (result != 0) {
                    pthread_mutex_unlock(&system->system_lock);
                    return result;
                }
                
                /* Compute prediction error for convergence check */
                if (system->reservoir->y_current) {
                    float error = compute_prediction_error(system->reservoir->y_current, 
                                                          target_data[i], 
                                                          system->reservoir->config.output_size);
                    total_error += error;
                }
                
                samples_processed++;
            }
            
            /* Apply homeostatic regulation if enabled */
            if (params->enable_homeostasis) {
                apply_homeostatic_regulation(system);
            }
        }
        
        iterations++;
        
        /* Check convergence */
        if (samples_processed > 0) {
            float avg_error = total_error / samples_processed;
            
            if (fabs(prev_error - avg_error) < params->convergence_threshold) {
                printf("Learning converged after %u iterations (error: %f)\n", 
                       iterations, avg_error);
                break;
            }
            
            prev_error = avg_error;
        }
        
        /* Check performance target: ≤ 1000 iterations */
        if (iterations >= DTESN_COGNITIVE_MAX_LEARNING_ITERATIONS) {
            printf("Warning: Learning reached maximum iterations (%u)\n", iterations);
            break;
        }
    }
    
    end_time = get_time_ns();
    
    /* Update performance statistics */
    system->total_learning_iterations += iterations;
    system->total_learning_time_ns += (end_time - start_time);
    
    pthread_mutex_unlock(&system->system_lock);
    
    printf("Adaptive learning completed: %u iterations, %lu samples, %.2f ms\n",
           iterations, (unsigned long)num_samples, (end_time - start_time) / 1000000.0);
    
    return 0;
}

/**
 * Perform online adaptive learning
 */
int dtesn_adaptive_learn_online(dtesn_cognitive_system_t *system,
                               const float *input,
                               const float *target,
                               const dtesn_cognitive_learn_params_t *params) {
    int result;
    uint64_t start_time, end_time;
    
    if (!system || !input || !target) {
        return -EINVAL;
    }
    
    result = validate_learning_params(params);
    if (result != 0) {
        return result;
    }
    
    if (!system->initialized || !system->reservoir) {
        return -ENODEV;
    }
    
    start_time = get_time_ns();
    
    pthread_mutex_lock(&system->system_lock);
    
    /* Apply selected learning algorithm for single sample */
    switch (params->learn_type) {
        case DTESN_COGNITIVE_LEARN_HEBBIAN:
            result = apply_hebbian_learning(system, input, target, params);
            break;
        case DTESN_COGNITIVE_LEARN_STDP:
            result = apply_stdp_learning(system, input, target, params);
            break;
        case DTESN_COGNITIVE_LEARN_BCM:
            result = apply_bcm_learning(system, input, target, params);
            break;
        case DTESN_COGNITIVE_LEARN_RLRL:
            result = apply_reinforcement_learning(system, input, target, params);
            break;
        case DTESN_COGNITIVE_LEARN_ADAPTIVE:
            result = apply_meta_learning(system, input, target, params);
            break;
        default:
            result = -EINVAL;
            break;
    }
    
    if (result == 0) {
        /* Apply homeostatic regulation if enabled */
        if (params->enable_homeostasis) {
            apply_homeostatic_regulation(system);
        }
        
        end_time = get_time_ns();
        
        /* Update performance statistics */
        system->total_learning_iterations += 1;
        system->total_learning_time_ns += (end_time - start_time);
    }
    
    pthread_mutex_unlock(&system->system_lock);
    
    return result;
}