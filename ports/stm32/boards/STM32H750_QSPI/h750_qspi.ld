/*
    Custom linker script for STM32F769DISC board with external memory-mapped QSPI flash

    Memory layout:

    FLASH_QSPI  .text_qspi

    FLASH_APP   .isr_vector
    FLASH_APP   .text
    FLASH_APP   .data

    RAM         .data
    RAM         .bss
    RAM         .heap
    RAM         .stack
*/

MEMORY
{
    FLASH_APP (rx)  : ORIGIN = 0x08008000, LENGTH = 32K   /* after the Mboot */
    FLASH_FS (r)    : ORIGIN = 0x08010000, LENGTH = 64K     /*  */
    FLASH_QSPI (rx) : ORIGIN = 0x90000000, LENGTH = 8M     /* external QSPI flash in XIP mode */
    DTCM (xrw)      : ORIGIN = 0x20000000, LENGTH = 128K    /* Used for storage cache */
    RAM (xrw)       : ORIGIN = 0x20020000, LENGTH = 384K    /* SRAM1 = 368K, SRAM2 = 16K */
}

/* Produce a link error if there is not this amount of RAM for these sections */
_minimum_stack_size = 2K;
_minimum_heap_size = 16K;

/* Define the stack.  The stack is full descending so begins just above last byte
   of RAM.  Note that EABI requires the stack to be 8-byte aligned for a call. */
_estack = ORIGIN(RAM) + LENGTH(RAM) - _estack_reserve;
_sstack = _estack - 32K; /* tunable */

/* RAM extents for the garbage collector */
_ram_start = ORIGIN(RAM);
_ram_end = ORIGIN(RAM) + LENGTH(RAM);
_heap_start = _ebss; /* heap starts just after statically allocated memory */
_heap_end = _sstack;

/* Filesystem cache in RAM, and storage in flash */
_micropy_hw_internal_flash_storage_ram_cache_start = ORIGIN(DTCM);
_micropy_hw_internal_flash_storage_ram_cache_end = ORIGIN(DTCM) + LENGTH(DTCM);
_micropy_hw_internal_flash_storage_start = ORIGIN(FLASH_FS);
_micropy_hw_internal_flash_storage_end = ORIGIN(FLASH_FS) + LENGTH(FLASH_FS);

ENTRY(Reset_Handler)

REGION_ALIAS("FLASH_ISR", FLASH_APP); /* ISR vector list in the internal Flash */
REGION_ALIAS("FLASH_COMMON", FLASH_QSPI); /* code and text in the external qspi flash */

SECTIONS
{
    /* Define the code that goes in QSPI flash, some extend modules */
    .text_qspi :
    {
        . = ALIGN(4);
        *extmod/*(.text* .rodata*)
        . = ALIGN(4);
    } >FLASH_QSPI

    INCLUDE common_isr.ld
    INCLUDE common_text.ld
    INCLUDE common_extratext_data_in_flash.ld
    INCLUDE common_bss_heap_stack.ld
}
