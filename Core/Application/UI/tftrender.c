/*
 * tftrender.c
 *
 *  Created on: Nov 15, 2026
 *      Author: asus
 */

#include "tftrender.h"
#include "ili9341.h"
#include "sd_storage.h"

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>



void TFT_Display_Init(void) {
    ILI9341_Init();

    /* Header */
    ILI9341_FillRectangle(0U, 0U, 240U, 45U, BLUE);

    ILI9341_WriteString(12U, 12U, "ENVIRONMENT", WHITE, BLUE, 2U);

    ILI9341_DrawLine(50U, CYAN);

    /* Temperature section */
    ILI9341_WriteString(15U, 65U, "TEMPERATURE", CYAN, BLACK, 2U);

    ILI9341_DrawLine(95U, GREY);

    /* Pressure section */
    ILI9341_WriteString(15U, 175U, "PRESSURE", YELLOW, BLACK, 2U);

    ILI9341_DrawLine(205U, GREY);

    /* Footer */
    ILI9341_WriteString(15U, 285U, "STM32F401RE", GREEN, BLACK, 2U);

    ILI9341_WriteString(15U, 305U, "DATA LOGGER", WHITE, BLACK, 1U);
}


/* Update sensor readings */
void TFT_Display_Update(void)
{
    char temp_buffer[20];
    char pressure_buffer[20];

    snprintf(temp_buffer, sizeof(temp_buffer), "%s%d.%02d", sign, t_whole, t_dec);

    snprintf(pressure_buffer, sizeof(pressure_buffer), "%d.%02d", p_whole, p_dec);

    /* Update temperature */
    ILI9341_FillRectangle(15U, 105U, 210U, 48U, BLACK);

    ILI9341_WriteString(15U, 115U, temp_buffer, GREEN, BLACK, 3U);

    ILI9341_WriteString(165U, 125U, "C", WHITE, BLACK, 2U);

    /* Update pressure */
    ILI9341_FillRectangle(15U, 215U, 210U, 48U, BLACK);

    ILI9341_WriteString(15U, 225U, pressure_buffer, ORANGE, BLACK, 2U);

    ILI9341_WriteString(165U, 235U, "PA", WHITE, BLACK, 1U);
}
