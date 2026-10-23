// INCLUDES --------------------------------------------------------------
#include <Arduino.h>     // requisitado pela LMIC
#include <lmic.h>        // LMIC
#include <hal/hal.h>     // LMIC
#include <SPI.h>         // LMIC
#include "driver/uart.h" // uart
#include "driver/gpio.h" // gpio
#include <string.h>      // memset
#include <sys/types.h>   // size_t
// includes --------------------------------------------------------------

// DEFINES ---------------------------------------------------------------
// monitor serial
#define SERIAL_BAUD_RATE 9600
// intervalo de envio lora
#define LORA_INTERVAL 1800 // em segundos
// uart defines
#define UART_NUM           UART_NUM_2
#define UART_BAUD_RATE     115200
#define BUF_SIZE           256
#define UART_BUFFER_SIZE   8
#define UART_TX_TIMEOUT_MS 100
// chaves odor-fiware
//#define APPEUI_KEY 0x77, 0x77, 0x68, 0x68, 0x68, 0x68, 0x77, 0x77                                                         //lsb
//#define DEVEUI_KEY 0x25, 0x61, 0x07, 0xD0, 0x7E, 0xD5, 0xB3, 0x70                                                         //lsb
//#define APPKEY_KEY 0xFF, 0xCC, 0xD6, 0x71, 0x15, 0x35, 0x49, 0xC5, 0x2F, 0x43, 0xF3, 0x7B, 0xAB, 0xEB, 0x45, 0x0A         //msb
// chaves odor-fiware-1
#define APPEUI_KEY 0x11, 0x77, 0x11, 0x77, 0x11, 0x77, 0x11, 0x77                                                         //lsb
#define DEVEUI_KEY 0x7F, 0x68, 0x07, 0xD0, 0x7E, 0xD5, 0xB3, 0x70                                                         //lsb
#define APPKEY_KEY 0x51, 0xA1, 0x47, 0xBB, 0x39, 0x7B, 0x7E, 0x2F, 0x90, 0x13, 0x1D, 0xEA, 0x88, 0xE6, 0xFA, 0xCF         //msb
// pinos
#define UART_TX_PIN   43
#define UART_RX_PIN   44
#define LORA_NSS_PIN  10
#define LORA_RST_PIN  48
#define LORA_DIO0_PIN 14
#define LORA_DIO1_PIN 21
#define LORA_DIO2_PIN 47
// defines ---------------------------------------------------------------

// VARIÁVEIS GLOBAIS -----------------------------------------------------
// uart variaveis globais
uint8_t uart_buffer[UART_BUFFER_SIZE];
typedef enum{
  GET_SENSOR_VOC    = 0x41, // "A"
  GET_SENSOR_H2S    = 0x42, // "B"
  GET_SENSOR_SO2    = 0x43, // "C"
  GET_SENSOR_NH3    = 0x44, // "D"
  GET_SENSOR_CH3SH  = 0x45, // "E"
  GET_SENSOR_MICS   = 0x46, // "F"
  GET_SENSOR_PRESS    = 0x47, // "G"
  GET_SENSOR_ANEMO  = 0x48, // "H"
  WAKE_UP_FLAG      = 0x79, // "y"
  GET_SENSOR_TEMP   = 0x49, // "I"
  GET_SENSOR_HUM	  = 0x4A  // "J"
} enum_get_sensors;
// lmic variaveis globais
static osjob_t sendjob;
const unsigned TX_INTERVAL = LORA_INTERVAL;
const lmic_pinmap lmic_pins = {
    .nss = LORA_NSS_PIN,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = LORA_RST_PIN,
    .dio = {LORA_DIO0_PIN, LORA_DIO1_PIN, LORA_DIO2_PIN},
};
bool flag_do_send = false; // Flag para controlar o envio de dados após conexão bem-sucedida
static uint8_t lora_payload[52];
// payloader variaveis globais
struct sensor_data {
  float ppb_avg;
  float ppb_max;
  float ppb_min;
  float temp_avg;
  float temp_max;
  float temp_min;
  float hum_avg;
  float hum_max;
  float hum_min;
  float press_avg;
  float press_max;
  float press_min;
  float wind_speed_avg;
  float wind_speed_max;
  float wind_speed_min;
  uint8_t wind_direction;
};
sensor_data voc_data;
sensor_data h2s_data;
sensor_data so2_data;
sensor_data nh3_data;
sensor_data ch3sh_data;
sensor_data temp_data;
sensor_data hum_data;
sensor_data press_data;
sensor_data anemo_data;
// variáveis globais -----------------------------------------------------

