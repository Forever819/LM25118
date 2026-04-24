#include "debug.h"
#include "flash_param.h"
#include "string.h"
#define CFG_ADDRESS 0x0800F000

u8 Flash_Read_Cfg(void)
{
    memcpy(flash_data.u32, (u32 *)CFG_ADDRESS, sizeof(flash_data));
    if (flash_data.cfg.magic != 0x1234AACC) {
        if (flash_data.cfg.save_count == 0xffffffff) {
            printf("Dectected first boot!\r\n");
            flash_data.cfg.magic      = 0x1234AACC;
            flash_data.cfg.save_count = 1;
            flash_data.cfg.Vset       = 500;
            flash_data.cfg.Iset       = 500;
            Flash_Save_Cfg();
        } else {
            printf("Magic Failed!\r\n");
            return 1;
        }
    }
    return 0;
}

FLASH_Status Flash_Save_Cfg(void)
{
    flash_data.cfg.save_count++;
    /* Zero unused padding bytes to avoid writing garbage to flash */
    memset(&flash_data.u8[sizeof(flash_data.cfg)], 0,
           sizeof(flash_data.u8) - sizeof(flash_data.cfg));
    FLASH_Status s = FLASH_ROM_ERASE(CFG_ADDRESS, 256);
    if (s != FLASH_COMPLETE) {
        printf("erase err!\r\n");
    }
    s = FLASH_ROM_WRITE(CFG_ADDRESS, flash_data.u32, 256);
    if (s != FLASH_COMPLETE) {
        printf("write err!\r\n");
    }
    return s;
}