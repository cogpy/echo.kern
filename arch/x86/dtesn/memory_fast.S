/*
 * DTESN x86_64 Memory Management Fast Paths
 * =========================================
 * 
 * Optimized assembly for memory management operations:
 * - OEIS A000081-compliant allocation
 * - Fast memory copy for DTESN structures
 * - Cache-optimized memory access patterns
 * - NUMA-aware allocation hints
 * 
 * Performance targets:
 * - Allocation: ≤ 100ns for small structures
 * - Copy: 40GB/s memory bandwidth utilization
 * - Validation: ≤ 20ns for OEIS compliance check
 */

#include <linux/linkage.h>

.text

/*
 * dtesn_memcpy_fast - Optimized memory copy for DTESN structures
 * 
 * Input:
 *   %rdi = void *dest
 *   %rsi = const void *src
 *   %rdx = size_t size
 * 
 * Returns:
 *   %rax = dest
 * 
 * Optimized for common DTESN structure sizes (64, 128, 256, 512 bytes).
 * Uses AVX2 for cache-line-sized transfers.
 */
SYM_FUNC_START(dtesn_memcpy_fast)
    movq    %rdi, %rax              /* Return dest */
    
    /* Check size and use optimized path */
    cmpq    $512, %rdx
    je      .Lcopy_512
    cmpq    $256, %rdx
    je      .Lcopy_256
    cmpq    $128, %rdx
    je      .Lcopy_128
    cmpq    $64, %rdx
    je      .Lcopy_64
    cmpq    $32, %rdx
    je      .Lcopy_32
    
    /* Generic copy for other sizes */
    movq    %rdx, %rcx
    rep movsb
    ret
    
.Lcopy_512:
    /* 512-byte copy with AVX2 (16 × 32-byte loads/stores) */
    vmovdqu (%rsi), %ymm0
    vmovdqu 32(%rsi), %ymm1
    vmovdqu 64(%rsi), %ymm2
    vmovdqu 96(%rsi), %ymm3
    vmovdqu 128(%rsi), %ymm4
    vmovdqu 160(%rsi), %ymm5
    vmovdqu 192(%rsi), %ymm6
    vmovdqu 224(%rsi), %ymm7
    
    vmovdqu %ymm0, (%rdi)
    vmovdqu %ymm1, 32(%rdi)
    vmovdqu %ymm2, 64(%rdi)
    vmovdqu %ymm3, 96(%rdi)
    vmovdqu %ymm4, 128(%rdi)
    vmovdqu %ymm5, 160(%rdi)
    vmovdqu %ymm6, 192(%rdi)
    vmovdqu %ymm7, 224(%rdi)
    
    vmovdqu 256(%rsi), %ymm0
    vmovdqu 288(%rsi), %ymm1
    vmovdqu 320(%rsi), %ymm2
    vmovdqu 352(%rsi), %ymm3
    vmovdqu 384(%rsi), %ymm4
    vmovdqu 416(%rsi), %ymm5
    vmovdqu 448(%rsi), %ymm6
    vmovdqu 480(%rsi), %ymm7
    
    vmovdqu %ymm0, 256(%rdi)
    vmovdqu %ymm1, 288(%rdi)
    vmovdqu %ymm2, 320(%rdi)
    vmovdqu %ymm3, 352(%rdi)
    vmovdqu %ymm4, 384(%rdi)
    vmovdqu %ymm5, 416(%rdi)
    vmovdqu %ymm6, 448(%rdi)
    vmovdqu %ymm7, 480(%rdi)
    
    vzeroupper
    ret
    
.Lcopy_256:
    /* 256-byte copy (8 × 32-byte) */
    vmovdqu (%rsi), %ymm0
    vmovdqu 32(%rsi), %ymm1
    vmovdqu 64(%rsi), %ymm2
    vmovdqu 96(%rsi), %ymm3
    vmovdqu 128(%rsi), %ymm4
    vmovdqu 160(%rsi), %ymm5
    vmovdqu 192(%rsi), %ymm6
    vmovdqu 224(%rsi), %ymm7
    
    vmovdqu %ymm0, (%rdi)
    vmovdqu %ymm1, 32(%rdi)
    vmovdqu %ymm2, 64(%rdi)
    vmovdqu %ymm3, 96(%rdi)
    vmovdqu %ymm4, 128(%rdi)
    vmovdqu %ymm5, 160(%rdi)
    vmovdqu %ymm6, 192(%rdi)
    vmovdqu %ymm7, 224(%rdi)
    
    vzeroupper
    ret
    
.Lcopy_128:
    /* 128-byte copy (4 × 32-byte) */
    vmovdqu (%rsi), %ymm0
    vmovdqu 32(%rsi), %ymm1
    vmovdqu 64(%rsi), %ymm2
    vmovdqu 96(%rsi), %ymm3
    
    vmovdqu %ymm0, (%rdi)
    vmovdqu %ymm1, 32(%rdi)
    vmovdqu %ymm2, 64(%rdi)
    vmovdqu %ymm3, 96(%rdi)
    
    vzeroupper
    ret
    
.Lcopy_64:
    /* 64-byte copy (2 × 32-byte) */
    vmovdqu (%rsi), %ymm0
    vmovdqu 32(%rsi), %ymm1
    
    vmovdqu %ymm0, (%rdi)
    vmovdqu %ymm1, 32(%rdi)
    
    vzeroupper
    ret
    
