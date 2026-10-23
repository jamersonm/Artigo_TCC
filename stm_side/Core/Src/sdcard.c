/**
  ******************************************************************************
  * @file    sdcard.c
  * @author  Felipe Xavier
  * @version 1.0
  * @date    19-04-2024
  * @brief
  *
  *  ******************************************************************************
  * @attention
  *
  **/

/*******************************************************************************
************************************ Includes **********************************
*******************************************************************************/
#include "sdcard.h"
#include "string.h"
#include "app_fatfs.h"
#include "stdio.h"

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
/*int initFileMedidas(char *sArquivo)
{
	FATFS FatFs; 	//Fatfs handle
	FIL fil; 		//File handle
	FRESULT fres;   //Result after operations

	char header[200] = "TIME; D0; D1; D2; D3; D4; D5; D6; D7; D8; D9; D10; D11; D12; D13; D14; D15; D16; D17; D18; D19; D20; D21; D22; D23; D24; D25; D26; D27; D28; D29; D30;\r\n";

	//Open the file system
	fres = f_mount(&FatFs, "", 1); //1=mount now
	if (fres != FR_OK)
		return -1;

	if(f_open(&fil, sArquivo, FA_READ) == FR_OK)
		return +2;

	//Copy in a string
	BYTE readBuf[sizeof(header)];

	fres = f_open(&fil, sArquivo, FA_WRITE | FA_OPEN_ALWAYS | FA_CREATE_ALWAYS);
	if(fres == FR_OK) {
		strncpy((char*)readBuf, header, strlen(header));
		UINT bytesWrote;
		fres = f_write(&fil, readBuf, strlen(header), &bytesWrote);
	}

	//Be a tidy kiwi - don't forget to close your file!
	f_close(&fil);

	//We're done, so de-mount the drive
	f_mount(NULL, "", 0);
	return +1;
}

void appendFileMedidas(char *sArquivo, DATASDCARD *medidas)
{
	FATFS FatFs; 	//Fatfs handle
	FIL fil; 		//File handle
	FRESULT fres;   //Result after operations

	char message[512];
	int pos = 0;

	//Open the file system
	fres = f_mount(&FatFs, "", 1); //1=mount now
	if (fres != FR_OK) {
		while(1);
	}

	//VERIFICA SE O RTC ESTA FUNCIONANDO...ADICIONA HORA E DATA
	if(medidas->hrtc != NULL)
	{
		RTC_DateTypeDef rtc_data;
		RTC_TimeTypeDef rtc_time;

		HAL_RTC_GetDate(medidas->hrtc, &rtc_data, RTC_FORMAT_BIN);
		HAL_RTC_GetTime(medidas->hrtc, &rtc_time, RTC_FORMAT_BIN);

		pos = sprintf(message, "%d/%d/%d %d:%d:%d;", rtc_data.Date, rtc_data.Month, rtc_data.Year,
				rtc_time.Hours, rtc_time.Minutes, rtc_time.Seconds);
	}
	else {
		pos = sprintf(message, " - ;");
	}

	//ADICIONA MEDIDAS....
	for(int i=0; i<medidas->tam; i++){
        // compilador esta reclamando de .f no sprintf, valores vazios no registro
		//pos += sprintf(message+pos,"%.3f;", medidas->data[i]);
        int valor_escala = (int)(medidas->data[i] * 1000.0f);
        pos += sprintf(message + pos, "%d.%03d;", valor_escala / 1000, abs(valor_escala % 1000));
    }
	sprintf(message+pos,"\r\n");

	//Copy in a string
	BYTE readBuf[sizeof(message)];

	//Now let's try and write a file "write.txt"
	fres = f_open(&fil, sArquivo, FA_OPEN_APPEND | FA_WRITE);
	if(fres == FR_OK) {
		strncpy((char*)readBuf, message, strlen(message));
		UINT bytesWrote;
		fres = f_write(&fil, readBuf, strlen(message), &bytesWrote);
	}

	//Be a tidy kiwi - don't forget to close your file!
	f_close(&fil);

	//We're done, so de-mount the drive
	f_mount(NULL, "", 0);
}*/

