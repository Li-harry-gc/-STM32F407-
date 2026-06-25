#ifndef __OLED_H
#define __OLED_H

#include "main.h"

#define OLED_ADDR       (0x3C << 1)

#define OLED_WIDTH      128
#define OLED_HEIGHT     64
#define OLED_PAGES      8

void OLED_Init(void);
void OLED_Clear(void);
void OLED_Refresh(void);
void OLED_ShowChar(uint8_t x, uint8_t y, char ch);
void OLED_ShowString(uint8_t x, uint8_t y, const char *str);
void OLED_ShowNum(uint8_t x, uint8_t y, float num, uint8_t decimal);
void OLED_Show(void);

void OLED_WriteCmd(uint8_t cmd);
void OLED_WriteData(uint8_t data);

#endif
