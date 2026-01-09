#define KEY_DEBOUNCE_MS 50  // 消抖时间
#define KEY_DOUBLE_MS 100   // 双击时间窗口
#define KEY_LONG_MS 500     // 长按阈值

typedef enum {
    Key_Idle = 0,
    Key_Pressed,
    Key_Released,
    Key_DoublePressed,
    Key_LongPressed  // 长按会一直触发 flag = 1;
} Key_State;

typedef struct {
    GPIO_TypeDef *GPIOX;
    uint16_t GPIO_Pin;

    uint16_t cnt;
    Key_State state;

    volatile uint8_t pressed_flag;
    volatile uint8_t double_pressed_flag;
    volatile uint8_t long_pressed_flag;
} Key_t;

void BSP_Key_Init (void);
void BSP_Key_Scan (void);
void BSP_Key_Task (void);

extern Key_t Key_Encoder;
extern Key_t Key_Enable;
