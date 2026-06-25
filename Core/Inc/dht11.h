#ifndef __DHT11_H
#define __DHT11_H

#include "main.h"

#define DHT11_OK         0
#define DHT11_TIMEOUT    1
#define DHT11_CHECK_ERR  2

#define DHT11_PIN       GPIO_PIN_1
#define DHT11_PORT      GPIOA

uint8_t DHT11_Read(float *temp, float *humi);

#endif
