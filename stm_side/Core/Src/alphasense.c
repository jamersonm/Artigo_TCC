/**
  ******************************************************************************
  * @file    alphasense.c
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
************************************ Includes **********************************
*******************************************************************************/
#include "alphasense.h"

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

void calc_H2S_NT(sensorAlphasense *sensor)
{
	if(sensor->temperature <= -30) {
		sensor->NT_factor = -0.6;
		return;
	}

	if(sensor->temperature >= 50) {
		sensor->NT_factor = -1.8;
		return;
	}

	float x = sensor->temperature;

	sensor->NT_factor = -    2.48e-13*x*x*x*x*x*x*x*x
						+ 		2e-10*x*x*x*x*x*x*x
						-    1.28e-08*x*x*x*x*x*x
						- 	  2.1e-07*x*x*x*x*x
						+    2.36e-05*x*x*x*x
						+    3.78e-05*x*x*x
						-    1.32e-02*x*x
						-	 4.18e-02*x + 8e-01;
}

void calc_H2S_KT(sensorAlphasense *sensor)
{
	if(sensor->temperature <= -30) {
		sensor->KT_factor = 0.2;
		return;
	}

	if(sensor->temperature >= 50) {
		sensor->KT_factor = 0.7;
		return;
	}

	float x = sensor->temperature;

	sensor->KT_factor =  + 1.48e-10*x*x*x*x*x*x
			   	   	   	 + 2.32e-08*x*x*x*x*x
						 - 2.02e-06*x*x*x*x
						 - 3.01e-05*x*x*x
						 + 2.61e-03*x*x
						 + 2.39e-02*x - 1.628e-01;
}

void calc_H2S_KT1(sensorAlphasense *sensor)
{
	if(sensor->temperature <= -30) {
		sensor->KT1_factor = -14;
		return;
	}

	if(sensor->temperature >= 50) {
		sensor->KT1_factor = -1.2;
		return;
	}

	float x = sensor->temperature;

	sensor->KT1_factor = + 1.86e-08*x*x*x*x*x*x
			  	  	  	 - 1.52e-06*x*x*x*x*x
			             + 7.62e-06*x*x*x*x
			             + 1.79e-03*x*x*x
			             - 3.51e-02*x*x
						 - 1.49e-01*x + 4.8636;
}

void calc_H2S_KT2(sensorAlphasense *sensor)
{
	if(sensor->temperature <= -30) {
		sensor->KT2_factor = 52;
		return;
	}

	if(sensor->temperature >= 50) {
		sensor->KT2_factor = -180;
		return;
	}

	float x = sensor->temperature;

	sensor->KT2_factor =   - 5.6829e-02*x*x - 1.7418*x + 44.5;
}




void calc_SO2_NT(sensorAlphasense *sensor)
{
	if(sensor->temperature <= -30) {
		sensor->NT_factor = 1.6;
		return;
	}

	if(sensor->temperature >= 50) {
		sensor->NT_factor = 5.8;
		return;
	}

	float x = sensor->temperature;

	sensor->NT_factor = +  7.6e-07*x*x*x*x
			            + 7.54e-06*x*x*x
			            - 5.79e-04*x*x
						- 2.52e-03*x + 1.6563;
}

void calc_SO2_KT(sensorAlphasense *sensor)
{
	if(sensor->temperature <= -30) {
		sensor->KT_factor = 1;
		return;
	}

	if(sensor->temperature >= 50) {
		sensor->KT_factor = 3.6;
		return;
	}

	float x = sensor->temperature;

	sensor->KT_factor = + 4.458e-07*x*x*x*x
			   	   	    + 5.316e-06*x*x*x
						- 3.206e-04*x*x
						- 1.781e-03*x + 1.0296;
}

void calc_SO2_KT2(sensorAlphasense *sensor)
{
	if(sensor->temperature <= -30) {
		sensor->KT2_factor = -4;
		return;
	}

	if(sensor->temperature >= 50) {
		sensor->KT2_factor = 450;
		return;
	}

	float x = sensor->temperature;

	sensor->KT2_factor = + 9.0443e-05*x*x*x*x
			   	   	   	 + 5.9104e-04*x*x*x
						 - 7.4911e-02*x*x
						 - 1.7387e-01*x + 4.0839;
}

