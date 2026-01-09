#include "stdint.h"
void BSP_Display_Init (void);
void BSP_Display_CMD (char *string);
void BSP_Display_Loop (void);
void BSP_TFT_Printf (const char *format, ...);

typedef enum {
    TFT_OFF_SW = 0,
    TFT_CC_SW,
    TFT_CV_SW,
    TFT_IDLE,
    TFT_CC,
    TFT_CV,
    TFT_OTA,
    TFT_TOUCHING,
} BSP_TFT_State_e;

typedef struct {
    BSP_TFT_State_e tft_state;
    uint32_t Iset;
    uint32_t Vset;
    uint32_t En_key;
    uint8_t sys_state;
} TFT_t;

extern TFT_t tft_dev;
