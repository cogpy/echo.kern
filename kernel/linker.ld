/*
 * Echo.Kern Linker Script
 * 
 * Defines memory layout for kernel image.
 */

OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)

SECTIONS
{
    /* Kernel starts at 1MB physical address */
    . = 0x100000;
    
    /* Text section (code) */
    .text ALIGN(4K) : {
        *(.text*)
    }
    
    /* Read-only data */
    .rodata ALIGN(4K) : {
        *(.rodata*)
    }
    
    /* Initialized data */
    .data ALIGN(4K) : {
        *(.data*)
    }
    
    /* Uninitialized data (BSS) */
    .bss ALIGN(4K) : {
        *(COMMON)
        *(.bss*)
    }
    
    /* Discard unnecessary sections */
    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
        *(.note*)
    }
}
