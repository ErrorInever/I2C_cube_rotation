#ifndef I2C_H_
#define I2C_H_
#include "stm32f446xx.h"
#include <stdint.h>

typedef struct {
    GPIO_TypeDef* port;
    uint16_t pin;
} I2C_t;

extern const I2C_t SDA;
extern const I2C_t SCL;

void I2C_init(void);

#endif
