#include "SerialTFT_k.h"
#include "debug.h"
#include "stdarg.h"
#include "string.h"

#define SERIAL_USART USART2
#define TFT_DATA_SIZE 128

TFT_t tft_dev;

static char buf[32], *buf_p;
static DMA_InitTypeDef DMA_InitStructure;
static u8 TFT_Data_Buf[TFT_DATA_SIZE];

void BSP_Display_Init (void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    NVIC_InitTypeDef NVIC_InitStructure;
    RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOD | RCC_APB2Periph_USART2 | RCC_APB2Periph_AFIO, ENABLE);
    RCC_HBPeriphClockCmd (RCC_HBPeriph_DMA1, ENABLE);
    GPIO_PinRemapConfig (GPIO_PartialRemap3_USART2, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init (GPIOD, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init (GPIOD, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

    USART_Init (USART2, &USART_InitStructure);
    USART_Cmd (USART2, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    USART_ITConfig (USART2, USART_IT_IDLE, ENABLE);
    NVIC_Init (&NVIC_InitStructure);

    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART2->DATAR;
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)TFT_Data_Buf;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = TFT_DATA_SIZE;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init (DMA1_Channel7, &DMA_InitStructure);

    USART_DMACmd (USART2, USART_DMAReq_Rx, ENABLE);
    DMA_Cmd (DMA1_Channel7, ENABLE);
    tft_dev.tft_state = TFT_IDLE;
    tft_dev.sys_state = 0;
}

void BSP_TFT_Printf (const char *format, ...) {
    va_list args;
    buf_p = buf;
    va_start (args, format);
    vsnprintf (buf, sizeof (buf), format, args);
    va_end (args);

    // printf("%s\r\n",buf_p);

    while (*buf_p) {
        while (USART_GetFlagStatus (SERIAL_USART, USART_FLAG_TXE) == RESET);
        USART_SendData (SERIAL_USART, *buf_p);
        buf_p++;
    }
    for (uint8_t i = 0; i < 3; i++) {
        while (USART_GetFlagStatus (SERIAL_USART, USART_FLAG_TXE) == RESET);
        USART_SendData (SERIAL_USART, 0xff);
    }
}

void USART2_IRQHandler (void) __attribute__ ((interrupt()));

void USART2_IRQHandler (void) {
    if (USART_GetITStatus (USART2, USART_IT_IDLE) == SET) {
        USART_ReceiveData (USART2);  // clear ITStatus

        // process datapack
        if (TFT_Data_Buf[0] == 0x05 && TFT_Data_Buf[1] == 0x29) {
            if (TFT_Data_Buf[2] == 0xe0) {
            } else if (TFT_Data_Buf[2] == 0xe1) {
                memcpy (&tft_dev.Vset, &TFT_Data_Buf[3], 4);
                memcpy (&tft_dev.Iset, &TFT_Data_Buf[7], 4);
                memcpy (&tft_dev.En_key, &TFT_Data_Buf[15], 4);
                // printf("%d %d %d\r\n",tft_dev.Iset,tft_dev.Vset,tft_dev.En);
            }
        }

        // request next datapack
        memset (TFT_Data_Buf, 0x00, sizeof (TFT_Data_Buf));
        DMA_Init (DMA1_Channel7, &DMA_InitStructure);
    }
}