void calc_CO_NT(sensorAlphasense *sensor)
{
	if(sensor->temperature <= -30) {
		sensor->NT_factor = 0.7;
		return;
	}

	if(sensor->temperature >= 50) {
		sensor->NT_factor = 4.5;
		return;
	}

	float x = sensor->temperature;

	sensor->NT_factor = + 1.5741e-09*x*x*x*x*x*x
			  	  	  	- 5.9829e-08*x*x*x*x*x
						- 3.7678e-06*x*x*x*x
						+ 8.5179e-05*x*x*x
						+ 3.9007e-03*x*x
						+ 2.2908e-02*x + 6.2059e-01;
}

void calc_CO_KT(sensorAlphasense *sensor)
{
	if(sensor->temperature <= -30) {
		sensor->KT_factor = 0.2;
		return;
	}

	if(sensor->temperature >= 50) {
		sensor->KT_factor = 1.5;
		return;
	}

	float x = sensor->temperature;

	sensor->KT_factor =  + 6.7593e-10*x*x*x*x*x*x
			  	  	  	 - 2.6774e-08*x*x*x*x*x
						 - 1.5046e-06*x*x*x*x
						 + 3.6945e-05*x*x*x
						 + 1.4432e-03*x*x
						 + 6.4357e-03*x + 1.6552e-01;
}

void calc_WEC(sensorAlphasense *sensor)
{
	/*
	 * VALORES DE TENSAO EM mV
	 */
	switch(sensor->equation)
	{
		case 1:
			//WEC = (WE_U - WE_E) - NT*(AE_U-AE_E)
			sensor->WE_C_value = (sensor->WE_U_raw - sensor->WE_E_value) -sensor->NT_factor*(sensor->AE_U_raw - sensor->AE_E_value);
		break;
		case 2:
			//WEC = (WE_U - WE_E) - KT*(WE_0/AE_0)*(AE_U/AE_E)
			sensor->WE_C_value = (sensor->WE_U_raw - sensor->WE_E_value) - sensor->KT_factor*(sensor->WE_0_value/sensor->AE_0_value)*(sensor->AE_U_raw - sensor->AE_E_value);
		break;
		case 3:
			//WEC = (WE_U - WE_E) - (WE_0 - AE_0) - KT1*(AE_U-AE_E)
			sensor->WE_C_value = (sensor->WE_U_raw - sensor->WE_E_value) - (sensor->WE_0_value - sensor->AE_0_value) - sensor->KT1_factor*(sensor->AE_U_raw - sensor->AE_E_value);
		break;
		case 4:
			//WEC = (WE_U - WE_E) - WE_0 - KT2
			sensor->WE_C_value = (sensor->WE_U_raw - sensor->WE_E_value) - sensor->WE_0_value - sensor->KT2_factor;
		break;
		default:
			sensor->WE_C_value = -1;
	}
}

void calc_temperature_factors(sensorAlphasense *sensor)
{
	if(sensor->code == H2S_B4)
	{
		if(sensor->equation == 1)
			calc_H2S_NT(sensor);
		else if(sensor->equation == 2)
			calc_H2S_KT(sensor);

		return;
	}

	if(sensor->code == SO2_B4)
	{
		if(sensor->equation == 1)
			calc_SO2_NT(sensor);
		else if(sensor->equation == 2)
			calc_SO2_KT(sensor);
		else if(sensor->equation == 4)
			calc_SO2_KT2(sensor);

		return;
	}

	if(sensor->code == CO_B4)
	{
		if(sensor->equation == 1)
			calc_CO_NT(sensor);
		else if(sensor->equation == 2)
			calc_CO_KT(sensor);

		return;
	}

	if(sensor->code == NH3_B1)
	{
		sensor->NT_factor = 0;
		sensor->KT_factor = 0;
		sensor->KT1_factor = 0;
		sensor->KT2_factor = 0;

		return;
	}

	if(sensor->code == VOC_B4)
	{
		sensor->NT_factor = 0;
		sensor->KT_factor = 0;
		sensor->KT1_factor = 0;
		sensor->KT2_factor = 0;

		return;
	}
}

void calc_ppm_ppb(sensorAlphasense *sensor)
{
	sensor->ppb = 0;
	sensor->ppm = 0;

	calc_temperature_factors(sensor);
	calc_WEC(sensor);

	if(sensor->WE_C_value < 0)
		return;

	sensor->ppb = sensor->WE_C_value/sensor->gain_mV_ppb;
	sensor->ppm = sensor->WE_C_value/(sensor->gain_mV_ppb*1000);
}





