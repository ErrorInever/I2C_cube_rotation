#ifndef OLED_H_
#define OLED_H_

#include <stdbool.h>
#include <stdint.h>


bool ping_oled(void);
void OLED_SendCommand(uint8_t cmd);
void OLED_Init_Full(void);
void OLED_Clear(void);
void OLED_Fast_Clear(void);
void OLED_Update(void);
void OLED_Draw_Pixel(int x, int y, uint8_t color);
void OLED_Fast_Draw_Pixel(int x, int y, uint8_t color);
void OLED_Draw_Line(int x0, int y0, int x1, int y1);

#endif
