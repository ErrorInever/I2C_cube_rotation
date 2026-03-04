#include "main.h"
#include "i2c.h"
#include "stm32f446xx.h"
#include "stm32f4xx_hal_gpio.h"

// SDA - PB9. SCL - PB8

const I2C_t SDA = {GPIOB, GPIO_PIN_9};
const I2C_t SCL = {GPIOB, GPIO_PIN_8};

void I2C_init(void) {
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
    (void)RCC->APB1ENR;
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    (void)RCC->AHB1ENR;
    // open drain
    SDA.port->OTYPER &= ~GPIO_OTYPER_OT8_Msk;
    SDA.port->OTYPER |= GPIO_OTYPER_OT8;
    SCL.port->OTYPER &= ~GPIO_OTYPER_OT9_Msk;
    SCL.port->OTYPER |= GPIO_OTYPER_OT9;
    // setup AF4 for SDA and SCL
    SDA.port->MODER &= ~GPIO_MODER_MODER9_Msk;
    SDA.port->MODER |= GPIO_MODER_MODER9_1;
    SCL.port->MODER &= ~GPIO_MODER_MODER8_Msk;
    SCL.port->MODER |= GPIO_MODER_MODER8_1;
    SDA.port->AFR[1] &= ~GPIO_AFRH_AFSEL8_Msk;
    SDA.port->AFR[1] |= GPIO_AFRH_AFSEL8_2;
    SCL.port->AFR[1] &= ~GPIO_AFRH_AFSEL9_Msk;
    SCL.port->AFR[1] |= GPIO_AFRH_AFSEL9_2;
    // setup freq peref
    I2C1->CR2 &= ~I2C_CR2_FREQ;
    I2C1->CR2 |= 42; // 42mhz (APB1)
    // setup 100khz speed of bus I2C
    I2C1->CCR &= ~I2C_CCR_CCR;
    I2C1->CCR |= 35;
    // rise time
    I2C1->TRISE = 13; 
    // enable I2C
    I2C1->CR1 |= I2C_CR1_PE;
}