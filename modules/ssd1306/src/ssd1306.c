/*
        ssd1306.c file is External display library file.
    Copyright (C) 2018 Nima Mohammadi

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "ssd1306.h"

#define SSD1306_I2C_PORT              I2C0
/* Write command */
#define SSD1306_WRITECOMMAND(command) ssd1306_I2C_Write(SSD1306_I2C_ADDR, 0x00, (command))
/* Write data */
#define SSD1306_WRITEDATA(data)       ssd1306_I2C_Write(SSD1306_I2C_ADDR, 0x40, (data))
/* Absolute value */
#define ABS(x)                        ((x) > 0 ? (x) : -(x))

/* SSD1306 data buffer */
static uint8_t SSD1306_Buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];

/* Private SSD1306 structure */
typedef struct {
    uint16_t CurrentX;
    uint16_t CurrentY;
    uint8_t Inverted;
    uint8_t Initialized;
} SSD1306_t;

/* Private variable */
static SSD1306_t SSD1306;

uint8_t SSD1306_Init(void) {
    /* Init I2C */
    ssd1306_I2C_Init();

    /* A little delay */
    uint32_t p = 2500;
    while (p > 0) p--;

    /* Init LCD */
    SSD1306_WRITECOMMAND(0xAE); // display off
    SSD1306_WRITECOMMAND(0x20); // Set Memory Addressing Mode
    SSD1306_WRITECOMMAND(
        0x10); // 00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
    SSD1306_WRITECOMMAND(0xB0); // Set Page Start Address for Page Addressing Mode,0-7
    SSD1306_WRITECOMMAND(0xC8); // Set COM Output Scan Direction
    SSD1306_WRITECOMMAND(0x00); //---set low column address
    SSD1306_WRITECOMMAND(0x10); //---set high column address
    SSD1306_WRITECOMMAND(0x40); //--set start line address
    SSD1306_WRITECOMMAND(0x81); //--set contrast control register
    SSD1306_WRITECOMMAND(0xFF);
    SSD1306_WRITECOMMAND(0xA1); //--set segment re-map 0 to 127
    SSD1306_WRITECOMMAND(0xA6); //--set normal display
    SSD1306_WRITECOMMAND(0xA8); //--set multiplex ratio(1 to 64)
    SSD1306_WRITECOMMAND(0x3F); //
    SSD1306_WRITECOMMAND(0xA4); // 0xa4,Output follows RAM content;0xa5,Output ignores RAM content
    SSD1306_WRITECOMMAND(0xD3); //-set display offset
    SSD1306_WRITECOMMAND(0x00); //-not offset
    SSD1306_WRITECOMMAND(0xD5); //--set display clock divide ratio/oscillator frequency
    SSD1306_WRITECOMMAND(0xF0); //--set divide ratio
    SSD1306_WRITECOMMAND(0xD9); //--set pre-charge period
    SSD1306_WRITECOMMAND(0x22); //
    SSD1306_WRITECOMMAND(0xDA); //--set com pins hardware configuration
    SSD1306_WRITECOMMAND(0x12);
    SSD1306_WRITECOMMAND(0xDB); //--set vcomh
    SSD1306_WRITECOMMAND(0x20); // 0x20,0.77xVcc
    SSD1306_WRITECOMMAND(0x8D); //--set DC-DC enable
    SSD1306_WRITECOMMAND(0x14); //
    SSD1306_WRITECOMMAND(0xAF); //--turn on SSD1306 panel

    /* Clear screen */
    SSD1306_Fill(SSD1306_COLOR_BLACK);

    /* Update screen */
    SSD1306_UpdateScreen();

    /* Set default values */
    SSD1306.CurrentX = 0;
    SSD1306.CurrentY = 0;

    /* Initialized OK */
    SSD1306.Initialized = 1;

    /* Return OK */
    return 1;
}

void SSD1306_UpdateScreen(void) {
    uint8_t m;

    for (m = 0; m < 8; m++) {
        SSD1306_WRITECOMMAND(0xB0 + m);
        SSD1306_WRITECOMMAND(0x00);
        SSD1306_WRITECOMMAND(0x10);

        /* Write multi data */
        ssd1306_I2C_WriteMulti(SSD1306_I2C_ADDR, 0x40, &SSD1306_Buffer[SSD1306_WIDTH * m], SSD1306_WIDTH);
    }
}