// PROTOTIPOS ------------------------------------------------------------
// uart prototipos
void uart_setup();
void uart_send_command(enum_get_sensors sensor);
void uart_request_sensor_data(enum_get_sensors sensor, bool print_debug);
void uart_request_all_sensor(bool print_debug);
bool uart_verify_checksum(uint8_t* data);
bool uart_verify_fist_byte(uint8_t* data, enum_get_sensors sensor);
// lmic prototipos
void do_send(osjob_t *j);
void onEvent(ev_t ev);
// payloader prototipos
void payloader_builder(uint8_t* data, enum_get_sensors sensor);
void payloader_parser_data(uint8_t* payload);
void payloader_print_data();
void payloader_clear();
// prototipos ------------------------------------------------------------

/* -----------------------------------------------------------------------
   #####################################################################
  ----------------------------------------------------------------------*/

// UART ------------------------------------------------------------------
// uart - configuração
void uart_setup(){
  uart_config_t uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    ESP_ERROR_CHECK(uart_param_config(UART_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM, UART_TX_PIN, UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM, BUF_SIZE, BUF_SIZE, 0, NULL, 0));

  uart_flush_input(UART_NUM);
  vTaskDelay(pdMS_TO_TICKS(100)); 
  uart_flush_input(UART_NUM);
}
// uart - enviar comando único
void uart_send_command(enum_get_sensors sensor){
    Serial.printf("Enviando comando para sensor: 0x%02X\n", sensor);
    uint8_t cmd = (uint8_t)sensor;
    uart_write_bytes(UART_NUM, (const char *)&cmd, 1);
    uart_wait_tx_done(UART_NUM, UART_TX_TIMEOUT_MS / portTICK_PERIOD_MS);
}
// uart - solicita todos os sensores
void uart_request_all_sensor(bool print_debug) {
  uart_request_sensor_data(GET_SENSOR_VOC, print_debug);
  uart_request_sensor_data(GET_SENSOR_H2S, print_debug);
  uart_request_sensor_data(GET_SENSOR_SO2, print_debug);
  uart_request_sensor_data(GET_SENSOR_NH3, print_debug);
  uart_request_sensor_data(GET_SENSOR_CH3SH, print_debug);
  uart_request_sensor_data(GET_SENSOR_TEMP, print_debug);
  uart_request_sensor_data(GET_SENSOR_HUM, print_debug);
  uart_request_sensor_data(GET_SENSOR_PRESS, print_debug);
  uart_request_sensor_data(GET_SENSOR_ANEMO, print_debug);
  payloader_parser_data(lora_payload);
  if(print_debug) {
    //print_payload();
    payloader_print_data();
  }
}
// uart - solicitar dados do sensor até 3 vezes e confere
void uart_request_sensor_data(enum_get_sensors sensor, bool print_debug) {
    int attempts = 0;

    while (attempts++ < 3) {
        if (print_debug) {
            Serial.printf("\n--- Tentativa %d | Sensor: 0x%02X ---\n", attempts, sensor);
        }

        // 1️⃣ Preparação e Envio
        uart_flush_input(UART_NUM);
        uint8_t cmd = (uint8_t)sensor;
        uart_write_bytes(UART_NUM, (const char *)&cmd, 1);
        uart_wait_tx_done(UART_NUM, 100 / portTICK_PERIOD_MS);

        // 2️⃣ Leitura do Frame
        int total = 0;
        int retry_read = 0;
        
        while (total < UART_BUFFER_SIZE && retry_read < 10) {
            int bytes_read = uart_read_bytes(
                UART_NUM, 
                uart_buffer + total, 
                UART_BUFFER_SIZE - total, 
                200 / portTICK_PERIOD_MS
            );

            if (bytes_read > 0) {
                total += bytes_read;
                if (print_debug) {
                    Serial.printf("Lidos: %d | Total: %d/%d\n", bytes_read, total, UART_BUFFER_SIZE);
                }
            } else {
                retry_read++;
            }
        }

        // 3️⃣ Debug do Conteúdo Recebido
        if (print_debug) {
            Serial.printf("Dados no Buffer: ");
            for (int i = 0; i < total; i++) {
                Serial.printf("%02X ", uart_buffer[i]);
            }
            Serial.printf("\n");
        }

        // 4️⃣ Validações
        if (total != UART_BUFFER_SIZE) {
            if (print_debug) Serial.printf("ERRO: Tamanho Incorreto (%d/%d)\n", total, UART_BUFFER_SIZE);
            continue; 
        }

        if (!uart_verify_checksum(uart_buffer)) {
            if (print_debug) Serial.printf("ERRO: Checksum Invalido!");
            continue;
        }

        if (!uart_verify_fist_byte(uart_buffer, sensor)) {
            if (print_debug) Serial.printf("ERRO: Header Incorreto!");
            continue;
        }

        // 5️⃣ Sucesso
        if (print_debug) Serial.printf("SUCESSO: Pacote validado.");
        
        payloader_builder(uart_buffer, sensor);
        return; 
    }

    if (print_debug){
        Serial.printf("FALHA CRITICA: 3 tentativas esgotadas.\n");
        payloader_clear();
    }
}
// funcoes auxiliares de verificação
bool uart_verify_fist_byte(uint8_t* data, enum_get_sensors sensor){
  if(data[0] != (uint8_t)sensor){
    return false;
  }
  return true;
} 
bool uart_verify_checksum(uint8_t* data){
  // calculate XOR checksum
  uint8_t xor_checksum = 0x00;
  for(int i = 0; i < UART_BUFFER_SIZE - 1; i++){
    xor_checksum ^= data[i];
  } 
  // verifica o checksum
  if(xor_checksum != data[UART_BUFFER_SIZE - 1]){
    return false;
  }
  return true;
}
// uart ------------------------------------------------------------------

