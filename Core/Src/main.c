#include "main.h"
#include "rcc.h"
#include "i2c.h"
#include "oled.h"
#include "stm32f4xx_hal.h"
#include "uart.h"
#include <stdint.h>
#include <math.h>

typedef struct {
    float x, y, z;
} Point3D;

// Cube vertices (centered at 0,0,0)
Point3D vertices[8] = {
    {-20, -20,  20}, { 20, -20,  20}, { 20,  20,  20}, {-20,  20,  20},
    {-20, -20, -20}, { 20, -20, -20}, { 20,  20, -20}, {-20,  20, -20}
};

void rotate(Point3D *p, float angleX, float angleY) {
    float radX = angleX;
    float radY = angleY;

    // Rotation around X
    float y = p->y;
    p->y = y * cos(radX) - p->z * sin(radX);
    p->z = y * sin(radX) + p->z * cos(radX);

    // Rotation around Y
    float x = p->x;
    p->x = x * cos(radY) + p->z * sin(radY);
    p->z = -x * sin(radY) + p->z * cos(radY);
}

void project(Point3D p, int *screenX, int *screenY) {
    *screenX = (int)(p.x + 64);
    *screenY = (int)(p.y + 32);
}


int main(void) {
  SystemClock_Config();
  USART2_init();
  I2C_init();
  if(ping_oled()) {
    UART_Printf("OLED STATUS: %d\r\n", 1);
  }
  else {
    UART_Printf("OLED STATUS: %d\r\n", 0);
  }
  OLED_Init_Full();


  float angleX = 0, angleY = 0;

  while (1) {
    OLED_Fast_Clear();

    int screenX[8], screenY[8];

    for (int i = 0; i < 8; i++) {
        // take data from the original array without changing it
        Point3D v = vertices[i]; 
        
        // Rotate the temporary copy
        rotate(&v, angleX, angleY);
        
        // project and IMMEDIATELY move it to the center of the screen
        // v.x and v.y should be around +-20 to fit into 128x64
        screenX[i] = (int)(v.x + 64);
        screenY[i] = (int)(v.y + 32);
    }

    // Drawing the edges
    for (int i = 0; i < 4; i++) {
        OLED_Draw_Line(screenX[i], screenY[i], screenX[(i+1)%4], screenY[(i+1)%4]);
        OLED_Draw_Line(screenX[i+4], screenY[i+4], screenX[((i+1)%4)+4], screenY[((i+1)%4)+4]);
        OLED_Draw_Line(screenX[i], screenY[i], screenX[i+4], screenY[i+4]);
    }

    OLED_Update();

    angleX += 0.03f;
    angleY += 0.02f;
    
    HAL_Delay(20);

  }

}



/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}