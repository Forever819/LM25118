typedef union {
    /* 1. 按32位访问 */
    uint32_t u32[256];

    /* 2. 按字节访问 */
    uint8_t u8[1024];

    /* 3. 按结构体访问 */
    struct
    {
        u32 magic;       // 数据标志
        u32 version;     // 配置版本
        u32 save_count;  // 启动次数

        s32 oled_direction;
        s32 Vset;
        s32 Iset;
        s32 Pset;
    } cfg;

} flash_param_t;

u8 Flash_Read_Cfg (void);
FLASH_Status Flash_Save_Cfg (void);

extern flash_param_t flash_data;