@echo off
set "HEXFILE=%~1"
set "HEXFILE=%HEXFILE:\=/%"

openocd -f ./wch-riscv.cfg ^
  -c init ^
  -c halt ^
  -c "flash write_image %HEXFILE%" ^
  -c "verify_image %HEXFILE%" ^
  -c wlink_reset_resume ^
  -c exit
