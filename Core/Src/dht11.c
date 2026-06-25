#include "dht11.h"

extern void delay_us(uint32_t us);

static void DHT11_Pin_Output(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT11_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
}

static void DHT11_Pin_Input(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT11_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
}

/**
 * @brief  读取一位数据，关中断保护，防止 TIM1 干扰
 */
static uint8_t DHT11_ReadBit(void)
{
    uint32_t wait;

    /* 等低电平结束（数据位开始） */
    wait = 0;
    while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == RESET)
    {
        if (++wait > 300) return 0;
        delay_us(1);
    }

    /* 等待约 30µs 后再采样 */
    delay_us(30);

    if (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == SET)
    {
        /* 是 "1"：等脉冲结束 */
        wait = 0;
        while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == SET)
        {
            if (++wait > 200) return 1;
            delay_us(1);
        }
        return 1;
    }

    return 0;
}

static uint8_t DHT11_ReadByte(void)
{
    uint8_t byte = 0;
    for (int i = 0; i < 8; i++)
    {
        byte = (byte << 1) | DHT11_ReadBit();
    }
    return byte;
}

uint8_t DHT11_Read(float *temp, float *humi)
{
    uint8_t buf[5] = {0};
    uint32_t wait;
    uint8_t checksum;

    /* 1. 拉低 20ms 触发（用 HAL_Delay，不关中断） */
    DHT11_Pin_Output();
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, RESET);
    HAL_Delay(20);
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, SET);
    delay_us(30);

    /* 2. 切换输入，等 DHT11 响应 */
    DHT11_Pin_Input();

    wait = 0;
    while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == SET)
    {
        if (++wait > 200) return DHT11_TIMEOUT;
        delay_us(1);
    }

    wait = 0;
    while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == RESET)
    {
        if (++wait > 200) return DHT11_TIMEOUT;
        delay_us(1);
    }
    wait = 0;
    while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == SET)
    {
        if (++wait > 200) return DHT11_TIMEOUT;
        delay_us(1);
    }

    /* 3. 关中断，读 40bit 数据（关键时序段） */
    __disable_irq();

    for (int i = 0; i < 5; i++)
    {
        buf[i] = DHT11_ReadByte();
    }

    __enable_irq();

    /* 4. 校验 */
    checksum = buf[0] + buf[1] + buf[2] + buf[3];
    if (checksum != buf[4])
    {
        return DHT11_CHECK_ERR;
    }

    /* 5. 转浮点 */
    *humi = (float)buf[0] + (float)buf[1] / 10.0f;
    *temp = (float)buf[2] + (float)buf[3] / 10.0f;

    /* 恢复输出高电平 */
    DHT11_Pin_Output();
    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, SET);

    return DHT11_OK;
}