// LMIC ------------------------------------------------------------------
//chaves OTAA LMIC
static const u1_t PROGMEM APPEUI[8] = { APPEUI_KEY };
void os_getArtEui(u1_t *buf) { memcpy_P(buf, APPEUI, 8); }
static const u1_t PROGMEM DEVEUI[8]  = { DEVEUI_KEY }; 
void os_getDevEui(u1_t *buf) { memcpy_P(buf, DEVEUI, 8); }
static const u1_t PROGMEM APPKEY[16] = { APPKEY_KEY };
void os_getDevKey(u1_t *buf) { memcpy_P(buf, APPKEY, 16); }
// Callback de evento: todo evento do LoRaAN irá chamar essa
// callback, de forma que seja possível saber o status da
// comunicação com o gateway LoRaWAN.
void onEvent(ev_t ev)
{
    Serial.print(os_getTime());
    Serial.print(": ");
    switch (ev)
    {
    case EV_JOINING:
        Serial.println(F("EV_JOINING"));
        break;
    case EV_JOINED:
        Serial.println(F("EV_JOINED"));
        flag_do_send = true; // Habilita o envio de dados após conexão bem-sucedida
        break;
    case EV_JOIN_FAILED:
        Serial.println(F("EV_JOIN_FAILED"));
        LMIC_setLinkCheckMode(0);
        break;
    case EV_TXCOMPLETE:
        Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
        if (LMIC.txrxFlags & TXRX_ACK)
            Serial.println(F("Received ack"));
        if (LMIC.dataLen)
        {
            Serial.print(F("Received "));
            Serial.print(LMIC.dataLen);
            Serial.println(F(" bytes of payload"));
        }
        if (LMIC.dataLen == 1) 
        {
            uint8_t dados_recebidos = LMIC.frame[LMIC.dataBeg + 0];
            Serial.print(F("Dados recebidos: "));
            Serial.write(dados_recebidos);
        }
        // Agenda a transmissão automática com intervalo de TX_INTERVAL
        os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(TX_INTERVAL), do_send);
        break;
    case EV_TXSTART:
        Serial.println(F("EV_TXSTART"));
        break;
    case EV_JOIN_TXCOMPLETE:
        Serial.println(F("EV_JOIN_TXCOMPLETE: no JoinAccept"));
        break;
    default:
        Serial.print(F("Unknown event: "));
        Serial.println((unsigned)ev);
        break;
    }
}
// Função de envio de dados para o gateway LoRaWAN
void do_send(osjob_t *j)
{
  if(flag_do_send){
    //static uint8_t payload[] = "Hello, world!";
    uart_request_all_sensor(true);
  }
    // Verifica se não está ocorrendo uma transmissão no momento TX/RX
    if (LMIC.opmode & OP_TXRXPEND){
        Serial.println(F("OP_TXRXPEND, not sending"));
    }
    else{
        //envio
        LMIC_setTxData2(1, lora_payload, sizeof(lora_payload), 0);
        Serial.println(F("Sended"));
    }
}
// lmic ------------------------------------------------------------------

