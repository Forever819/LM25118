#include "debug.h"
#include "flash_param.h"
#include "string.h"
#define CFG_ADDRESS 0x0800F000

#define DEBUG_ENABLE

#ifdef DEBUG_ENABLE
#define xprintf(fmt, ...) printf("[%s:%d] " fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define xprintf(...) ((void)0)
#endif

u8 MCU_Flash_Read_Cfg(void)
{
    memcpy(flash_data.u32, (u32 *)CFG_ADDRESS, sizeof(flash_data));
    if (flash_data.cfg.magic != 0x1234AACC) {
        if (flash_data.cfg.save_count == 0xffffffff) {
            xprintf("Dectected first boot!\r\n");
            flash_data.cfg.magic      = 0x1234AACC;
            flash_data.cfg.save_count = 1;
            flash_data.cfg.Vset       = 500;
            flash_data.cfg.Iset       = 500;
            flash_data.cfg.vin_slope  = 0.0135904f;
            flash_data.cfg.vout_slope = 0.0135904f;
            flash_data.cfg.iin_slope  = 0.0172976f;
            flash_data.cfg.iout_slope = 0.0081299f;
            flash_data.cfg.oled_direction = 0;
            Flash_Save_Cfg();
        } else {
            xprintf("Magic Failed!\r\n");
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
        xprintf("erase err!\r\n");
    }
    s = FLASH_ROM_WRITE(CFG_ADDRESS, flash_data.u32, 256);
    if (s != FLASH_COMPLETE) {
        xprintf("write err!\r\n");
    }
    return s;
}