void SSD1306_ToggleInvert(void) {
    uint16_t i;

    /* Toggle invert */
    SSD1306.Inverted = !SSD1306.Inverted;

    /* Do memory toggle */
    for (i = 0; i < sizeof(SSD1306_Buffer); i++) {
        SSD1306_Buffer[i] = ~SSD1306_Buffer[i];
    }
}

void SSD1306_Fill(SSD1306_COLOR_t color) {
    /* Set memory */
    memset(SSD1306_Buffer, (color == SSD1306_COLOR_BLACK) ? 0x00 : 0xFF, sizeof(SSD1306_Buffer));
}

void SSD1306_DrawPixel(uint16_t x, uint16_t y, SSD1306_COLOR_t color) {
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) {
        /* Error */
        return;
    }

    /* Check if pixels are inverted */
    if (SSD1306.Inverted) {
        color = (SSD1306_COLOR_t)!color;
    }

    /* Set color */
    if (color == SSD1306_COLOR_WHITE) {
        SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] |= 1 << (y % 8);
    } else {
        SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
    }
}

void SSD1306_GotoXY(uint16_t x, uint16_t y) {
    /* Set write pointers */
    SSD1306.CurrentX = x;
    SSD1306.CurrentY = y;
}

char SSD1306_Putc(char ch, FontDef_t * Font, SSD1306_COLOR_t color) {
    uint32_t i, b, j;

    /* Check available space in LCD */
    if (SSD1306_WIDTH <= (SSD1306.CurrentX + Font->FontWidth) ||
        SSD1306_HEIGHT <= (SSD1306.CurrentY + Font->FontHeight)) {
        /* Error */
        return 0;
    }

    /* Go through font */
    for (i = 0; i < Font->FontHeight; i++) {
        b = Font->data[(ch - 32) * Font->FontHeight + i];
        for (j = 0; j < Font->FontWidth; j++) {
            if ((b << j) & 0x8000) {
                SSD1306_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (SSD1306_COLOR_t)color);
            } else {
                SSD1306_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (SSD1306_COLOR_t)!color);
            }
        }
    }

    /* Increase pointer */
    SSD1306.CurrentX += Font->FontWidth;

    /* Return character written */
    return ch;
}

char SSD1306_Puts(const char * str, FontDef_t * Font, SSD1306_COLOR_t color) {
    /* Write characters */
    while (*str) {
        /* Write character by character */
        if (SSD1306_Putc(*str, Font, color) != *str) {
            /* Return error */
            return *str;
        }

        /* Increase string pointer */
        str++;
    }

    /* Everything OK, zero should be returned */
    return *str;
}

void SSD1306_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, SSD1306_COLOR_t c) {
    int16_t dx, dy, sx, sy, err, e2, i, tmp;

    /* Check for overflow */
    if (x0 >= SSD1306_WIDTH) {
        x0 = SSD1306_WIDTH - 1;
    }
    if (x1 >= SSD1306_WIDTH) {
        x1 = SSD1306_WIDTH - 1;
    }
    if (y0 >= SSD1306_HEIGHT) {
        y0 = SSD1306_HEIGHT - 1;
    }
    if (y1 >= SSD1306_HEIGHT) {
        y1 = SSD1306_HEIGHT - 1;
    }

    dx = (x0 < x1) ? (x1 - x0) : (x0 - x1);
    dy = (y0 < y1) ? (y1 - y0) : (y0 - y1);
    sx = (x0 < x1) ? 1 : -1;
    sy = (y0 < y1) ? 1 : -1;
    err = ((dx > dy) ? dx : -dy) / 2;

    if (dx == 0) {
        if (y1 < y0) {
            tmp = y1;
            y1 = y0;
            y0 = tmp;
        }

        if (x1 < x0) {
            tmp = x1;
            x1 = x0;
            x0 = tmp;
        }

        /* Vertical line */
        for (i = y0; i <= y1; i++) {
            SSD1306_DrawPixel(x0, i, c);
        }

        /* Return from function */
        return;
    }

    if (dy == 0) {
        if (y1 < y0) {
            tmp = y1;
            y1 = y0;
            y0 = tmp;
        }

        if (x1 < x0) {
            tmp = x1;
            x1 = x0;
            x0 = tmp;
        }

        /* Horizontal line */
        for (i = x0; i <= x1; i++) {
            SSD1306_DrawPixel(i, y0, c);
        }

        /* Return from function */
        return;
    }

    while (1) {
        SSD1306_DrawPixel(x0, y0, c);
        if (x0 == x1 && y0 == y1) {
            break;
        }
        e2 = err;
        if (e2 > -dx) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dy) {
            err += dx;
            y0 += sy;
        }
    }
}