int initFileMedidas(char *sArquivo)
{
    // Tornar static tira esses ~560 bytes do Stack e joga para a RAM global (.bss)
    static FATFS FatFs; 
    FIL fil;
    FRESULT fres;
    UINT bytesWrote;

    // Cabeçalho constante economiza RAM e Stack
    const char *header = "TIME;D0;D1;D2;D3;D4;D5;D6;D7;D8;D9;D10;D11;D12;D13;D14;D15;D16;D17;D18;D19;D20;D21;D22;D23;D24;D25;D26;D27;D28;D29;D30;\r\n";

    // 1. Monta o SD
    fres = f_mount(&FatFs, "", 1);
    if (fres != FR_OK) return -1;

    // 2. Verifica se o arquivo já existe (para não sobrescrever dados antigos)
    // Se abrir em FA_READ com sucesso, o arquivo já existe.
    if (f_open(&fil, sArquivo, FA_READ) == FR_OK) {
        f_close(&fil);
        f_mount(NULL, "", 0); // Sempre desmonte antes de sair
        return 2; 
    }

    // 3. Se não existe, cria um novo
    fres = f_open(&fil, sArquivo, FA_CREATE_ALWAYS | FA_WRITE);
    if (fres == FR_OK) {
        // Grava direto do ponteiro header, sem precisar de readBuf
        fres = f_write(&fil, header, strlen(header), &bytesWrote);
        f_close(&fil);
    }

    // 4. Desmonta
    f_mount(NULL, "", 0);
    
    return (fres == FR_OK) ? 1 : -1;
}

void appendFileMedidas(char *sArquivo, DATASDCARD *medidas)
{
    // static para tirar do Stack e colocar na RAM global
    static FATFS FatFs; 
    FIL fil;
    FRESULT fres;

    // Aumentamos o buffer, mas ele fica no Stack. 
    // Se o sistema travar, mude para 'static char message[512];'
    static char message[512]; 
    int pos = 0;

    // 1. Monta o sistema de arquivos
    fres = f_mount(&FatFs, "", 1);
    if (fres != FR_OK) return; // Removido while(1) para não travar a estação

    // 2. RTC: Ordem correta (Time depois Date)
    if(medidas->hrtc != NULL)
    {
        RTC_DateTypeDef rtc_data;
        RTC_TimeTypeDef rtc_time;

        // IMPORTANTE: Leia o TIME primeiro
        HAL_RTC_GetTime(medidas->hrtc, &rtc_time, RTC_FORMAT_BIN);
        HAL_RTC_GetDate(medidas->hrtc, &rtc_data, RTC_FORMAT_BIN);

        pos = snprintf(message, sizeof(message), "%02d/%02d/%02d %02d:%02d:%02d;", 
                rtc_data.Date, rtc_data.Month, rtc_data.Year,
                rtc_time.Hours, rtc_time.Minutes, rtc_time.Seconds);
    }
    else {
        pos = snprintf(message, sizeof(message), " - ;");
    }

    // 3. Adiciona as medidas com a lógica de 3 casas decimais
    for(int i = 0; i < medidas->tam; i++){
        int valor_escala = (int)(medidas->data[i] * 1000.0f);
        // abs() garante que a parte decimal seja sempre positiva
        pos += snprintf(message + pos, sizeof(message) - pos, "%d.%03d;", 
                        valor_escala / 1000, abs(valor_escala % 1000));
        
        // Proteção para não estourar o buffer message
        if (pos >= sizeof(message) - 4) break; 
    }
    
    // Adiciona o fim de linha manualmente para economizar sprintf
    strcat(message, "\r\n");

    // 4. Gravação direta (Sem precisar de readBuf/strncpy)
    fres = f_open(&fil, sArquivo, FA_OPEN_APPEND | FA_WRITE);
    if(fres == FR_OK) {
        UINT bytesWrote;
        // Gravamos direto da string 'message'
        f_write(&fil, message, strlen(message), &bytesWrote);
        f_close(&fil);
    }

    // 5. Desmonta
    f_mount(NULL, "", 0);
}
