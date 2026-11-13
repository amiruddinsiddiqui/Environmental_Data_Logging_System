/*
 * sd_storage.c
 *
 * Created on: Nov 12, 2026
 * Author: asus
 */

#include "fatfs.h"
#include "sd_storage.h"
#include "uart.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

static FATFS FatFs;
static uint8_t sd_ready = 0;


/* Initialize SD card and create CSV header */
void SD_Card_Init(void) {
    FIL Fil;
    UINT WWC = 0;
    FRESULT FR_Status;

    sd_ready = 0;

    /* Mount SD card */
    FR_Status = f_mount(&FatFs, "", 1);

    if (FR_Status != FR_OK) {
        printf("[ERROR] Mount failed: %d\r\n", FR_Status);
        return;
    }

    printf("[INFO] SD Card mounted successfully\r\n");

    /* Create file if it doesn't exist; preserve existing data */
    FR_Status = f_open(&Fil, "ENV_LOG.TXT",
                       FA_WRITE | FA_OPEN_ALWAYS);

    if (FR_Status != FR_OK) {
        printf("[ERROR] File open failed: %d\r\n", FR_Status);
        return;
    }

    /* Write header only if file is empty */
    if (f_size(&Fil) == 0) {
        const char header[] = "Temperature (C), Pressure (Pa)\r\n";

        FR_Status = f_write(&Fil, header, sizeof(header) - 1, &WWC);

        if (FR_Status != FR_OK || WWC != sizeof(header) - 1) {
            printf("[ERROR] Header write failed\r\n");
            f_close(&Fil);
            return;
        }

        printf("[INFO] CSV header written\r\n");
    }

    FR_Status = f_close(&Fil);

    if (FR_Status != FR_OK) {
        printf("[ERROR] File close failed\r\n");
        return;
    }

    sd_ready = 1;

    printf("[INFO] SD logging ready\r\n");
}


/* Append temperature and pressure measurements */
void SD_Log_Data(double temperature, double pressure) {
    FIL Fil;
    UINT WWC = 0;
    FRESULT FR_Status;

    char log_buffer[100];

    if (!sd_ready)
        return;

    /* Convert floating-point values into integers */
    int t_scaled = (int)(temperature * 100);
    int p_scaled = (int)(pressure * 100);

    const char *sign = (t_scaled < 0) ? "-" : "";

    int len = snprintf(log_buffer, sizeof(log_buffer),
                       "%s%d.%02d, %d.%02d\r\n",
                       sign,
                       abs(t_scaled / 100),
                       abs(t_scaled % 100),
                       p_scaled / 100,
                       p_scaled % 100);

    if (len <= 0 || len >= sizeof(log_buffer))
        return;

    /* Open existing file */
    FR_Status = f_open(&Fil, "ENV_LOG.TXT",
                       FA_OPEN_EXISTING | FA_WRITE);

    if (FR_Status != FR_OK) {
        printf("[ERROR] Log file open failed: %d\r\n", FR_Status);
        return;
    }

    /* Move file pointer to the end */
    FR_Status = f_lseek(&Fil, f_size(&Fil));

    if (FR_Status != FR_OK) {
        printf("[ERROR] Seek failed: %d\r\n", FR_Status);
        f_close(&Fil);
        return;
    }

    /* Write measurement */
    FR_Status = f_write(&Fil, log_buffer, (UINT)len, &WWC);

    if (FR_Status != FR_OK || WWC != (UINT)len) {
        printf("[ERROR] Data write failed: %d\r\n", FR_Status);
    }

    f_close(&Fil);
}




