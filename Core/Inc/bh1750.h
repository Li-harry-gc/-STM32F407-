#ifndef __BH1750_H
#define __BH1750_H

#include "main.h"

#define BH1750_ADDR         (0x23 << 1)

#define BH1750_POWER_ON     0x01
#define BH1750_RESET        0x07
#define BH1750_CONT_H_RES   0x10

extern I2C_HandleTypeDef hi2c1;

void    BH1750_Init(void);
float   BH1750_ReadLux(void);

#endif
