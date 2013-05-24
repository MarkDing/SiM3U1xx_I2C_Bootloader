@echo off
cd ..\Host\FlashUtility
call Si32FlashUtility.exe  ..\..\Firmware\DFU_BL_Firmware\release\sim3u1xx_uart_bootloader.hex -v -i -e 2

pause