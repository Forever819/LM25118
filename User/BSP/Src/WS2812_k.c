#include "debug.h"
#include "WS2812_k.h"
#define SPI_BITS_PER_CODE 7
#define SPI_HIGH_FOR_1 6
#define SPI_HIGH_FOR_0 1

static u8 data2send[30];

void BSP_WS2812_Init (void) {
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    SPI_InitTypeDef SPI_InitStructure = {0};

    RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOC, ENABLE);
    RCC_APB2PeriphClockCmd (RCC_APB2Periph_SPI1, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_Init (GPIOC, &GPIO_InitStructure);


    // ws2812要求单位时间0.25us+-0.15 2.5M~10Mhz
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;  // 6mhz 一个位0.167us
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_Init (SPI1, &SPI_InitStructure);
    SPI_Cmd (SPI1, ENABLE);

    BSP_WS2812_Reset();
    BSP_WS2812_Set_Color (0x00000F);
}

void BSP_WS2812_Reset (void) {
    for (u8 i = 0; i < 50; i++) {
        while (SPI_I2S_GetFlagStatus (SPI1, SPI_I2S_FLAG_TXE) == RESET);
        SPI_I2S_SendData (SPI1, 0x00);
    }
}

static uint32_t Data_proc (uint32_t rgb_data, uint8_t *data2send) {
    uint8_t R = (rgb_data >> 16) & 0xFF;
    uint8_t G = (rgb_data >> 8) & 0xFF;
    uint8_t B = rgb_data & 0xFF;

    uint32_t grb = (G << 16) | (R << 8) | B;
    uint32_t data2send_size = 0;
    uint8_t byte_cnt = 0;
    uint8_t temp = 0x00;

    for (uint8_t bit = 0; bit < 24; bit++) {
        uint8_t high_bits = (grb & 0x800000) ? SPI_HIGH_FOR_1 : SPI_HIGH_FOR_0;

        for (uint8_t k = 0; k < SPI_BITS_PER_CODE; k++) {
            if (k < high_bits)
                temp |= 1;

            if (++byte_cnt == 7) {  // 留一个bit作padding,我也不知道为什么8bit就不行 容易乱闪
                *data2send = temp;
                byte_cnt = 0;
                data2send++;
                temp = 0x00;
                data2send_size++;
            }
            temp <<= 1;
        }
        grb <<= 1;
    }

    // 最后不足1字节的补齐
    if (byte_cnt != 0) {
        temp <<= (8 - byte_cnt);
        *data2send = temp;
        data2send_size++;
    }

    return data2send_size;
}

void BSP_WS2812_Set_Color (u32 color) {
    u16 size = 0;

    size = Data_proc (color, data2send);
    for (u8 i = 0; i < size; i++) {
        while (SPI_I2S_GetFlagStatus (SPI1, SPI_I2S_FLAG_TXE) == RESET);
        SPI_I2S_SendData (SPI1, data2send[i]);
    }
    BSP_WS2812_Reset();
}

// HSV转RGB函数，返回u32格式RGB
static uint32_t HSVtoRGB (float h, float s, float v) {
    float r = 0, g = 0, b = 0;

    int i = (int)(h * 6.0f);
    float f = h * 6.0f - i;
    float p = v * (1.0f - s);
    float q = v * (1.0f - f * s);
    float t = v * (1.0f - (1.0f - f) * s);

    i = i % 6;

    switch (i) {
    case 0:
        r = v;
        g = t;
        b = p;
        break;
    case 1:
        r = q;
        g = v;
        b = p;
        break;
    case 2:
        r = p;
        g = v;
        b = t;
        break;
    case 3:
        r = p;
        g = q;
        b = v;
        break;
    case 4:
        r = t;
        g = p;
        b = v;
        break;
    case 5:
        r = v;
        g = p;
        b = q;
        break;
    }

    uint8_t R = (uint8_t)(r * 255);
    uint8_t G = (uint8_t)(g * 255);
    uint8_t B = (uint8_t)(b * 255);

    return (R << 16) | (G << 8) | B;
}

#define LED_NUM 1    // LED 数量
#define DELAY_MS 20  // 每帧延时


static float hue = 0;

// 彩虹流光主循环
void WS2812_Rainbow (void) {
    uint32_t color = HSVtoRGB (hue, 1.0f, 0.4f);  // 饱和度1，亮度1
    BSP_WS2812_Set_Color (color);                 // 如果是单颗LED，需要改为带索引函数

    // 色相整体滚动
    hue += 0.01f;  // 每帧移动一点
    if (hue > 1.0f)
        hue -= 1.0f;

    Delay_Ms (20);  // 延时
}
