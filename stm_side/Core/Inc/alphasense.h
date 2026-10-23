/**
  ******************************************************************************
  * @file    alphasense.h
  * @author  Felipe Xavier
  * @version 1.0
  * @date    16-02-2024
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
#ifndef ALPHASENSE_H
#define ALPHASENSE_H
/*******************************************************************************
************************************ Includes **********************************
*******************************************************************************/
#include "main.h"
#include "stm32g4xx_hal.h"

/*******************************************************************************
********************************* Exported defines *******************************
*******************************************************************************/

/*******************************************************************************
******************************* Exported constants *****************************
*******************************************************************************/

/*******************************************************************************
********************************  typedef *******************************
*******************************************************************************/
enum sensorCode{H2S_B4, SO2_B4, VOC_B4, NH3_B1, CO_B4};

struct alphasense
{
	float ppm;
	float ppb;

	uint8_t equation;
	enum sensorCode code;
	float gain_mV_ppb;

	// valores de tensao em mV
	float WE_U_raw;   //uncorrected raw WE output (working electrode) [mV]
	float AE_U_raw;   //uncorrected raw AE output (auxiliary electrode) [mV]
	float WE_C_value;  //corrected WE output
	float WE_E_value;  //WE electronic offset on the AFE or ISB
	float AE_E_value;  //AE electronic offset on the AFE or ISB
	float WE_0_value;  //WE sensor zero, i.e. the sensor WE output in zero air
	float AE_0_value;  //AE sensor zero, i.e. the sensor AE output in zero air

	//FATORES DE CORRECAO TEMPERATURA
	float temperature;
	float NT_factor;   //temperature dependent correction factor for algorithm 1, refer to Table 3 for values
	float KT_factor;   //temperature dependent correction factor for algorithm 2, refer to Table 3 for values
	float KT1_factor; //temperature dependent correction factor for algorithm 3, refer to Table 3 for values
	float KT2_factor; //teperature dependent correction factor for algorithm 4, refer to Table 3 for values
};
typedef struct alphasense sensorAlphasense;

/*******************************************************************************
******************************* Exported functions *****************************
*******************************************************************************/
void calc_H2S_NT(sensorAlphasense *sensor);
void calc_H2S_KT(sensorAlphasense *sensor);
void calc_H2S_KT1(sensorAlphasense *sensor);
void calc_H2S_KT2(sensorAlphasense *sensor);
void calc_SO2_NT(sensorAlphasense *sensor);
void calc_SO2_KT(sensorAlphasense *sensor);
void calc_SO2_KT2(sensorAlphasense *sensor);
void calc_CO_NT(sensorAlphasense *sensor);
void calc_CO_KT(sensorAlphasense *sensor);
void calc_WEC(sensorAlphasense *sensor);
void calc_temperature_factors(sensorAlphasense *sensor);
void calc_ppm_ppb(sensorAlphasense *sensor);


//  *****************************************************************************/
#endif
/********************************* end of file ********************************/
