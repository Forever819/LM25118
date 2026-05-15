@echo off
set "HEXFILE=%~1"
set "HEXFILE=%HEXFILE:\=/%"
@REM with verify
@REM openocd -f ./wch-riscv.cfg -c init -c halt -c "flash erase_sector wch_riscv 0 last" -c "program %HEXFILE%" -c "verify_image %HEXFILE%" -c wlink_reset_resume -c exit

@REM without verify, faster
openocd -f ./wch-riscv.cfg ^
-c init ^
-c halt ^
-c "flash erase_sector wch_riscv 0 last" ^
-c "program %HEXFILE%" ^
-c wlink_reset_resume ^
-c exit