#include "oled.h"
#include "main.h"
#include "stm32f446xx.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>


static uint8_t OLED_BUFFER[1024]; // OLED buffer in RAM

bool ping_oled(void) {
    I2C1->CR1 |= I2C_CR1_START; // generate START
    // wait flag SB in SR1
    uint32_t timeout = 10000;
    while (!(I2C1->SR1 & I2C_SR1_SB)) if (--timeout == 0) return false;
    // send OLED adress
    I2C1->DR = 0x78;
    timeout = 10000;
    // wait flag ADDR in SR1
    while(!(I2C1->SR1 & I2C_SR1_ADDR)) {
        if(I2C1->SR1 & I2C_SR1_AF) { // if get NACK i.e. OLED not responsed
            I2C1->CR1 |= I2C_CR1_STOP;
            return false;
        }
        if(--timeout == 0) return false;
    }
    // reset flag ADDR (stm32 specific)
    (void)I2C1->SR1;
    (void)I2C1->SR2;
    // generate STOP
    I2C1->CR1 |= I2C_CR1_STOP;
    return true; // display OK
}

void OLED_SendCommand(uint8_t cmd) {
    // start
    I2C1->CR1 |= I2C_CR1_START;
    while (!(I2C1->SR1 & I2C_SR1_SB));
    // address
    I2C1->DR = 0x78;
    while(!(I2C1->SR1 & I2C_SR1_ADDR));
    // reset ADDR
    (void)I2C1->SR1;
    (void)I2C1->SR2;
    // control byte: we tell the processor that we are sending a COMMAND
    I2C1->DR = 0x00;
    while (!(I2C1->SR1 & I2C_SR1_TXE)); // waiting until the data register is empty
    // the command
    I2C1->DR = cmd;
    while (!(I2C1->SR1 & I2C_SR1_BTF)); // waiting for the byte transfer to complete
    // stop
    I2C1->CR1 |= I2C_CR1_STOP;
}

void OLED_Init_Full(void) {
    OLED_SendCommand(0xAE); // Display OFF
    OLED_SendCommand(0x20); // Set Memory Addressing Mode
    OLED_SendCommand(0x10); // Page Addressing Mode
    OLED_SendCommand(0xB0); // Set Page Start Address for Page Addressing Mode
    OLED_SendCommand(0x81); // Set Contrast Control
    OLED_SendCommand(0xFF); // maximum brightness
    OLED_SendCommand(0xA1); // Set Segment Re-map (horizontal view)
    OLED_SendCommand(0xA6); // Normal display (not inversed)
    // --- Starting the power "pump" ---
    OLED_SendCommand(0x8D); // Charge Pump Command
    OLED_SendCommand(0x14); // Enable Charge Pump (0x10 - OFF, 0x14 - ON)

    OLED_SendCommand(0xAF); // Display ON!
}

void OLED_Clear(void) {
    OLED_SendCommand(0x00); // Set Memory Mode horizontal 
    OLED_SendCommand(0x21); // Set Column Addr
    OLED_SendCommand(0x00); // from 0
    OLED_SendCommand(0x7F); // to 127
    OLED_SendCommand(0x22); // Set Pages Addr
    OLED_SendCommand(0x00); // from 0
    OLED_SendCommand(0x07); // to 7

    I2C1->CR1 |= I2C_CR1_START;
    while (!(I2C1->SR1 & I2C_SR1_SB));
    I2C1->DR = 0x78;
    while(!(I2C1->SR1 & I2C_SR1_ADDR));
    (void)I2C1->SR1;
    (void)I2C1->SR2;

    I2C1->DR = 0x40;  // control byte: data
    for (uint16_t i = 0; i < 1024UL; i++) { // 8 pages * 128 column = 1024 bytes
        I2C1->DR = 0x00; // fill
        while (!(I2C1->SR1 & I2C_SR1_TXE));
    }
    while (!(I2C1->SR1 & I2C_SR1_BTF));
    I2C1->CR1 |= I2C_CR1_STOP;
}

void OLED_Fast_Clear(void) {
    memset(OLED_BUFFER, 0, 1024);
}

void OLED_Update(void) {
    OLED_SendCommand(0x00); // Set Memory Mode horizontal 
    OLED_SendCommand(0x21); // Set Column Addr
    OLED_SendCommand(0x00); // from 0
    OLED_SendCommand(0x7F); // to 127
    OLED_SendCommand(0x22); // Set Pages Addr
    OLED_SendCommand(0x00); // from 0
    OLED_SendCommand(0x07); // to 7

    I2C1->CR1 |= I2C_CR1_START;
    while (!(I2C1->SR1 & I2C_SR1_SB));
    I2C1->DR = 0x78;
    while(!(I2C1->SR1 & I2C_SR1_ADDR));
    (void)I2C1->SR1;
    (void)I2C1->SR2;

    I2C1->DR = 0x40;
    for(uint16_t i = 0; i < 1024UL; i++) {
        I2C1->DR = OLED_BUFFER[i];
        while (!(I2C1->SR1 & I2C_SR1_TXE));
    }
    while (!(I2C1->SR1 & I2C_SR1_BTF));
    I2C1->CR1 |= I2C_CR1_STOP;
}

void OLED_Draw_Pixel(int x, int y, uint8_t color) {
    if (x < 0 || x >= 128 || y < 0 || y >= 64) return;
    if(color) {
        OLED_BUFFER[x + (y / 8) * 128] |= (1 << (y % 8)); // draw bit
    } else {
        OLED_BUFFER[x + (y / 8) * 128] &= ~(1 << (y % 8)); // clear bit
    }
}

void OLED_Fast_Draw_Pixel(int x, int y, uint8_t color) {
    if (x < 0 || x >= 128 || y < 0 || y >= 64) return;
    // optimized index: x + (y >> 3) << 7
    // first find a page (y >> 3), 
    // then (<< 7)
    uint16_t index = x + ((y >> 3) << 7);
    if (color) {
        OLED_BUFFER[index] |= (1 << (y & 7));  // draw bit
    } else {
        OLED_BUFFER[index] &= ~(1 << (y & 7)); // clear bit
    }
}

// Bresenham's line algorithm
void OLED_Draw_Line(int x0, int y0, int x1, int y1) {
    int dx = abs(x1 - x0);
    int dy = -abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    
    while (1) {
        OLED_Fast_Draw_Pixel(x0, y0, 1);
        
        if (x0 == x1 && y0 == y1) break;
        
        int e2 = 2 * err;
        if (e2 >= dy) { 
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}