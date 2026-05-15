typedef union {

    uint32_t u32[256];


    uint8_t u8[1024];

  
    struct
    {
        u32 magic;       
        u32 version;     
        u32 save_count; 

        s32 oled_direction;
        float Vset;
        float Iset;
        float Pset;
        float vin_slope;
        float vin_offset;
        float vout_slope;
        float iin_slope;
        float iout_slope;
    } cfg;

} flash_param_t;

u8 MCU_Flash_Read_Cfg (void);
FLASH_Status Flash_Save_Cfg (void);

extern flash_param_t flash_data;