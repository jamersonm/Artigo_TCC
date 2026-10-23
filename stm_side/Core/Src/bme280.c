/*
  ***************************************************************************************************************
  ***************************************************************************************************************
  ***************************************************************************************************************

  File:		  BME280_STM32.c
  Author:     ControllersTech.com
  Updated:    Dec 14, 2021

  ***************************************************************************************************************
  Copyright (C) 2017 ControllersTech.com

  This is a free software under the GNU license, you can redistribute it and/or modify it under the terms
  of the GNU General Public License version 3 as published by the Free Software Foundation.
  This software library is shared with public for educational purposes, without WARRANTY and Author is not liable for any damages caused directly
  or indirectly by this software, read more about this on the GNU General Public License.

  ***************************************************************************************************************
*/

#include "bme280.h"

extern I2C_HandleTypeDef hi2c3;
#define BME280_I2C &hi2c3

//#define SUPPORT_64BIT 1
#define SUPPORT_32BIT 1

#define BME280_ADDRESS 0xEC  // SDIO is grounded, the 7 bit address is 0x76 and 8 bit address = 0x76<<1 = 0xEC

//AJUSTE CALIBRACAO PRESSAO
#define P_CALIBRACAO_AJUSTE 6066



uint8_t chipID;
uint8_t TrimParam[36];
int32_t tRaw, pRaw, hRaw;
uint16_t dig_T1,  dig_P1, dig_H1, dig_H3;
int16_t  dig_T2, dig_T3, dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9, dig_H2,  dig_H4, dig_H5, dig_H6;



// Read the Trimming parameters saved in the NVM ROM of the device
void TrimRead(void)
{
	uint8_t trimdata[32];
	// Read NVM from 0x88 to 0xA1
	HAL_I2C_Mem_Read(BME280_I2C, BME280_ADDRESS, 0x88, 1, trimdata, 25, HAL_MAX_DELAY);

	// Read NVM from 0xE1 to 0xE7
	HAL_I2C_Mem_Read(BME280_I2C, BME280_ADDRESS, 0xE1, 1, (uint8_t *)trimdata+25, 7, HAL_MAX_DELAY);

	// Arrange the data as per the datasheet (page no. 24)
	dig_T1 = (trimdata[1]<<8) | trimdata[0];
	dig_T2 = (trimdata[3]<<8) | trimdata[2];
	dig_T3 = (trimdata[5]<<8) | trimdata[4];
	dig_P1 = (trimdata[7]<<8) | trimdata[5];
	dig_P2 = (trimdata[9]<<8) | trimdata[6];
	dig_P3 = (trimdata[11]<<8) | trimdata[10];
	dig_P4 = (trimdata[13]<<8) | trimdata[12];
	dig_P5 = (trimdata[15]<<8) | trimdata[14];
	dig_P6 = (trimdata[17]<<8) | trimdata[16];
	dig_P7 = (trimdata[19]<<8) | trimdata[18];
	dig_P8 = (trimdata[21]<<8) | trimdata[20];
	dig_P9 = (trimdata[23]<<8) | trimdata[22];
	dig_H1 = trimdata[24];
	dig_H2 = (trimdata[26]<<8) | trimdata[25];
	dig_H3 = (trimdata[27]);
	dig_H4 = (trimdata[28]<<4) | (trimdata[29] & 0x0f);
	dig_H5 = (trimdata[30]<<4) | (trimdata[29]>>4);
	dig_H6 = (trimdata[31]);
}

void readOldParams()
{
	dig_T1 = 28139;
	dig_T2 = 26438;
	dig_T3 = 50;
	dig_P1 = 37632;
	dig_P2 = -10687;
	dig_P3 = 3024;
	dig_P4 = 10337;
	dig_P5 = -23;
	dig_P6 = -7;
	dig_P7 = 9900;
	dig_P8 = -10230;
	dig_P9 = 4285;
	dig_H1 = 0;
	dig_H2 = 367;
	dig_H3 = 0;
	dig_H4 = 307;
	dig_H5 = 50;
	dig_H6 = 30;
}