// PAYLOADER -------------------------------------------------------------
/* UART BUFFER FORMAT
|------------|-----------------------|---------------|
| Byte Index | Description           | Size (Bytes)  |
|------------|-----------------------|---------------|
| 0          | Sensor ID             | 1             |
| 1-6        | Sensor Data           | 6             |
| 7          | Checksum (XOR of 0-6) | 1             |
|------------|-----------------------|---------------|
*/
// contrutor de payload - recebe o buffer da uart e aloca no devido espaço dopayload do lora
void payloader_builder(uint8_t* data, enum_get_sensors sensor){
  if(sensor == GET_SENSOR_ANEMO){
    lora_payload[48] = data[2]; // wind speed avg
    lora_payload[49] = data[3]; // wind speed max
    lora_payload[50] = data[4]; // wind speed min
    lora_payload[51] = data[1]; // wind direction
    return;
  } else {
    int index = 0;
    switch (sensor)
    {
    case GET_SENSOR_VOC:
      index = 0;
      break;
    case GET_SENSOR_H2S:
      index = 6;
      break;
    case GET_SENSOR_SO2:
      index = 12;
      break;
    case GET_SENSOR_NH3:
      index = 18;
      break;
    case GET_SENSOR_CH3SH:
      index = 24;
      break;
    case GET_SENSOR_TEMP:
      index = 30;
      break;
    case GET_SENSOR_HUM:
      index = 36;
      break;
    case GET_SENSOR_PRESS:
      index = 42;
      break;
    default:
      break;
    }
    lora_payload[index]     = data[1];
    lora_payload[index + 1] = data[2];
    lora_payload[index + 2] = data[3];
    lora_payload[index + 3] = data[4];
    lora_payload[index + 4] = data[5];
    lora_payload[index + 5] = data[6];
  }
}
// parser de payload - recebe o payload do lora e aloca nas devidas estruturas de dados para uso local
void payloader_parser_data(uint8_t* payload){
  // VOC
  voc_data.ppb_avg = (payload[0] << 8) | payload[1];
  voc_data.ppb_max = ((payload[2] << 8) | payload[3]);
  voc_data.ppb_min = ((payload[4] << 8) | payload[5]);
  // H2S
  h2s_data.ppb_avg = (payload[6] << 8) | payload[7];
  h2s_data.ppb_max = ((payload[8] << 8) | payload[9]);
  h2s_data.ppb_min = ((payload[10] << 8) | payload[11]);
  // SO2
  so2_data.ppb_avg = (payload[12] << 8) | payload[13];
  so2_data.ppb_max = ((payload[14] << 8) | payload[15]);
  so2_data.ppb_min = ((payload[16] << 8) | payload[17]);
  // NH3
  nh3_data.ppb_avg = (payload[18] << 8) | payload[19];
  nh3_data.ppb_max = ((payload[20] << 8) | payload[21]);
  nh3_data.ppb_min = ((payload[22] << 8) | payload[23]);
  // CH3SH
  ch3sh_data.ppb_avg = (payload[24] << 8) | payload[25];
  ch3sh_data.ppb_max = ((payload[26] << 8) | payload[27]);
  ch3sh_data.ppb_min = ((payload[28] << 8) | payload[29]);
  // TEMP
  temp_data.temp_avg = ((payload[30] << 8) | payload[31]) / 100.0;
  temp_data.temp_max = ((payload[32] << 8) | payload[33]) / 100.0;
  temp_data.temp_min = ((payload[34] << 8) | payload[35]) / 100.0;
  // HUM
  hum_data.hum_avg = ((payload[36] << 8) | payload[37]) / 100.0;
  hum_data.hum_max = ((payload[38] << 8) | payload[39]) / 100.0;
  hum_data.hum_min = ((payload[40] << 8) | payload[41]) / 100.0;
  // PRES
  press_data.press_avg = (((payload[42] << 8) | payload[43]) + 90000) / 100.0;
  press_data.press_max = (((payload[44] << 8) | payload[45]) + 90000) / 100.0;
  press_data.press_min = (((payload[46] << 8) | payload[47]) + 90000) / 100.0;
  // ANEMO
  anemo_data.wind_speed_avg = payload[48];
  anemo_data.wind_speed_max = payload[49];
  anemo_data.wind_speed_min = payload[50];
  anemo_data.wind_direction = payload[51];
}
// função de debug para imprimir os dados do payload de forma legível
void payloader_print_data() {
  Serial.println("\n--- Sensor Data ---");
  Serial.println("| Sensor | Média | Máximo | Mínimo |");
  Serial.printf("| VOC    | %.2f  | %.2f   | %.2f   |\n", voc_data.ppb_avg, voc_data.ppb_max, voc_data.ppb_min);
  Serial.printf("| H2S    | %.2f  | %.2f   | %.2f   |\n", h2s_data.ppb_avg, h2s_data.ppb_max, h2s_data.ppb_min);
  Serial.printf("| SO2    | %.2f  | %.2f   | %.2f   |\n", so2_data.ppb_avg, so2_data.ppb_max, so2_data.ppb_min);
  Serial.printf("| NH3    | %.2f  | %.2f   | %.2f   |\n", nh3_data.ppb_avg, nh3_data.ppb_max, nh3_data.ppb_min);
  Serial.printf("| CH3SH  | %.2f  | %.2f   | %.2f   |\n", ch3sh_data.ppb_avg, ch3sh_data.ppb_max, ch3sh_data.ppb_min);
  Serial.printf("| TEMP   | %.2f  | %.2f   | %.2f   |\n", temp_data.temp_avg, temp_data.temp_max, temp_data.temp_min);
  Serial.printf("| HUM    | %.2f  | %.2f   | %.2f   |\n", hum_data.hum_avg, hum_data.hum_max, hum_data.hum_min);
  Serial.printf("| PRES   | %.2f  | %.2f   | %.2f   |\n", press_data.press_avg, press_data.press_max, press_data.press_min);
  Serial.printf("| ANEMO  | %.2f  | %.2f   | %.2f   | Dir: %d° |\n", anemo_data.wind_speed_avg, anemo_data.wind_speed_max, anemo_data.wind_speed_min, anemo_data.wind_direction);
  Serial.println("-------------------");
}
// função para limpar os dados do payload em caso de falha na leitura da uart
void payloader_clear() {
  for (int i = 0; i < sizeof(lora_payload); i++) {
    lora_payload[i] = 0;
  }
}
// payloader -------------------------------------------------------------

