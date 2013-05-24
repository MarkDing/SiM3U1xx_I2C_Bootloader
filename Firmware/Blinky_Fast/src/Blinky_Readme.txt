--------------------------------------------------------------------------------
 Copyright (c) 2012 by Silicon Laboratories. 
 All rights reserved. This program and the accompanying materials
 are made available under the terms of the Silicon Laboratories End User 
 License Agreement which accompanies this distribution, and is available at
 http://developer.silabs.com/legal/version/v10/License_Agreement_v10.htm
 Original content and implementation provided by Silicon Laboratories.
--------------------------------------------------------------------------------

Program Description:
--------------------------------------------------------------------------------

This example code prints "hello world" to the debug viewer and uses the RTC0
module and interrupt to count 10 ms intervals. Main toggles two pins, which 
blinks two LEDs: PB2.10 every 500 ms, and PB2.11 every second. Main also prints 
the sensed logic level of the switches on PB2.8 and PB2.9.

NOTE: This example should not be run on an MCU Card while connected to a UDP
      motherboard. 
 
Resources Used:
--------------------------------------------------------------------------------
RTC0 module
PB0.9  (RTC1 oscillator input)
PB0.10 (RTC2 oscillator input)
PB1.3  (debug printf)
PB2.7 (RTC0 clock)
PB2.8 (switch)
PB2.9 (switch)
PB2.10 (led)
PB2.11 (led)

Notes On Example and Modes:
--------------------------------------------------------------------------------
Blinky (default): 
   AHB 20 MHz
   APB 20 MHz
   RTC0 on and counting, interrupt enabled
   PB1.3, PB2.7, PB2.10, PB2.11 set as digital push-pull outputs
   PB2.8 and PB2.9 set as digital inputs

Off (unused): 
   AHB 20 MHz
   APB 20 MHz
   RTC0 off, interrupt disabled
   All pins set as digital inputs

How to Use:
--------------------------------------------------------------------------------
1) Download the code to a SiM3U1xx device on an SiM3U1xx MCU Card
2) Power the MCU Card board. The easiest way to do this is by connecting a USB
   cable to J13 ("Device USB" connector).
3) In the IDE, open the debug printf viewer and then run the code.
   In uVision, the debug printf viewer can be opened by starting a debug session
   and then clicking View -> Serial Windows -> Debug (printf) Viewer.
   In IAR EWARM, the debug printf viewer can be opened by starting a debug session
   and then clicking View -> Terminal I/O.
4) Set the USER CONTROL switch to ON.
5) Run the code: "hello world" should appear in the debug viewer, PB2.10 and 
   PB2.11 should blink, and PB2.7 will output the RTC0 clock. Additionally, the
   debug viewer will show the sensed logic level of the switches on PB2.8 and 
   PB2.9 every 500 ms.


Revision History:
--------------------------------------------------------------------------------
Release 1.2
    -Removed deprecated RTC_A function calls (ES)
    -31 JUL 12
Release 1.1
    -Updated comments in gRTC0.c (ES)
    -10 APR 12
Release 1.0
    -Initial Revision (TP)
    -26 JAN 12

---eof--------------------------------------------------------------------------

