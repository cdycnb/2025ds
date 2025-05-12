#include "stm32f10x.h"
#include "stm32f10x_dac.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_tim.h"
#include "oled.h"  // 添加OLED库

#define FILTER_COEFFICIENT 0.1  // 滤波 器系数，可以根据需要调整
#define WAVE_DATA_LENGTH 128    // 波形数据长度

uint16_t filteredWaveData[WAVE_DATA_LENGTH];  // 存储滤波后的波形数据
uint16_t waveData[WAVE_DATA_LENGTH];  // 波形数据
int dataIndex = 0;
volatile uint8_t dataReady = 0;  // 数据接收完成标志
volatile uint8_t dacOutputIndex = 0;  // DAC输出索引

DAC_InitTypeDef DAC_InitStruct;
USART_InitTypeDef USART_InitStruct;
TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

void delay_ms(uint32_t ms) {
    SysTick_Config(SystemCoreClock / 1000);
    for (uint32_t i = 0; i < ms; i++) {
        while (!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG));
    }
    SysTick->CTRL = 0;
}
// LED初始化函数
void LED_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

// 点亮LED
void LED_On(void) {
    GPIO_ResetBits(GPIOA, GPIO_Pin_1);
}

// 熄灭LED
void LED_Off(void) {
    GPIO_SetBits(GPIOA, GPIO_Pin_1);
}

// USART1配置函数
void USART1_Config(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    // 配置TX引脚
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 配置RX引脚
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 配置USART参数
    USART_InitStruct.USART_BaudRate = 9600;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStruct);
    USART_Cmd(USART1, ENABLE);
}

// USART1中断处理函数
void USART1_IRQHandler(void) {
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        uint8_t byte = USART_ReceiveData(USART1);
        uint16_t rawValue = (uint16_t)byte;

        // 应用一阶IIR低通滤波器
        if (dataIndex == 0) {
            filteredWaveData[dataIndex] = rawValue;  // 第一个数据点直接赋值
        } else {
            filteredWaveData[dataIndex] = FILTER_COEFFICIENT * rawValue + (1 - FILTER_COEFFICIENT) * filteredWaveData[dataIndex - 1];
        }

        waveData[dataIndex] = filteredWaveData[dataIndex];  // 使用滤波后的数据
        dataIndex = (dataIndex + 1) % WAVE_DATA_LENGTH;  // 循环更新数据索引
        dataReady = 1;  // 标记有新数据
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}

// 将波形数据绘制在OLED上
void DrawWaveformOnOLED(uint16_t *waveData, int length) {
    OLED_Clear();  // 清除屏幕
    for (int i = 0; i < length; i++) {
        OLED_DrawPoint(i % 128, waveData[i] % 64);  // 绘制点，简单映射到屏幕大小
    }
    OLED_Refresh();  // 更新显示
}

// 定时器初始化函数
void TIM3_Init(void) {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    TIM_TimeBaseStructure.TIM_Period = 1000;  // 定时器周期
    TIM_TimeBaseStructure.TIM_Prescaler = 72 - 1;  // 预分频器
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM3, ENABLE);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

// 定时器中断处理函数
void TIM3_IRQHandler(void) {
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) {
        if (dataReady) {
            DAC_SetChannel1Data(DAC_Align_8b_R, waveData[dacOutputIndex]);
            dacOutputIndex = (dacOutputIndex + 1) % WAVE_DATA_LENGTH;
        }
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    }
}

int main(void) {
    // 初始化系统时钟
    //SystemInit();
    LED_Init();
    GPIO_SetBits(GPIOA, GPIO_Pin_1);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);

    // 初始化DAC
    DAC_InitStruct.DAC_Trigger = DAC_Trigger_None;
    DAC_InitStruct.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
    DAC_Init(DAC_Channel_1, &DAC_InitStruct);
    DAC_Cmd(DAC_Channel_1, ENABLE);

    // 初始化USART1
    USART1_Config();

    // 配置USART1中断
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    // 初始化OLED
    OLED_Init();

    // 初始化定时器
    TIM3_Init();

    while (1) {
        if (dataReady) {
            // 将波形数据显示在OLED上
            DrawWaveformOnOLED(waveData, WAVE_DATA_LENGTH);
            dataReady = 0;
            delay_ms(1);
        }
    }
}