/* -----------------------------------------------------------------------
   #####################################################################
  ----------------------------------------------------------------------*/

// SETUP -----------------------------------------------------------------
void setup()
{
    // configuração da uart
    vTaskDelay(pdMS_TO_TICKS(500)); 
    uart_setup();
    vTaskDelay(pdMS_TO_TICKS(500));
    // aguarda sinal de wake up do STM
    bool wake_up = false;
    while(!wake_up){
        int bytes_read = uart_read_bytes(UART_NUM, uart_buffer, 1, 1000 / portTICK_PERIOD_MS);
        if(bytes_read == 1 && uart_buffer[0] == WAKE_UP_FLAG){
        wake_up = true;
        }
    }
    memset(uart_buffer, 0, UART_BUFFER_SIZE);
    // ligar o transmissor LoRaWAN
    gpio_reset_pin(GPIO_NUM_5);
    gpio_set_direction(GPIO_NUM_5, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_5, 1);
    vTaskDelay(pdMS_TO_TICKS(5000));
    // iniciar comunicação com o monitor serial
    Serial.begin(SERIAL_BAUD_RATE);
    Serial.println(F("Wake up recebido"));
    // iniciar o LMIC
    os_init();
    LMIC_reset();
    //LMIC_setClockError(MAX_CLOCK_ERROR * 20 / 100); 
    //LMIC_setLinkCheckMode(0);
    os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(2), do_send);
}
// setup -----------------------------------------------------------------

// LOOP ------------------------------------------------------------------
void loop()
{
    os_runloop_once();
}
// loop ------------------------------------------------------------------

/* -----------------------------------------------------------------------
   #####################################################################
  ----------------------------------------------------------------------*/