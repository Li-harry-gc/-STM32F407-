#include "oled.h"
#include "oled_font.h"
#include "i2c.h"
#include <string.h>
#include <stdio.h>

static uint8_t OLED_Buffer[OLED_WIDTH * OLED_PAGES];

void OLED_WriteCmd(uint8_t cmd)
{
    uint8_t buf[2] = {0x00, cmd};
    HAL_I2C_Master_Transmit(&hi2c1, OLED_ADDR, buf, 2, 100);
}

void OLED_WriteData(uint8_t data)
{
    uint8_t buf[2] = {0x40, data};
    HAL_I2C_Master_Transmit(&hi2c1, OLED_ADDR, buf, 2, 100);
}

void OLED_Init(void)
{
    HAL_Delay(100);

    OLED_WriteCmd(0xAE);
    OLED_WriteCmd(0x20);
    OLED_WriteCmd(0x00);
    OLED_WriteCmd(0xB0);
    OLED_WriteCmd(0xC8);
    OLED_WriteCmd(0x00);
    OLED_WriteCmd(0x10);
    OLED_WriteCmd(0x40);
    OLED_WriteCmd(0x81);
    OLED_WriteCmd(0xFF);
    OLED_WriteCmd(0xA1);
    OLED_WriteCmd(0xA6);
    OLED_WriteCmd(0xA8);
    OLED_WriteCmd(0x3F);
    OLED_WriteCmd(0xA4);
    OLED_WriteCmd(0xD3);
    OLED_WriteCmd(0x00);
    OLED_WriteCmd(0xD5);
    OLED_WriteCmd(0x80);
    OLED_WriteCmd(0xD9);
    OLED_WriteCmd(0xF1);
    OLED_WriteCmd(0xDA);
    OLED_WriteCmd(0x12);
    OLED_WriteCmd(0xDB);
    OLED_WriteCmd(0x40);
    OLED_WriteCmd(0x8D);
    OLED_WriteCmd(0x14);
    OLED_WriteCmd(0xAF);
}

void OLED_Clear(void)
{
    memset(OLED_Buffer, 0x00, sizeof(OLED_Buffer));
}

void OLED_Refresh(void)
{
    for (uint8_t page = 0; page < OLED_PAGES; page++)
    {
        OLED_WriteCmd(0xB0 + page);
        OLED_WriteCmd(0x00);
        OLED_WriteCmd(0x10);
        for (uint8_t col = 0; col < OLED_WIDTH; col++)
        {
            OLED_WriteData(OLED_Buffer[page * OLED_WIDTH + col]);
        }
    }
}

void OLED_ShowChar(uint8_t x, uint8_t y, char ch)
{
    if (x > OLED_WIDTH - 6 || y > OLED_PAGES - 1) return;
    if (ch < ' ' || ch > '~') ch = ' ';

    for (uint8_t i = 0; i < 6; i++)
    {
        OLED_Buffer[y * OLED_WIDTH + x + i] = Font6x8[ch - 32][i];
    }
}

void OLED_ShowString(uint8_t x, uint8_t y, const char *str)
{
    while (*str)
    {
        if (x > OLED_WIDTH - 6)
        {
            x = 0;
            y++;
        }
        OLED_ShowChar(x, y, *str);
        x += 6;
        str++;
    }
}

void OLED_ShowNum(uint8_t x, uint8_t y, float num, uint8_t decimal)
{
    char buf[12];
    if (num < 0)
    {
        OLED_ShowChar(x, y, '-');
        x += 6;
        num = -num;
    }
    sprintf(buf, decimal ? "%.1f" : "%.0f", num);
    OLED_ShowString(x, y, buf);
}

void OLED_Show(void)
{
    extern uint8_t current_mode;
    extern float temperature, humidity, lux;
    extern uint8_t dht11_ok;

    OLED_Clear();

    switch (current_mode)
    {
        case 0: /* MODE_NORMAL */
            if (dht11_ok)
            {
                OLED_ShowString(0, 0, "Temp: ");
                OLED_ShowNum(42, 0, temperature, 1);
                OLED_ShowChar(78, 0, 0xDF);  /* ° 符号 */
                OLED_ShowChar(84, 0, 'C');

                OLED_ShowString(0, 3, "Humi: ");
                OLED_ShowNum(42, 3, humidity, 1);
                OLED_ShowChar(78, 3, '%');

                OLED_ShowString(0, 6, "Lux : ");
                if (lux >= 0)
                    OLED_ShowNum(42, 6, lux, 0);
                else
                    OLED_ShowString(42, 6, "---");
            }
            else
            {
                OLED_ShowString(8, 1, "DHT11 Error!");
                OLED_ShowString(8, 3, "Check Sensor");
                OLED_ShowString(0, 6, "Lux: ");
                if (lux >= 0)
                    OLED_ShowNum(36, 6, lux, 0);
                else
                    OLED_ShowString(36, 6, "---");
            }
            break;

        case 1: /* MODE_TEMP_ONLY */
            if (dht11_ok)
            {
                OLED_ShowString(20, 0, "Temperature");
                OLED_ShowNum(38, 3, temperature, 1);
                OLED_ShowChar(80, 3, 0xDF);
                OLED_ShowChar(86, 3, 'C');
            }
            else
            {
                OLED_ShowString(20, 3, "No Data");
            }
            break;

        case 2: /* MODE_HUMI_ONLY */
            if (dht11_ok)
            {
                OLED_ShowString(28, 0, "Humidity");
                OLED_ShowNum(38, 3, humidity, 1);
                OLED_ShowChar(80, 3, '%');
            }
            else
            {
                OLED_ShowString(20, 3, "No Data");
            }
            break;

        case 3: /* MODE_LUX_ONLY */
            OLED_ShowString(36, 0, "Illum");
            if (lux >= 0)
            {
                OLED_ShowNum(32, 3, lux, 0);
                OLED_ShowString(68, 3, "lux");
            }
            else
            {
                OLED_ShowString(20, 3, "Error");
            }
            break;

        default:
            break;
    }

    OLED_Refresh();
}