void read_calibration_data() {
    uint8_t calib_data[26];  // Buffer to store the calibration data

    // 1. Read temperature and pressure calibration data from 0x88 to 0xA1
    uint8_t reg = 0x88;
    HAL_I2C_Master_Transmit(BME280_I2C, BME280_ADDRESS, &reg, 1, HAL_MAX_DELAY);
    HAL_I2C_Master_Receive(BME280_I2C, BME280_ADDRESS, calib_data, 26, HAL_MAX_DELAY);

    // Store temperature calibration data
    dig_T1 = (calib_data[1] << 8) | calib_data[0];
    dig_T2 = (calib_data[3] << 8) | calib_data[2];
    dig_T3 = (calib_data[5] << 8) | calib_data[4];

    // Store pressure calibration data
    dig_P1 = (calib_data[7] << 8) | calib_data[6];
    dig_P2 = (calib_data[9] << 8) | calib_data[8];
    dig_P3 = (calib_data[11] << 8) | calib_data[10];
    dig_P4 = (calib_data[13] << 8) | calib_data[12];
    dig_P5 = (calib_data[15] << 8) | calib_data[14];
    dig_P6 = (calib_data[17] << 8) | calib_data[16];
    dig_P7 = (calib_data[19] << 8) | calib_data[18];
    dig_P8 = (calib_data[21] << 8) | calib_data[20];
    dig_P9 = (calib_data[23] << 8) | calib_data[22];

    // Read humidity calibration data from 0xA1, 0xE1 to 0xE7
    reg = 0xA1;
    HAL_I2C_Master_Transmit(BME280_I2C, BME280_ADDRESS, &reg, 1, HAL_MAX_DELAY);
    HAL_I2C_Master_Receive(BME280_I2C, BME280_ADDRESS, (uint8_t*)&dig_H1, 1, HAL_MAX_DELAY);  // dig_H1 is 1 byte

    uint8_t hum_data[7];  // Buffer for humidity data
    reg = 0xE1;
    HAL_I2C_Master_Transmit(BME280_I2C, BME280_ADDRESS, &reg, 1, HAL_MAX_DELAY);
    HAL_I2C_Master_Receive(BME280_I2C, BME280_ADDRESS, hum_data, 7, HAL_MAX_DELAY);

    // Store humidity calibration data
    dig_H2 = (hum_data[1] << 8) | hum_data[0];
    dig_H3 = hum_data[2];
    dig_H4 = (hum_data[3] << 4) | (hum_data[4] & 0x0F);
    dig_H5 = (hum_data[5] << 4) | (hum_data[4] >> 4);
    dig_H6 = (int8_t)hum_data[6];  // Signed 8-bit value
}

/* Configuration for the BME280

 * @osrs is the oversampling to improve the accuracy
 *       if osrs is set to OSRS_OFF, the respective measurement will be skipped
 *       It can be set to OSRS_1, OSRS_2, OSRS_4, etc. Check the header file
 *
 * @mode can be used to set the mode for the device
 *       MODE_SLEEP will put the device in sleep
 *       MODE_FORCED device goes back to sleep after one measurement. You need to use the BME280_WakeUP() function before every measurement
 *       MODE_NORMAL device performs measurement in the normal mode. Check datasheet page no 16
 *
 * @t_sb is the standby time. The time sensor waits before performing another measurement
 *       It is used along with the normal mode. Check datasheet page no 16 and page no 30
 *
 * @filter is the IIR filter coefficients
 *         IIR is used to avoid the short term fluctuations
 *         Check datasheet page no 18 and page no 30
 */

int BME280_Config (uint8_t osrs_t, uint8_t osrs_p, uint8_t osrs_h, uint8_t mode, uint8_t t_sb, uint8_t filter)
{
	// Read the Trimming parameters
	TrimRead();
	//read_calibration_data();
	if(dig_T1 == 0)
		readOldParams();

	uint8_t datatowrite = 0;
	uint8_t datacheck = 0;

	// Reset the device
	datatowrite = 0xB6;  // reset sequence
	if (HAL_I2C_Mem_Write(BME280_I2C, BME280_ADDRESS, RESET_REG, 1, &datatowrite, 1, 1000) != HAL_OK)
	{
		return -1;
	}

	HAL_Delay (100);


	// write the humidity oversampling to 0xF2
	datatowrite = osrs_h;
	if (HAL_I2C_Mem_Write(BME280_I2C, BME280_ADDRESS, CTRL_HUM_REG, 1, &datatowrite, 1, 1000) != HAL_OK)
	{
		return -1;
	}
	HAL_Delay (100);
	HAL_I2C_Mem_Read(BME280_I2C, BME280_ADDRESS, CTRL_HUM_REG, 1, &datacheck, 1, 1000);
	if (datacheck != datatowrite)
	{
		return -1;
	}


	// write the standby time and IIR filter coeff to 0xF5
	datatowrite = (t_sb <<5) |(filter << 2);
	if (HAL_I2C_Mem_Write(BME280_I2C, BME280_ADDRESS, CONFIG_REG, 1, &datatowrite, 1, 1000) != HAL_OK)
	{
		return -1;
	}
	HAL_Delay (100);
	HAL_I2C_Mem_Read(BME280_I2C, BME280_ADDRESS, CONFIG_REG, 1, &datacheck, 1, 1000);
	if (datacheck != datatowrite)
	{
		return -1;
	}


	// write the pressure and temp oversampling along with mode to 0xF4
	datatowrite = (osrs_t <<5) |(osrs_p << 2) | mode;
	if (HAL_I2C_Mem_Write(BME280_I2C, BME280_ADDRESS, CTRL_MEAS_REG, 1, &datatowrite, 1, 1000) != HAL_OK)
	{
		return -1;
	}
	HAL_Delay (100);
	HAL_I2C_Mem_Read(BME280_I2C, BME280_ADDRESS, CTRL_MEAS_REG, 1, &datacheck, 1, 1000);
	if (datacheck != datatowrite)
	{
		return -1;
	}

	return 0;
}


