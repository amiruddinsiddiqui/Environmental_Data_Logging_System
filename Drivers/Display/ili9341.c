/*
 * ili9341.c
 *
 *  Created on: Nov 15, 2026
 *      Author: asus
 */



#include "ili9341.h"
#include "basic_5x5_font.h"
#include "spi.h"
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"

#include "sd_storage.h"

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

/* ILI9341 commands */
#define SWRESET     0x01U
#define SLPOUT      0x11U
#define DISPON      0x29U
#define CASET       0x2AU
#define PASET       0x2BU
#define RAMWR       0x2CU
#define MADCTL      0x36U
#define COLMOD      0x3AU

/* Hardware pins
 * PB12 = DC
 * PB11 = RESET
 * PB1  = CS (controlled by spi_cs)
 */

#define DC_HIGH()       (GPIOB->BSRR = (1U << 12U))
#define DC_LOW()        (GPIOB->BSRR = (1U << 28U))

#define RESET_HIGH()    (GPIOB->BSRR = (1U << 11U))
#define RESET_LOW()     (GPIOB->BSRR = (1U << 27U))


/* Send command */
static void ILI9341_WriteCommand(uint8_t command)
{
    spi_cs(0U);
    DC_LOW();

    (void)spi_tx_n_rx(command);

    spi_cs(1U);
}


/* Send one data byte */
static void ILI9341_WriteDataByte(uint8_t data)
{
    spi_cs(0U);
    DC_HIGH();

    (void)spi_tx_n_rx(data);

    spi_cs(1U);
}


/* Set drawing coordinates */
static void ILI9341_SetAddress(uint16_t x1, uint16_t y1,
                               uint16_t x2, uint16_t y2)
{
    ILI9341_WriteCommand(CASET);

    ILI9341_WriteDataByte((uint8_t)(x1 >> 8U));
    ILI9341_WriteDataByte((uint8_t)x1);
    ILI9341_WriteDataByte((uint8_t)(x2 >> 8U));
    ILI9341_WriteDataByte((uint8_t)x2);

    ILI9341_WriteCommand(PASET);

    ILI9341_WriteDataByte((uint8_t)(y1 >> 8U));
    ILI9341_WriteDataByte((uint8_t)y1);
    ILI9341_WriteDataByte((uint8_t)(y2 >> 8U));
    ILI9341_WriteDataByte((uint8_t)y2);
}


/* Send repeated RGB565 pixels */
static void ILI9341_WriteColor(uint16_t color, uint32_t count)
{
    uint8_t high = (uint8_t)(color >> 8U);
    uint8_t low  = (uint8_t)color;

    spi_cs(0U);
    DC_HIGH();

    while (count--)
    {
        (void)spi_tx_n_rx(high);
        (void)spi_tx_n_rx(low);
    }

    spi_cs(1U);
}


/* Fill rectangle */
void ILI9341_FillRectangle(uint16_t x, uint16_t y,
                           uint16_t width, uint16_t height,
                           uint16_t color)
{
    if (x >= ILI9341_WIDTH ||
        y >= ILI9341_HEIGHT ||
        width == 0U ||
        height == 0U)
    {
        return;
    }

    if (width > ILI9341_WIDTH - x)
    {
        width = ILI9341_WIDTH - x;
    }

    if (height > ILI9341_HEIGHT - y)
    {
        height = ILI9341_HEIGHT - y;
    }

    ILI9341_SetAddress(x, y,
                       x + width - 1U,
                       y + height - 1U);

    ILI9341_WriteCommand(RAMWR);

    ILI9341_WriteColor(color, (uint32_t)width * height);
}


/* Fill complete display */
void ILI9341_FillScreen(uint16_t color)
{
    ILI9341_FillRectangle(0U, 0U,
                          ILI9341_WIDTH,
                          ILI9341_HEIGHT,
                          color);
}


/* Initialize ILI9341 */
void ILI9341_Init(void)
{
    /* Initialize SPI2 */
    spi2_init();

    /* Hardware reset */
    RESET_HIGH();
    HAL_Delay(10U);

    RESET_LOW();
    HAL_Delay(20U);

    RESET_HIGH();
    HAL_Delay(120U);

    /* Software reset */
    ILI9341_WriteCommand(SWRESET);
    HAL_Delay(120U);

    /* Exit sleep mode */
    ILI9341_WriteCommand(SLPOUT);
    HAL_Delay(120U);

    /* RGB565 pixel format */
    ILI9341_WriteCommand(COLMOD);
    ILI9341_WriteDataByte(0x55U);
    HAL_Delay(10U);

    /* BGR color order */
    ILI9341_WriteCommand(MADCTL);
    ILI9341_WriteDataByte(0x08U);

    /* Display ON */
    ILI9341_WriteCommand(DISPON);
    HAL_Delay(100U);

    /* Clear display */
    ILI9341_FillScreen(BLACK);
}


/* Draw one character from your 6x8 bitmap */
void ILI9341_DrawChar(uint16_t x, uint16_t y, char c,
                      uint16_t color, uint16_t background,
                      uint8_t scale)
{
    uint8_t ch = (uint8_t)c;

    if (scale == 0U)
    {
        return;
    }

    if (ch < 32U || ch > 127U)
    {
        ch = '?';
    }

    const uint8_t *bitmap = font[ch - 32U];

    uint16_t width  = CHAR_WIDTH * scale;
    uint16_t height = CHAR_HEIGHT * scale;

    if ((x + width > ILI9341_WIDTH) ||
        (y + height > ILI9341_HEIGHT))
    {
        return;
    }

    ILI9341_SetAddress(x, y,
                       x + width - 1U,
                       y + height - 1U);

    ILI9341_WriteCommand(RAMWR);

    spi_cs(0U);
    DC_HIGH();

    for (uint8_t row = 0U; row < CHAR_HEIGHT; row++)
    {
        for (uint8_t repeat_y = 0U; repeat_y < scale; repeat_y++)
        {
            for (uint8_t col = 0U; col < CHAR_WIDTH; col++)
            {
                uint16_t pixel_color =
                    (bitmap[col] & (1U << row)) ?
                    color : background;

                uint8_t high = (uint8_t)(pixel_color >> 8U);
                uint8_t low  = (uint8_t)pixel_color;

                for (uint8_t repeat_x = 0U;
                     repeat_x < scale;
                     repeat_x++)
                {
                    (void)spi_tx_n_rx(high);
                    (void)spi_tx_n_rx(low);
                }
            }
        }
    }

    spi_cs(1U);
}


/* Print string */
void ILI9341_WriteString(uint16_t x, uint16_t y,
                         const char *str,
                         uint16_t color, uint16_t background,
                         uint8_t scale)
{
    if (str == NULL || scale == 0U)
    {
        return;
    }

    while (*str)
    {
        if (*str == '\n')
        {
            y += (CHAR_HEIGHT + 2U) * scale;
            x = 10U;

            str++;
            continue;
        }

        if (x + (CHAR_WIDTH * scale) > ILI9341_WIDTH)
        {
            break;
        }

        ILI9341_DrawChar(x, y, *str,
                         color, background, scale);

        x += (CHAR_WIDTH + 1U) * scale;

        str++;
    }
}


/* Draw separator */
void ILI9341_DrawLine(uint16_t y, uint16_t color)
{
    ILI9341_FillRectangle(10U, y, 220U, 2U, color);
}