void SSD1306_DrawRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, SSD1306_COLOR_t c) {
    /* Check input parameters */
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) {
        /* Return error */
        return;
    }

    /* Check width and height */
    if ((x + w) >= SSD1306_WIDTH) {
        w = SSD1306_WIDTH - x;
    }
    if ((y + h) >= SSD1306_HEIGHT) {
        h = SSD1306_HEIGHT - y;
    }

    /* Draw 4 lines */
    SSD1306_DrawLine(x, y, x + w, y, c);         /* Top line */
    SSD1306_DrawLine(x, y + h, x + w, y + h, c); /* Bottom line */
    SSD1306_DrawLine(x, y, x, y + h, c);         /* Left line */
    SSD1306_DrawLine(x + w, y, x + w, y + h, c); /* Right line */
}

void SSD1306_DrawFilledRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, SSD1306_COLOR_t c) {
    uint8_t i;

    /* Check input parameters */
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) {
        /* Return error */
        return;
    }

    /* Check width and height */
    if ((x + w) >= SSD1306_WIDTH) {
        w = SSD1306_WIDTH - x;
    }
    if ((y + h) >= SSD1306_HEIGHT) {
        h = SSD1306_HEIGHT - y;
    }

    /* Draw lines */
    for (i = 0; i <= h; i++) {
        /* Draw lines */
        SSD1306_DrawLine(x, y + i, x + w, y + i, c);
    }
}

void SSD1306_DrawTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3,
                          SSD1306_COLOR_t color) {
    /* Draw lines */
    SSD1306_DrawLine(x1, y1, x2, y2, color);
    SSD1306_DrawLine(x2, y2, x3, y3, color);
    SSD1306_DrawLine(x3, y3, x1, y1, color);
}

void SSD1306_DrawFilledTriangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t x3, uint16_t y3,
                                SSD1306_COLOR_t color) {
    int16_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0, yinc1 = 0, yinc2 = 0, den = 0, num = 0,
            numadd = 0, numpixels = 0, curpixel = 0;

    deltax = ABS(x2 - x1);
    deltay = ABS(y2 - y1);
    x = x1;
    y = y1;

    if (x2 >= x1) {
        xinc1 = 1;
        xinc2 = 1;
    } else {
        xinc1 = -1;
        xinc2 = -1;
    }

    if (y2 >= y1) {
        yinc1 = 1;
        yinc2 = 1;
    } else {
        yinc1 = -1;
        yinc2 = -1;
    }

    if (deltax >= deltay) {
        xinc1 = 0;
        yinc2 = 0;
        den = deltax;
        num = deltax / 2;
        numadd = deltay;
        numpixels = deltax;
    } else {
        xinc2 = 0;
        yinc1 = 0;
        den = deltay;
        num = deltay / 2;
        numadd = deltax;
        numpixels = deltay;
    }

    for (curpixel = 0; curpixel <= numpixels; curpixel++) {
        SSD1306_DrawLine(x, y, x3, y3, color);

        num += numadd;
        if (num >= den) {
            num -= den;
            x += xinc1;
            y += yinc1;
        }
        x += xinc2;
        y += yinc2;
    }
}

void SSD1306_DrawCircle(int16_t x0, int16_t y0, int16_t r, SSD1306_COLOR_t c) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    SSD1306_DrawPixel(x0, y0 + r, c);
    SSD1306_DrawPixel(x0, y0 - r, c);
    SSD1306_DrawPixel(x0 + r, y0, c);
    SSD1306_DrawPixel(x0 - r, y0, c);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        SSD1306_DrawPixel(x0 + x, y0 + y, c);
        SSD1306_DrawPixel(x0 - x, y0 + y, c);
        SSD1306_DrawPixel(x0 + x, y0 - y, c);
        SSD1306_DrawPixel(x0 - x, y0 - y, c);

        SSD1306_DrawPixel(x0 + y, y0 + x, c);
        SSD1306_DrawPixel(x0 - y, y0 + x, c);
        SSD1306_DrawPixel(x0 + y, y0 - x, c);
        SSD1306_DrawPixel(x0 - y, y0 - x, c);
    }
}