int BMEReadRaw(void)
{
	uint8_t RawData[8];

	// Check the chip ID before reading
	HAL_I2C_Mem_Read(&hi2c3, BME280_ADDRESS, ID_REG, 1, &chipID, 1, 1000);

	if (chipID == 0x60)
	{
		// Read the Registers 0xF7 to 0xFE
		HAL_I2C_Mem_Read(BME280_I2C, BME280_ADDRESS, PRESS_MSB_REG, 1, RawData, 8, HAL_MAX_DELAY);

		/* Calculate the Raw data for the parameters
		 * Here the Pressure and Temperature are in 20 bit format and humidity in 16 bit format
		 */
		pRaw = (RawData[0]<<12)|(RawData[1]<<4)|(RawData[2]>>4);
		tRaw = (RawData[3]<<12)|(RawData[4]<<4)|(RawData[5]>>4);
		hRaw = (RawData[6]<<8)|(RawData[7]);

		return 0;
	}

	else return -1;
}

/* To be used when doing the force measurement
 * the Device need to be put in forced mode every time the measurement is needed
 */
void BME280_WakeUP(void)
{
	uint8_t datatowrite = 0;

	// first read the register
	HAL_I2C_Mem_Read(BME280_I2C, BME280_ADDRESS, CTRL_MEAS_REG, 1, &datatowrite, 1, 1000);

	// modify the data with the forced mode
	datatowrite = datatowrite | MODE_FORCED;

	// write the new data to the register
	HAL_I2C_Mem_Write(BME280_I2C, BME280_ADDRESS, CTRL_MEAS_REG, 1, &datatowrite, 1, 1000);

	HAL_Delay (100);
}

/************* COMPENSATION CALCULATION AS PER DATASHEET (page 25) **************************/

/* Returns temperature in DegC, resolution is 0.01 DegC. Output value of “5123” equals 51.23 DegC.
   t_fine carries fine temperature as global value
*/
int32_t t_fine;
float BME280_compensate_T_int32(int32_t adc_T)
{

	int32_t var1, var2, T;
	var1 = ((((adc_T>>3) - ((int32_t)dig_T1<<1))) * ((int32_t)dig_T2)) >> 11;
	var2 = (((((adc_T>>4) - ((int32_t)dig_T1)) * ((adc_T>>4) - ((int32_t)dig_T1)))>> 12) *((int32_t)dig_T3)) >> 14;
	t_fine = var1 + var2;
	T = (t_fine * 5 + 128) >> 8;
	return T/ 100.0;


	/*
	int32_t var1, var2;

    if (adc_T == 0x800000) // value in case temp measurement was disabled
    	return -500;

    adc_T >>= 4;

    var1 = (int32_t)((adc_T / 8) - ((int32_t)dig_T1* 2));
    var1 = (var1 * ((int32_t)dig_T2)) / 2048;
    var2 = (int32_t)((adc_T / 16) - ((int32_t)dig_T1));
    var2 = (((var2 * var2) / 4096) * ((int32_t)dig_T3)) / 16384;

    //t_fine = var1 + var2 + t_fine_adjust;
    t_fine = var1 + var2;

    int32_t T = (t_fine * 5 + 128) / 256;

    return (float)T / 100;
    */
}


