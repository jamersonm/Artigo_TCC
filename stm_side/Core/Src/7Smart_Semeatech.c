/**
  ******************************************************************************
  * @file    7Smart_Semeatech.c
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
************************************ Includes **********************************
*******************************************************************************/
#include "7Smart_Semeatech.h"
#include <stdio.h>


/*******************************************************************************
******************************** Private typedef *******************************
*******************************************************************************/

/*******************************************************************************
******************************** Private define ********************************
*******************************************************************************/

/*******************************************************************************
********************************* Private macro ********************************
*******************************************************************************/

/*******************************************************************************
******************************* Private variables ******************************
*******************************************************************************/





/*******************************************************************************
******************************* Private functions ******************************
*******************************************************************************/

void findAddrI2C(I2C_HandleTypeDef *hi2c)
{
	char buffer[10];
	//uint8_t Space[] = " - ";
	//uint8_t StartMSG[] = "\r\n Starting I2C Scanning: \r\n";
	//uint8_t EndMSG[] = "\r\n Done! \r\n\r\n";

    uint8_t i = 0, ret;

    //HAL_UART_Transmit(huart, StartMSG, sizeof(StartMSG), 10000);

    for(i=1; i<128; i++)
    {
    	ret = HAL_I2C_IsDeviceReady(hi2c, (uint16_t)(i<<1), 3, 5);
        if (ret != HAL_OK)
        {
        	//HAL_UART_Transmit(huart, Space, sizeof(Space), 10000);
        }
        else if(ret == HAL_OK) {
            sprintf(buffer, "0x%x", i);
            //HAL_UART_Transmit(huart, (uint8_t *)buffer, sizeof(buffer), 10000);
         }
     }

    //HAL_UART_Transmit(huart, EndMSG, sizeof(EndMSG), 10000);
}


unsigned short modbus_CRC16(unsigned char *ptr,unsigned char len)
{
	unsigned short wcrc=0XFFFF;
	int i=0,j=0;
	for(i=0;i<len;i++)
	{
	   wcrc^=*ptr++;
	   for(j=0;j<8;j++)
	   {
		   if(wcrc&0X0001)
				wcrc=wcrc>>1^0XA001;
		   else
				wcrc>>=1;

		}
	}

	return wcrc<<8|wcrc>>8;  //little endian (LSB fist)
}

unsigned char CheckSum(unsigned char *buf, unsigned char len)
{
    uint8_t i, ret = 0;

    for(i=0; i<len; i++)
        ret += *(buf++);

    ret = ~ret;
    return ret;
}


signed int read_ug_value(I2C_HandleTypeDef *hi2c, uint16_t I2C_address, uint8_t timeout)
{
	unsigned char I2C_Receive[5];
	signed int gas_ug_value;
	uint8_t t = 0;

	while(HAL_I2C_Mem_Read(hi2c, I2C_address << 1, ug_address, 2, I2C_Receive, 5, 100) != HAL_OK && t <= timeout)
	{
		HAL_Delay(1000);
		t++;
	}

	if(t > timeout)
		return -1;

	if(CheckSum(I2C_Receive, 5) == 0)
		gas_ug_value=(signed int)(I2C_Receive[0] * 0x1000000 + I2C_Receive[1] * 0x10000 + I2C_Receive[2] * 0x100 + I2C_Receive[3]);

	return gas_ug_value;
}


signed int read_ppb_value(I2C_HandleTypeDef *hi2c, uint16_t I2C_address, uint8_t timeout)
{
	unsigned char I2C_Receive[5];
	signed int gas_ug_value;
	uint8_t t = 0;

	while(HAL_I2C_Mem_Read(hi2c, I2C_address << 1, ppb_address, 2, I2C_Receive, 5, 100) != HAL_OK && t <= timeout)
	{
		HAL_Delay(1000);
		t++;
	}

	if(t > timeout)
		return -1;

	if(CheckSum(I2C_Receive, 5) == 0)
		gas_ug_value=(signed int)(I2C_Receive[0] * 0x1000000 + I2C_Receive[1] * 0x10000 + I2C_Receive[2] * 0x100 + I2C_Receive[3]);

	return gas_ug_value;
}

