/*
 * bmp280.c
 *
 *  Created on: Nov 9, 2026
 *      Author: asus
 */

#include "stm32f4xx.h"
#include "bmp280.h"
#include "i2c.h"
#include "dma.h"


#define SLAVE_ADDR        0x76U
#define SLAVE_ADDR_WRITE  ((SLAVE_ADDR << 1) | 0U)
#define SLAVE_ADDR_READ   ((SLAVE_ADDR << 1) | 1U)

#define BMP280_REG_CTRL_MEAS     0xF4
#define PRESSURE_MSB_REG         0xF7
#define CALIBRATION_DIG_T1_REG   0x88

uint8_t raw_buffer[5];
uint16_t merged_buffer[2];

uint8_t calibration_buffer[24];

uint32_t adc_P;
uint32_t adc_T;

typedef struct BMP280_Calib_t {
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;

    uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;

} BMP280_Calib_t;

BMP280_Calib_t calib;


static void bmp280_read_raw_temp_n_pressure(uint8_t *buffer, uint8_t size);
static uint16_t read_u16_le(uint8_t index);
static int16_t read_s16_le(uint8_t index);


void bmp280_init(void) {
    // configure BMP280: Normal mode, temp/pressure oversampling x1
    i2c_start();
    i2c_address(SLAVE_ADDR_WRITE);
    i2c_write(BMP280_REG_CTRL_MEAS); // ctrl_meas register
    i2c_write(0x27); // (osrs_t osrs_p mode):-> (001 001 11); normal mode (bits 1-0 = 11), oversampling x1 (bits 5-2 = 001 001)
    i2c_stop();

    // read factory calibration coefficients
    bmp280_read_calibration();

    // extract calibration coefficients
    bmp280_extract_calibration();
}

static void bmp280_read_raw_temp_n_pressure(uint8_t *buffer, uint8_t size) {
    i2c_start();
    i2c_address(SLAVE_ADDR_WRITE);
    i2c_write(PRESSURE_MSB_REG); // point to pressure MSB register (0xF7)

    i2c_start(); // repeated start for reading
//    i2c_read(SLAVE_ADDR_READ, buffer, size);
    i2c_read_dma(SLAVE_ADDR_READ, buffer, size);
}


void bmp280_merge_bytes_to_adc(void) {
	// pressure-> 16 adc data
	merged_buffer[0] = ((uint16_t)raw_buffer[0]<<8U) | (uint16_t)raw_buffer[1]<<0U;
	// temperature-> 16 adc data
	merged_buffer[1] = ((uint16_t)raw_buffer[3]<<8U) | (uint16_t)raw_buffer[4]<<0U;

	// reconstruct 20-bit-aligned ADC values
	adc_P = (uint32_t)merged_buffer[0] << 4U;
	adc_T = (uint32_t)merged_buffer[1] << 4U;
}

void bmp280_read_calibration(void) {
	i2c_start();
    i2c_address(SLAVE_ADDR_WRITE);
    i2c_write(CALIBRATION_DIG_T1_REG);

    i2c_start(); // repeated start for reading
    i2c_read(SLAVE_ADDR_READ, calibration_buffer, 24);
}


// read a 16-bit unsigned integer from two bytes stored in Little Endian(LE) format
// the first byte is LSB, and the second byte is MSB
static uint16_t read_u16_le(uint8_t index) {
    return (uint16_t)(((uint16_t)calibration_buffer[index + 1] << 8U) | calibration_buffer[index]);
}

static int16_t read_s16_le(uint8_t index) {
    return (int16_t)read_u16_le(index);
}

void bmp280_extract_calibration(void) {
    calib.dig_T1 = read_u16_le(0);
    calib.dig_T2 = read_s16_le(2);
    calib.dig_T3 = read_s16_le(4);

    calib.dig_P1 = read_u16_le(6);
    calib.dig_P2 = read_s16_le(8);
    calib.dig_P3 = read_s16_le(10);
    calib.dig_P4 = read_s16_le(12);
    calib.dig_P5 = read_s16_le(14);
    calib.dig_P6 = read_s16_le(16);
    calib.dig_P7 = read_s16_le(18);
    calib.dig_P8 = read_s16_le(20);
    calib.dig_P9 = read_s16_le(22);
}



int32_t t_fine;

// returns temperature in degrees Celsius
// uses raw temperature ADC data and factory calibration coefficients
double bmp280_compensate_T_double(void) {
    double var1, var2, T;

    var1 = (((double)adc_T / 16384.0) - ((double)calib.dig_T1 / 1024.0)) * (double)calib.dig_T2;

    var2 = (((double)adc_T / 131072.0 - ((double)calib.dig_T1 / 8192.0)) * ((double)adc_T / 131072.0 - ((double)calib.dig_T1 / 8192.0))) *
    		(double)calib.dig_T3;

    // calculate fine temperature for pressure compensation
    t_fine = (int32_t)(var1 + var2);

    // calculate temperature in degrees Celsius
    T = (var1 + var2) / 5120.0;

    return T;
}


// returns atmospheric pressure in Pascals (Pa)
// uses raw pressure ADC data, calibration coefficients and t_fine
double bmp280_compensate_P_double(void) {
    double var1, var2, p;

    var1 = ((double)t_fine / 2.0) - 64000.0;
    var2 = var1 * var1 * ((double)calib.dig_P6) / 32768.0;

    var2 = var2 + var1 * ((double)calib.dig_P5) * 2.0;
    var2 = (var2 / 4.0) + (((double)calib.dig_P4) * 65536.0);

    var1 = (((double)calib.dig_P3) * var1 * var1 / 524288.0 +
            ((double)calib.dig_P2) * var1) / 524288.0;
    var1 = (1.0 + var1 / 32768.0) * ((double)calib.dig_P1);

    // avoid division by zero
    if (var1 == 0.0) {
        return 0.0;
    }

    p = 1048576.0 - (double)adc_P;
    p = (p - (var2 / 4096.0)) * 6250.0 / var1;

    var1 = ((double)calib.dig_P9) * p * p / 2147483648.0;
    var2 = p * ((double)calib.dig_P8) / 32768.0;

    p = p + (var1 + var2 + ((double)calib.dig_P7)) / 16.0;

    return p;
}


void bmp280_read_measurements(void){
    bmp280_read_raw_temp_n_pressure(raw_buffer, sizeof(raw_buffer));
}