.Lcopy_32:
    /* 32-byte copy */
    vmovdqu (%rsi), %ymm0
    vmovdqu %ymm0, (%rdi)
    
    vzeroupper
    ret
SYM_FUNC_END(dtesn_memcpy_fast)

/*
 * dtesn_alloc_validate_oeis - Validate allocation against OEIS A000081
 * 
 * Input:
 *   %edi = uint32_t depth_level (0-13)
 *   %esi = uint32_t current_count (existing allocations at this depth)
 * 
 * Returns:
 *   %rax = 1 if allocation allowed, 0 if would violate OEIS
 * 
 * Ultra-fast validation using direct table lookup.
 */
SYM_FUNC_START(dtesn_alloc_validate_oeis)
    /* Bounds check on depth */
    cmpl    $14, %edi
    jae     .Lvalidate_fail
    
    /* Load maximum count from OEIS table */
    leaq    .Loeis_limits(%rip), %rax
    movl    (%rax, %rdi, 4), %eax
    
    /* Check if current_count + 1 <= max */
    leal    1(%rsi), %edx
    cmpl    %edx, %eax
    jb      .Lvalidate_fail
    
    /* Allowed */
    movl    $1, %eax
    ret
    
.Lvalidate_fail:
    xorl    %eax, %eax
    ret
SYM_FUNC_END(dtesn_alloc_validate_oeis)

/*
 * dtesn_prefetch_structure - Prefetch DTESN structure into cache
 * 
 * Input:
 *   %rdi = const void *ptr (structure to prefetch)
 *   %esi = uint32_t size (structure size)
 * 
 * Prefetches structure into L1 cache with temporal locality hint.
 */
SYM_FUNC_START(dtesn_prefetch_structure)
    /* Prefetch in 64-byte cache line chunks */
    movl    %esi, %eax
    shrl    $6, %eax                /* num_lines = size / 64 */
    incl    %eax                    /* Round up */
    
    xorl    %ecx, %ecx
    
.Lprefetch_loop:
    prefetcht0 (%rdi, %rcx, 64)
    incl    %ecx
    cmpl    %ecx, %eax
    ja      .Lprefetch_loop
    
    ret
SYM_FUNC_END(dtesn_prefetch_structure)

/*
 * dtesn_memzero_fast - Fast zero-fill for DTESN structures
 * 
 * Input:
 *   %rdi = void *dest
 *   %rsi = size_t size
 * 
 * Returns:
 *   %rax = dest
 * 
 * Uses AVX2 for vectorized zeroing.
 */
SYM_FUNC_START(dtesn_memzero_fast)
    movq    %rdi, %rax              /* Return dest */
    
    /* Zero YMM register */
    vpxor   %ymm0, %ymm0, %ymm0
    
    /* Process 32-byte chunks */
    movq    %rsi, %rcx
    shrq    $5, %rcx                /* num_chunks = size / 32 */
    
    xorl    %edx, %edx
    
.Lzero_loop_32:
    testq   %rcx, %rcx
    jz      .Lzero_remainder
    
    vmovdqu %ymm0, (%rdi, %rdx, 32)
    
    incq    %rdx
    decq    %rcx
    jmp     .Lzero_loop_32
    
.Lzero_remainder:
    /* Handle remaining bytes */
    movq    %rsi, %rcx
    andq    $31, %rcx
    jz      .Lzero_done
    
    shlq    $5, %rdx                /* Convert chunk count to bytes */
    addq    %rdx, %rdi
    rep stosb
    
.Lzero_done:
    vzeroupper
    ret
SYM_FUNC_END(dtesn_memzero_fast)

/*
 * dtesn_cache_flush - Flush DTESN structure from cache
 * 
 * Input:
 *   %rdi = const void *ptr
 *   %esi = uint32_t size
 * 
 * Flushes cache lines to ensure memory coherency.
 */
SYM_FUNC_START(dtesn_cache_flush)
    movl    %esi, %eax
    shrl    $6, %eax                /* num_lines = size / 64 */
    incl    %eax
    
    xorl    %ecx, %ecx
    
.Lflush_loop:
    clflush (%rdi, %rcx, 64)
    incl    %ecx
    cmpl    %ecx, %eax
    ja      .Lflush_loop
    
    mfence                          /* Memory barrier */
    ret
SYM_FUNC_END(dtesn_cache_flush)

/*
 * Constants
 */
.section .rodata

.align 4
.Loeis_limits:
    /* OEIS A000081 maximum counts by depth */
    .long   1       /* depth 0: 1 */
    .long   1       /* depth 1: 1 */
    .long   2       /* depth 2: 2 */
    .long   4       /* depth 3: 4 */
    .long   9       /* depth 4: 9 */
    .long   20      /* depth 5: 20 */
    .long   48      /* depth 6: 48 */
    .long   115     /* depth 7: 115 */
    .long   286     /* depth 8: 286 */
    .long   719     /* depth 9: 719 */
    .long   1842    /* depth 10: 1842 */
    .long   4766    /* depth 11: 4766 */
    .long   12486   /* depth 12: 12486 */
    .long   32973   /* depth 13: 32973 */

/*
 * Export symbols
 */
.section ".export_symbols","a"
.balign 8
.quad dtesn_memcpy_fast
.quad dtesn_alloc_validate_oeis
.quad dtesn_prefetch_structure
.quad dtesn_memzero_fast
.quad dtesn_cache_flush
