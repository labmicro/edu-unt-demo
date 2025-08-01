/*
  SSD 1306 OLED driver
*/

#ifndef __LCD_H
#define __LCD_H

#include "systick.h"
#include "stdlib.h"
#include "gd32vf103_gpio.h"

// Set horizontal or vertical screen display:
// 0 or 1 for vertical screen
// 2 or 3 for horizontal screen
#define USE_HORIZONTAL 2
#define HAS_BLK_CNTL   0

#if USE_HORIZONTAL == 0 || USE_HORIZONTAL == 1
#define LCD_W 64
#define LCD_H 128
#else
#define LCD_W 128
#define LCD_H 64
#endif

typedef unsigned char u8;
typedef unsigned int u16;
typedef unsigned long u32;

#define LED_ON
#define LED_OFF

#if HAS_BLK_CNTL
#define OLED_BLK_Clr() gpio_bit_reset(GPIOA, GPIO_PIN_5) // BLK
#define OLED_BLK_Set() gpio_bit_set(GPIOA, GPIO_PIN_5)
#else
#define OLED_BLK_Clr()
#define OLED_BLK_Set()
#endif

#define OLED_CMD    0 // Write command
#define OLED_DATA   1 // Write data

#define TRANSPARENT 1
#define OPAQUE      0

extern u16 BACK_COLOR; // Background color

void LCD_Writ_Bus(u8 dat);
void LCD_WR_DATA8(u8 dat);
void LCD_WR_DATA(u16 dat);
void LCD_WR_REG(u8 dat);
void LCD_Address_Set(u16 x1, u16 y1, u16 x2, u16 y2);
void LCD_Init(void);
void LCD_Clear(u16 Color);
void LCD_ShowChinese(u16 x, u16 y, u8 index, u8 size, u16 color);
void LCD_DrawPoint(u16 x, u16 y, u16 color);
void LCD_DrawPoint_big(u16 x, u16 y, u16 color);
void LCD_Fill(u16 xsta, u16 ysta, u16 xend, u16 yend, u16 color);
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2, u16 color);
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2, u16 color);
void Draw_Circle(u16 x0, u16 y0, u8 r, u16 color);
void LCD_ShowChar(u16 x, u16 y, char num, u8 mode, u16 color);
void LCD_ShowString(u16 x, u16 y, const char * p, u16 color);
void LCD_ShowStr(u16 x, u16 y, const char * p, u16 color, u8 mode);
u32 mypow(u8 m, u8 n);
void LCD_ShowNum(u16 x, u16 y, u16 num, u8 len, u16 color);
void LCD_ShowNum1(u16 x, u16 y, float num, u8 len, u16 color);
void LCD_ShowPicture(u16 x1, u16 y1, u16 x2, u16 y2, u8 * image);
void LCD_ShowLogo(u16 y);

// Brush color
#define WHITE      0xFFFF
#define BLACK      0x0000
#define BLUE       0x001F
#define BRED       0XF81F
#define GRED       0XFFE0
#define GBLUE      0X07FF
#define RED        0xF800
#define MAGENTA    0xF81F
#define GREEN      0x07E0
#define CYAN       0x7FFF
#define YELLOW     0xFFE0
#define BROWN      0XBC40
#define BRRED      0XFC07 // maroon
#define GRAY       0X8430
// GUI Color
#define DARKBLUE   0X01CF // Navy blue
#define LIGHTBLUE  0X7D7C
#define GRAYBLUE   0X5458
// The above three colors are the colors of PANEL

#define LIGHTGREEN 0X841F
#define LGRAY      0XC618 // Light gray (PANNEL), form background color
#define DGRAY      0X31A6 // Dark gray

#define LGRAYBLUE  0XA651 // Light gray blue (middle layer color)
#define LBBLUE     0X2B12 // Light brown blue (inverted color of selected item)

#endif