void SSD1306_DrawFilledCircle(int16_t x0, int16_t y0, int16_t r, SSD1306_COLOR_t c) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    SSD1306_DrawPixel(x0, y0 + r, c);
    SSD1306_DrawPixel(x0, y0 - r, c);
    SSD1306_DrawPixel(x0 + r, y0, c);
    SSD1306_DrawPixel(x0 - r, y0, c);
    SSD1306_DrawLine(x0 - r, y0, x0 + r, y0, c);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        SSD1306_DrawLine(x0 - x, y0 + y, x0 + x, y0 + y, c);
        SSD1306_DrawLine(x0 + x, y0 - y, x0 - x, y0 - y, c);

        SSD1306_DrawLine(x0 + y, y0 + x, x0 - y, y0 + x, c);
        SSD1306_DrawLine(x0 + y, y0 - x, x0 - y, y0 - x, c);
    }
}

void SSD1306_ON(void) {
    SSD1306_WRITECOMMAND(0x8D);
    SSD1306_WRITECOMMAND(0x14);
    SSD1306_WRITECOMMAND(0xAF);
}
void SSD1306_OFF(void) {
    SSD1306_WRITECOMMAND(0x8D);
    SSD1306_WRITECOMMAND(0x10);
    SSD1306_WRITECOMMAND(0xAE);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//  _____ ___   _____
// |_   _|__ \ / ____|
//   | |    ) | |
//   | |   / /| |
//  _| |_ / /_| |____
// |_____|____|\_____|
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////

void ssd1306_I2C_Init() {
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_I2C0);
    rcu_periph_clock_enable(RCU_AF);

    gpio_pin_remap_config(GPIO_I2C0_REMAP, ENABLE);
    gpio_init(GPIOB, GPIO_MODE_AF_OD, GPIO_OSPEED_50MHZ, GPIO_PIN_8 | GPIO_PIN_9);

    i2c_deinit(I2C0);
    i2c_clock_config(I2C0, 10000, I2C_DTCY_2);
    i2c_enable(I2C0);
    /* enable acknowledge */
    i2c_ack_config(I2C0, I2C_ACK_ENABLE);
}

void ssd1306_I2C_WriteMulti(uint8_t address, uint8_t reg, uint8_t * data, uint16_t size) {
    /* send a start condition to I2C bus */
    i2c_start_on_bus(I2C0);
    /* wait until SBSEND bit is set */
    while (!i2c_flag_get(I2C0, I2C_FLAG_SBSEND));

    /* send slave address to I2C bus */
    i2c_master_addressing(I2C0, address << 1, I2C_TRANSMITTER);

    i2c_ack_config(I2C0, I2C_ACK_ENABLE);

    /* wait until ADDSEND bit is set */
    while (!i2c_flag_get(I2C0, I2C_FLAG_ADDSEND));
    /* clear ADDSEND bit */
    i2c_flag_clear(I2C0, I2C_FLAG_ADDSEND);

    while (!i2c_flag_get(I2C0, I2C_FLAG_TBE));
    i2c_data_transmit(I2C0, reg);

    for (int count = 0; count < size; count++) {
        /* wait until the transmit data buffer is empty */
        while (!i2c_flag_get(I2C0, I2C_FLAG_TBE));
        /* data transmission with the number of first register to read */
        i2c_data_transmit(I2C0, data[count]);

        if (size - 1 == count) {
            /* wait until the second last data byte is received into the shift register */
            while (!i2c_flag_get(I2C0, I2C_FLAG_BTC));
            /* disable acknowledge */
            i2c_ack_config(I2C0, I2C_ACK_DISABLE);
        }
    }
}

void ssd1306_I2C_Write(uint8_t address, uint8_t reg, uint8_t data) {
    ssd1306_I2C_WriteMulti(address, reg, &data, 1);
}