#if SUPPORT_64BIT
/* Returns pressure in Pa as unsigned 32 bit integer in Q24.8 format (24 integer bits and 8 fractional bits).
   Output value of “24674867” represents 24674867/256 = 96386.2 Pa = 963.862 hPa
*/
float BME280_compensate_P_int64(int32_t adc_P)
{
	/*
	int64_t var1, var2, p;
	var1 = ((int64_t)t_fine) - 128000;
	var2 = var1 * var1 * (int64_t)dig_P6;
	var2 = var2 + ((var1*(int64_t)dig_P5)<<17);
	var2 = var2 + (((int64_t)dig_P4)<<35);
	var1 = ((var1 * var1 * (int64_t)dig_P3)>>8) + ((var1 * (int64_t)dig_P2)<<12);
	var1 = (((((int64_t)1)<<47)+var1))*((int64_t)dig_P1)>>33;
	if (var1 == 0)
	{
		return 0; // avoid exception caused by division by zero
	}
	p = 1048576-adc_P;
	p = (((p<<31)-var2)*3125)/var1;
	var1 = (((int64_t)dig_P9) * (p>>13) * (p>>13)) >> 25;
	var2 = (((int64_t)dig_P8) * p) >> 19;
	p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7)<<4);

	//return (uint32_t)p;
	return p/256.0;
	*/

	/*
	 int64_t var1, var2, var3, var4;

	 BME280_compensate_T_int32(tRaw); // must be done first to get t_fine

	 if (adc_P == 0x800000) // value in case pressure measurement was disabled
		return 0;

	 adc_P >>= 4;

	 var1 = ((int64_t)t_fine) - 128000;
	 var2 = var1 * var1 * (int64_t)dig_P6;
	 var2 = var2 + ((var1 * (int64_t)dig_P5) * 131072);
	 var2 = var2 + (((int64_t)dig_P4) * 34359738368);
	 var1 = ((var1 * var1 * (int64_t)dig_P3) / 256) + ((var1 * ((int64_t)dig_P2) * 4096));
	 var3 = ((int64_t)1) * 140737488355328;
	 var1 = (var3 + var1) * ((int64_t)dig_P1) / 8589934592;

	 if (var1 == 0) {
		return 0; // avoid exception caused by division by zero
	  }

	 var4 = 1048576 - adc_P;
	 var4 = (((var4 * 2147483648) - var2) * 3125) / var1;
	 var1 = (((int64_t)dig_P9) * (var4 / 8192) * (var4 / 8192)) / 33554432;
	 var2 = (((int64_t)dig_P8) * var4) / 524288;
	 var4 = ((var4 + var1 + var2) / 256) + (((int64_t)dig_P7) * 16);

	 float P = var4 / 256.0;

	 return P;
	 */

	int64_t var1, var2, p;

	var1 = ((int64_t)t_fine) - 128000;
	var2 = var1 * var1 * (int64_t)dig_P6;
	var2 = var2 + ((var1 * (int64_t)dig_P5) << 17);
	var2 = var2 + (((int64_t)dig_P4) << 35);
	var1 = ((var1 * var1 * (int64_t)dig_P3) >> 8) + ((var1 * (int64_t)dig_P2) << 12);
	var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)dig_P1) >> 33;

	if (var1 == 0) {
		return 0;  // Avoid division by zero
	}

	p = 1048576 - adc_P;
	p = (((p << 31) - var2) * 3125) / var1;
	var1 = (((int64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
	var2 = (((int64_t)dig_P8) * p) >> 19;

	p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);

	return (uint32_t)p;  // This is the compensated pressure in Pa (Pascal)


}

#elif SUPPORT_32BIT
// Returns pressure in Pa as unsigned 32 bit integer. Output value of “96386” equals 96386 Pa = 963.86 hPa
uint32_t BME280_compensate_P_int32(int32_t adc_P)
{
	int32_t var1, var2;
	uint32_t p;
	var1 = (((int32_t)t_fine)>>1) - (int32_t)64000;
	var2 = (((var1>>2) * (var1>>2)) >> 11 ) * ((int32_t)dig_P6);
	var2 = var2 + ((var1*((int32_t)dig_P5))<<1);
	var2 = (var2>>2)+(((int32_t)dig_P4)<<16);
	var1 = (((dig_P3 * (((var1>>2) * (var1>>2)) >> 13 )) >> 3) + ((((int32_t)dig_P2) *var1)>>1))>>18;
	var1 =((((32768+var1))*((int32_t)dig_P1))>>15);
	if (var1 == 0)
	{
		return 0; // avoid exception caused by division by zero
	}
	p = (((uint32_t)(((int32_t)1048576)-adc_P)-(var2>>12)))*3125;
	if (p < 0x80000000)
	{
		p = (p << 1) / ((uint32_t)var1);
	}
	else
	{
		p = (p / (uint32_t)var1) * 2;
	}
	var1 = (((int32_t)dig_P9) * ((int32_t)(((p>>3) * (p>>3))>>13)))>>12;
	var2 = (((int32_t)(p>>2)) * ((int32_t)dig_P8))>>13;
	p = (uint32_t)((int32_t)p + ((var1 + var2 + dig_P7) >> 4));

	return p + P_CALIBRACAO_AJUSTE;
}
#endif

/* Returns humidity in %RH as unsigned 32 bit integer in Q22.10 format (22 integer and 10 fractional bits).
   Output value of “47445” represents 47445/1024 = 46.333 %RH
*/
float bme280_compensate_H_int32(int32_t adc_H)
{
	/*
	int32_t v_x1_u32r;
	v_x1_u32r = (t_fine - ((int32_t)76800));
	v_x1_u32r = (((((adc_H << 14) - (((int32_t)dig_H4) << 20) - (((int32_t)dig_H5) *\
			v_x1_u32r)) + ((int32_t)16384)) >> 15) * (((((((v_x1_u32r *\
					((int32_t)dig_H6)) >> 10) * (((v_x1_u32r * ((int32_t)dig_H3)) >> 11) +\
							((int32_t)32768))) >> 10) + ((int32_t)2097152)) * ((int32_t)dig_H2) +\
					8192) >> 14));
	v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *\
			((int32_t)dig_H1)) >> 4));
	v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
	v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
	return (uint32_t)(v_x1_u32r>>12);
	*/

	int32_t var1, var2, var3, var4, var5;

	BME280_compensate_T_int32(tRaw); // must be done first to get t_fine

	if (adc_H == 0x8000) // value in case humidity measurement was disabled
	    return -50;

	var1 = t_fine - ((int32_t)76800);
	var2 = (int32_t)(adc_H * 16384);
	var3 = (int32_t)(((int32_t)dig_H4) * 1048576);
	var4 = ((int32_t)dig_H5) * var1;
	var5 = (((var2 - var3) - var4) + (int32_t)16384) / 32768;
	var2 = (var1 * ((int32_t)dig_H6)) / 1024;
	var3 = (var1 * ((int32_t)dig_H3)) / 2048;
	var4 = ((var2 * (var3 + (int32_t)32768)) / 1024) + (int32_t)2097152;
	var2 = ((var4 * ((int32_t)dig_H2)) + 8192) / 16384;
	var3 = var5 * var2;
	var4 = ((var3 / 32768) * (var3 / 32768)) / 128;
	var5 = var3 - ((var4 * ((int32_t)dig_H1)) / 16);
	var5 = (var5 < 0 ? 0 : var5);
	var5 = (var5 > 419430400 ? 419430400 : var5);
	uint32_t H = (uint32_t)(var5 / 4096);

	return (float)H / 1024.0;
}
/*********************************************************************************************************/

/* measure the temp, pressure and humidity
 * the values will be stored in the parameters passed to the function
 */
void BME280_Measure (BME280 *bme)
{
	if (BMEReadRaw() == 0)
	{
		  if (tRaw == 0x800000)
			  bme->temp = 0; // value in case temp measurement was disabled
		  else
		  {
			  //bme->temp = (BME280_compensate_T_int32 (tRaw))/100.0;  // as per datasheet, the temp is x100
			  bme->temp = BME280_compensate_T_int32 (tRaw);
		  }

		  if (pRaw == 0x800000)
			  bme->pres = 0; // value in case temp measurement was disabled
		  else
		  {
#if SUPPORT_64BIT
			  bme->pres = (BME280_compensate_P_int64 (pRaw));  // as per datasheet, the pressure is x256
			  //bme->pres = bme->pres/101325;

#elif SUPPORT_32BIT
			  bme->pres = (BME280_compensate_P_int32 (pRaw));  // as per datasheet, the pressure is Pa
			  bme->pres_atm = bme->pres/101300.0;
#endif
		  }

		  if (hRaw == 0x8000)
			  bme->humid = 0; // value in case temp measurement was disabled
		  else
		  {
			  //bme->humid = (bme280_compensate_H_int32 (hRaw))/1024.0;  // as per datasheet, the temp is x1024
			  bme->humid = bme280_compensate_H_int32 (hRaw);
		  }
	}


	// if the device is detached
	else
	{
		bme->temp = bme->pres = bme->humid = 0;
	}
}
