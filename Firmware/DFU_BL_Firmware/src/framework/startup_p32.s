
    .syntax unified
    .thumb

//*****************************************************************************
//
// The vector table.
// This relies on the linker script to place at correct location in memory.
//
//*****************************************************************************
    .section .isr_vector
Vectors:
    .word   _vStackTop                      // The initial stack pointer
    .word   ResetISR  			            // The reset handler
    .word   NMI_Handler                     // The NMI handler
    .word   HardFault_Handler               // The hard fault handler
    .word   Default_Handler           	    // The MPU fault handler
    .word   Default_Handler          	    // The bus fault handler
    .word   Default_Handler         	    // The usage fault handler
    .word   0                               // Reserved
    .word   0                               // Reserved
    .word   0                               // Reserved
    .word   0                               // Reserved
    .word   Default_Handler                 // SVCall handler
    .word   Default_Handler                 // Debug monitor handler
    .word   0                               // Reserved
    .word   Default_Handler                 // The PendSV handler
    .word   Default_Handler                 // The SysTick handler

    .text
    .thumb_func
CopyCode2SRAM:
    // Copy the text and data sections from flash to SRAM.
    .extern __data_section_table
    ldr     r3, =__data_section_table
    ldr		r0, [r3], #4 // LOADADDR (.data)
    ldr		r1, [r3], #4 // ADDR (.data)
    ldr 	r2, [r3]     // SIZEOF (.data)
    add 	r2, r1, r2
copy_loop:
    ldr     r3, [r0], #4
    str     r3, [r1], #4
    cmp     r1, r2
    blt     copy_loop

    // Zero fill the bss segment
    movs    r0, #0x00000000
    .extern __bss_section_table
    ldr     r3, =__bss_section_table
    ldr 	r1, [r3], #4 // ADDR (.bss)
    ldr     r2, [r3]	 // SIZEOF (.bss)
    add		r2, r1, r2
zero_loop:
    str     r0, [r1], #4
    cmp     r1, r2
    blt     zero_loop

    bx      lr

//*****************************************************************************
//
// The reset handler, which gets called when the processor starts.
//
//*****************************************************************************
    .globl  ResetISR
    .thumb_func
ResetISR:
	// Copy code from flash to SRAM
    bl      CopyCode2SRAM
    .extern SystemInit
    bl SystemInit

    .extern main
    bl main

    .thumb_func
NMI_Handler:
    b       .

    .thumb_func
HardFault_Handler:
    b       .

    .thumb_func
Default_Handler:
    b       .

    .end
