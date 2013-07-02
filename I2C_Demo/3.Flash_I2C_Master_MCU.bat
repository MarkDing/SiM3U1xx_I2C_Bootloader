@echo off
cd ..\Host\FlashUtility
call Si32FlashUtility.exe  ..\..\Firmware\Hexfile\sim3u1xx_i2c_master_mcu.hex -v -i -e 2

pause