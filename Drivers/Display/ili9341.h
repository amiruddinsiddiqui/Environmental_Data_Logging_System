/*
 * ili9341.h
 *
 *  Created on: Nov 15, 2026
 *      Author: asus
 */

#ifndef DISPLAY_ILI9341_H_
#define DISPLAY_ILI9341_H_





#include <stdint.h>

/* Display dimensions */
#define ILI9341_WIDTH   240U
#define ILI9341_HEIGHT  320U

/* RGB565 colors */
#define BLACK    0x0000U
#define WHITE    0xFFFFU
#define RED      0xF800U
#define GREEN    0x07E0U
#define BLUE     0x001FU
#define CYAN     0x07FFU
#define YELLOW   0xFFE0U
#define ORANGE   0xFD20U
#define GREY     0x7BEFU


/* Display functions */
void ILI9341_Init(void);
void ILI9341_FillScreen(uint16_t color);
void ILI9341_FillRectangle(uint16_t x, uint16_t y,
                           uint16_t width, uint16_t height,
                           uint16_t color);

void ILI9341_DrawChar(uint16_t x, uint16_t y, char c,
                      uint16_t color, uint16_t background,
                      uint8_t scale);

void ILI9341_WriteString(uint16_t x, uint16_t y,
                         const char *str,
                         uint16_t color, uint16_t background,
                         uint8_t scale);
void ILI9341_DrawLine(uint16_t y, uint16_t color);


#endif /* DISPLAY_ILI9341_H_ */
