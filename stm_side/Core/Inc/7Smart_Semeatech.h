/**
  ******************************************************************************
  * @file    7Smart_Semeatech.h
  * @author  Felipe Xavier
  * @version 1.0
  * @date    14-02-2024
  * @brief
  *
  *
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */

/*******************************************************************************
********************* Define to prevent recursive inclusion ********************
*******************************************************************************/
#ifndef _7SMART_SEMEATECH_H
#define _7SMART_SEMEATECH_H

/*******************************************************************************
************************************ Includes **********************************
*******************************************************************************/
#include "main.h"
#include "stm32g4xx_hal.h"
/*******************************************************************************
********************************* Exported defines *******************************
*******************************************************************************/

#define ug_address   			0x0000
#define ppb_address  			0x0002
#define temperature_address 	0x0004
#define humidity_address		0x0005

#define NH3_I2C_address			0x08
#define CH3SH_I2C_address		0x15
#define SO2_I2C_address			0x05
#define H2S_I2C_address			0x03
#define VOC_I2C_address			0x16




/*******************************************************************************
******************************* Exported constants *****************************
*******************************************************************************/

/*******************************************************************************
********************************  typedef *******************************
*******************************************************************************/
struct semeatech
{
	uint16_t I2C_address;
	signed int ppb;
	signed int ug;
	float temperature;
	float humidity;
};
typedef struct semeatech sensorSematech;

/*******************************************************************************
******************************* Exported functions *****************************
*******************************************************************************/
unsigned short modbus_CRC16(unsigned char *ptr,unsigned char len);
unsigned char CheckSum(unsigned char *buf, unsigned char len);
void findAddrI2C(I2C_HandleTypeDef *hi2c);

signed int read_ug_value(I2C_HandleTypeDef *hi2c, uint16_t I2C_address, uint8_t timeout);
signed int read_ppb_value(I2C_HandleTypeDef *hi2c, uint16_t I2C_address, uint8_t timeout);
float read_temperature(I2C_HandleTypeDef *hi2c, uint16_t I2C_address, uint8_t timeout);
float read_humidity(I2C_HandleTypeDef *hi2c, uint16_t I2C_address, uint8_t timeout);

uint8_t read_sensor(I2C_HandleTypeDef *hi2c, sensorSematech *sensor);
uint8_t read_NH3_I2C(I2C_HandleTypeDef *hi2c, sensorSematech *sensor);
uint8_t read_CH3SH_I2C(I2C_HandleTypeDef *hi2c, sensorSematech *sensor);
uint8_t read_SO2_I2C(I2C_HandleTypeDef *hi2c, sensorSematech *sensor);
uint8_t read_H2S_I2C(I2C_HandleTypeDef *hi2c, sensorSematech *sensor);
uint8_t read_VOC_I2C(I2C_HandleTypeDef *hi2c, sensorSematech *sensor);
void clearSensor(sensorSematech *sensor);


//  *****************************************************************************/


#endif

/********************************* end of file ********************************/
