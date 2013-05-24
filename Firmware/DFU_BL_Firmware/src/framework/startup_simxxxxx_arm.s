;-------------------------------------------------------------------------------
; Copyright (c) 2013 by Silicon Laboratories Inc.  All rights reserved.
; The program contained in this listing is proprietary to Silicon Laboratories,
; headquartered in Austin, Texas, U.S.A. and is subject to worldwide copyright
; protection, including protection under the United States Copyright Act of 1976
; as an unpublished work, pursuant to Section 104 and Section 408 of Title XVII
; of the United States code.  Unauthorized copying, adaptation, distribution,
; use, or display is prohibited by this law.
;
; Silicon Laboratories provides this software solely and exclusively
; for use on Silicon Laboratories' microcontroller products.
;
; This software is provided "as is".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
; OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
; MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
; SILICON LABORATORIES SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL,
; INCIDENTAL, OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
;-------------------------------------------------------------------------------
; Includes
#include "const.h"

;-------------------------------------------------------------------------------
; RVMDK chain

  PRESERVE8
  THUMB

;-------------------------------------------------------------------------------
; interrupt vectors
  AREA    RESET, DATA, READONLY
  EXPORT __Vectors
  EXPORT __Vectors_End
  EXPORT __Vectors_Size
  IMPORT ||Image$$ARM_LIB_STACK$$ZI$$Limit||
  
__Vectors   
  ; stack pointer
  DCD ||Image$$ARM_LIB_STACK$$ZI$$Limit||   ; Top of Stack

  ; CPU interrupts
  DCD Reset_Handler      ; Reset Handler

#if (MCU_FAMILY == SiM3U1 && COMM_PROTOCOL == USB)
  DCD HardFault_Handler        ; NMI Handler
  DCD HardFault_Handler  ; Hard Fault Handler
  DCD HardFault_Handler  ; MPU Fault Handler
  DCD HardFault_Handler   ; Bus Fault Handler
  DCD HardFault_Handler ; Usage Fault Handler
  DCD 0                  ; reserved
  DCD 0                  ; reserved
  DCD 0                  ; reserved
  DCD 0                  ; reserved
  DCD HardFault_Handler        ; SVCall Handler
  DCD HardFault_Handler   ; Debug Monitor Handler
  DCD 0                  ; reserved
  DCD HardFault_Handler     ; PendSV Handler
  DCD HardFault_Handler    ; SysTick Handler

  ; MCU interrupts
  DCD HardFault_Handler     ; Watchdog Timer 0
  DCD HardFault_Handler       ; External interrupt 0
  DCD HardFault_Handler       ; External interrupt 1
  DCD HardFault_Handler     ; RTC Alarm
  DCD HardFault_Handler       ; DMA Channel 0
  DCD HardFault_Handler       ; DMA Channel 1
  DCD HardFault_Handler       ; DMA Channel 2
  DCD HardFault_Handler       ; DMA Channel 3
  DCD HardFault_Handler       ; DMA Channel 4
  DCD HardFault_Handler       ; DMA Channel 5
  DCD HardFault_Handler       ; DMA Channel 6
  DCD HardFault_Handler       ; DMA Channel 7
  DCD HardFault_Handler       ; DMA Channel 8
  DCD HardFault_Handler       ; DMA Channel 9
  DCD HardFault_Handler      ; DMA Channel 10
  DCD HardFault_Handler      ; DMA Channel 11
  DCD HardFault_Handler      ; DMA Channel 12
  DCD HardFault_Handler      ; DMA Channel 13
  DCD HardFault_Handler      ; DMA Channel 14
  DCD HardFault_Handler      ; DMA Channel 15
  DCD HardFault_Handler      ; Timer 0 Low (16 bits only)
  DCD HardFault_Handler      ; Timer 0 High (16 or 32 bits)
  DCD HardFault_Handler      ; Timer 1 Low (16 bits only)
  DCD HardFault_Handler      ; Timer 1 High (16 or 32 bits)
  DCD HardFault_Handler        ; EPCA 0 Capture Compare (any channel)
  DCD HardFault_Handler         ; PCA 0 Capture Compare (any channel)
  DCD HardFault_Handler         ; PCA 1 Capture Compare (any channel)
  DCD HardFault_Handler       ; USART 0
  DCD HardFault_Handler       ; USART 1
  DCD HardFault_Handler         ; SPI 0
  DCD HardFault_Handler         ; SPI 1
  DCD HardFault_Handler         ; SPI 2
  DCD HardFault_Handler         ; I2C 0
  DCD HardFault_Handler         ; I2C 1
  DCD USB0_IRQHandler         ; USB 0
#endif
__Vectors_End

__Vectors_Size\
  EQU  __Vectors_End - __Vectors
  AREA    |.text|, CODE, READONLY

;-------------------------------------------------------------------------------
; Reset Handler (Optimized for code size)
Reset_Handler\
  PROC
    EXPORT Reset_Handler [WEAK]
        IMPORT __main
	 ;IMPORT main
        LDR R0, = __main
	 ;LDR R0, = main
    BX  R0
  ENDP

;-------------------------------------------------------------------------------
; Dummy exception handlers

#if (MCU_FAMILY == SiM3U1 && COMM_PROTOCOL == USB)

HardFault_Handler\
  PROC
    EXPORT HardFault_Handler [WEAK]
    B .
  ENDP

USB0_IRQHandler\
  PROC
    EXPORT USB0_IRQHandler [WEAK]
    B .
  ENDP

#endif


;-------------------------------------------------------------------------------
; Alignment

  ALIGN 
      
  END

;---eof-------------------------------------------------------------------------
