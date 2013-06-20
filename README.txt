SiM3U1xxx I2C Bootloader

* It contains four parts.
** Bootloader firmware
** Master MCU firmware
** PC data source software.
** User application code.

* It supports both Keil and Precesion32 IDE 

* Introduction:

  Please refer from AN762 and AN763 for reference. I2C bootloader solution add master MCU which 
act as bridge. Connect Master MCU and PC through UART. connect Master MCU and Target MCU through
I2C bus. 

* Build master and bootloader firmware with release mode configuration.
  In Precision32, debug mode code size exceed 8K bytes which need modify user application code 
size to 16K bytes to fit this range.

* For user applicatoin code entry address. 
** In master and bootloader firmware, main.c
void user_app_jump(void)
{
    __asm volatile ("ldr r0, =0x00002000");
    __asm volatile ("ldr sp, [r0]");
    __asm volatile ("ldr pc, [r0, #4] ");
}

** In master and bootloader firmware, userconfig.h
#define USER_APP_START_ADDR 0x00002000


* I2C PIN assignment
Master MCU:
P0.0   --- SDA
P0.2   --- SCL

Target MCU:
P0.14  --- SDA
P0.15  --- SCL

* Running.
Execute "SiM3U1xx_I2C_Bootloader\I2C_Demo\3.Launch_DFU_Utility.bat" to burn user application into 
target MCU board. 