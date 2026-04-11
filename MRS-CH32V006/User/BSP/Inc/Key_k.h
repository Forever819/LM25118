// #define KEY_DEBOUNCE_MS 50  // 消抖时间
// #define KEY_DOUBLE_MS 100   // 双击时间窗口
// #define KEY_LONG_MS 500     // 长按阈值

// typedef enum {
//     xKey_Idle = 0,
//     xKey_Pressed,
//     xKey_Released,
//     xKey_DoublePressed,
//     xKey_LongPressed  // 长按会一直触发 flag = 1;
// } xKey_State;

// // 按键事件（由用户轮询获取）
// typedef enum {
//     xKEY_EVENT_NONE = 0,
//     xKEY_EVENT_CLICK,       // 单击
//     xKEY_EVENT_LONG_PRESS,  // 长按（持续触发或单次，见配置）
//     xKEY_EVENT_DOUBLE_PRESS,  // 双击
// } xKey_Event_t;

// typedef struct {
//     GPIO_TypeDef *GPIOX;
//     uint16_t GPIO_Pin;

//     uint16_t cnt;
//     xKey_State state;
//     xKey_Event_t event;

//     u32 press_tick;     // 按下时刻tick
//     u32 long_press_ms;  // 长按判定时间（ms），默认500

// } xKey_t;

// void BSP_Key_Init (void);
// void BSP_Key_Scan (void);
// void BSP_Key_Task (void);
// xKey_Event_t BSP_Key_Get_Event (xKey_t *key); // 主循环轮询

// extern xKey_t Key_Enable;
