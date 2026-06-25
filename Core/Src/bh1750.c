#include "bh1750.h"

void BH1750_Init(void)
{
    uint8_t cmd;

    HAL_Delay(10);

    cmd = BH1750_POWER_ON;
    HAL_I2C_Master_Transmit(&hi2c1, BH1750_ADDR, &cmd, 1, 100);

    cmd = BH1750_RESET;
    HAL_I2C_Master_Transmit(&hi2c1, BH1750_ADDR, &cmd, 1, 100);

    cmd = BH1750_CONT_H_RES;
    HAL_I2C_Master_Transmit(&hi2c1, BH1750_ADDR, &cmd, 1, 100);

    HAL_Delay(180);
}

float BH1750_ReadLux(void)
{
    uint8_t buf[2] = {0};
    uint16_t raw;

    if (HAL_I2C_Master_Receive(&hi2c1, BH1750_ADDR, buf, 2, 200) != HAL_OK)
    {
        return -1.0f;
    }

    raw = ((uint16_t)buf[0] << 8) | buf[1];
    return (float)raw / 1.2f;
}