float read_temperature(I2C_HandleTypeDef *hi2c, uint16_t I2C_address, uint8_t timeout)
{
	unsigned char I2C_Receive[3];
	float temperature;
	uint8_t t = 0;

	while(HAL_I2C_Mem_Read(hi2c,I2C_address << 1,temperature_address,2,I2C_Receive,3,100)!=HAL_OK && t <= timeout)
	{
		HAL_Delay(1000);
		t++;
	}

	if(t > timeout)
		return -1;

	if(CheckSum(I2C_Receive, 3)==0)
		temperature=(float)(I2C_Receive[0] * 0x100 + I2C_Receive[1])/100;

	return temperature;
}


float read_humidity(I2C_HandleTypeDef *hi2c, uint16_t I2C_address, uint8_t timeout)
{
	unsigned char I2C_Receive[3];
	float humidity;
	uint8_t t = 0;

	while(HAL_I2C_Mem_Read(hi2c,I2C_address << 1,humidity_address,2,I2C_Receive,3,100)!=HAL_OK && t <= timeout)
	{
		HAL_Delay(1000);
		t++;
	}

	if(t > timeout)
		return -1;

	if(CheckSum(I2C_Receive, 3)==0)
		humidity=(float)(I2C_Receive[0] * 0x100 + I2C_Receive[1])/100;

	return humidity;
}


void clearSensor(sensorSematech *sensor)
{
	sensor->ug = 0;
	sensor->ppb = 0;
	sensor->temperature = 0;
	sensor->humidity = 0;
}

uint8_t read_sensor(I2C_HandleTypeDef *hi2c, sensorSematech *sensor)
{
	clearSensor(sensor);

	//uint8_t ret = HAL_I2C_IsDeviceReady(hi2c, (uint16_t)(sensor->I2C_address << 1), 3, 5);
	//if(ret != HAL_OK)
	//	return -1;

	while(HAL_I2C_IsDeviceReady(hi2c, (uint16_t)(sensor->I2C_address << 1), 3, 5) != HAL_OK );

	sensor->ug = read_ug_value(hi2c, sensor->I2C_address, 3);
	HAL_Delay(100);
	sensor->ppb = read_ppb_value(hi2c, sensor->I2C_address, 3);
	HAL_Delay(100);
	sensor->temperature = read_temperature(hi2c, sensor->I2C_address, 3);
	HAL_Delay(100);
	sensor->humidity = read_humidity(hi2c, sensor->I2C_address, 3);

	return 1;
}

uint8_t read_NH3_I2C(I2C_HandleTypeDef *hi2c, sensorSematech *sensor)
{
	sensor->I2C_address = NH3_I2C_address;

	return read_sensor(hi2c, sensor);
}

uint8_t read_CH3SH_I2C(I2C_HandleTypeDef *hi2c, sensorSematech *sensor)
{
	sensor->I2C_address = CH3SH_I2C_address;

	return read_sensor(hi2c, sensor);
}

uint8_t read_SO2_I2C(I2C_HandleTypeDef *hi2c, sensorSematech *sensor)
{
	sensor->I2C_address = SO2_I2C_address;

	return read_sensor(hi2c, sensor);
}

uint8_t read_H2S_I2C(I2C_HandleTypeDef *hi2c, sensorSematech *sensor)
{
	sensor->I2C_address = H2S_I2C_address;

	return read_sensor(hi2c, sensor);
}

uint8_t read_VOC_I2C(I2C_HandleTypeDef *hi2c, sensorSematech *sensor)
{
	sensor->I2C_address = VOC_I2C_address;

	return read_sensor(hi2c, sensor);
}

/********************************* end of file ********************************/
