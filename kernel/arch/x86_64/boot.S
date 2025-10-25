/*
 * Echo.Kern x86_64 Bootstrap Entry Point
 * 
 * This is the very first code that runs when the kernel boots.
 * It's called by the bootloader (e.g., GRUB2, UEFI).
 */

.section .text
.global _start
.extern stage0_bootstrap

/*
 * _start - Kernel entry point
 * 
 * Called by bootloader with:
 * - Machine in protected/long mode (depending on bootloader)
 * - No paging set up
 * - Stack pointer invalid
 * - Interrupts disabled
 */
_start:
    /* Disable interrupts (just to be safe) */
    cli
    
    /* Set up temporary stack */
    mov $stack_top, %rsp
    mov %rsp, %rbp
    
    /* Clear direction flag */
    cld
    
    /* Zero all general-purpose registers for clean state */
    xor %rax, %rax
    xor %rbx, %rbx
    xor %rcx, %rcx
    xor %rdx, %rdx
    xor %rsi, %rsi
    xor %rdi, %rdi
    xor %r8, %r8
    xor %r9, %r9
    xor %r10, %r10
    xor %r11, %r11
    xor %r12, %r12
    xor %r13, %r13
    xor %r14, %r14
    xor %r15, %r15
    
    /* Call C bootstrap code */
    call stage0_bootstrap
    
    /* Should never return, but if it does, halt */
halt:
    hlt
    jmp halt

/*
 * Temporary bootstrap stack (16KB)
 */
.section .bss
.align 16
stack_bottom:
    .skip 16384  /* 16KB stack */
stack_top:
