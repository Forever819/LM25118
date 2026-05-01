@echo off
set "HEXFILE=%~1"
set "HEXFILE=%HEXFILE:\=/%"
set "PARAM_BIN=param_dump.bin"

del /f /q "%PARAM_BIN%" 2>nul

openocd -f ./wch-riscv.cfg ^
-c init ^
-c halt ^
-c "dump_image %PARAM_BIN% 0x0800F400 0x400" ^
-c "flash erase_sector wch_riscv 0 last" ^
-c "program %HEXFILE%" ^
-c "flash write_image %PARAM_BIN% 0x0000F400" ^
-c wlink_reset_resume ^
-c exit

del /f /q "%PARAM_BIN%" 2>nul