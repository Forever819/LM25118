#include "debug.h"
typedef struct
{
    s16 x;
    s16 y;
    u8 width;
    u8 height;
    u8 digit;
    u8 point_location;
} cursor_area_t;

void OLED_UI_Init (void);
void OLED_Ram_Update(void);
void OLED_UI_OVP (void);
void OLED_UI_OCP (void);
void OLED_UI_SCP (void);

#define CURSOR_AREA_NUM 4

extern cursor_area_t cursor_area[CURSOR_AREA_NUM], *cursor;
extern s32 cursor_index ;  // defualt vset
extern s32 sub_cursor ;
