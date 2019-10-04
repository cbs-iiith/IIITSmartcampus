#include <Arduino.h>

#include <esp32ModbusRTU.h>
#include <esp32ModbusTypeDefs.h>
#include <ModbusMessage.h>

#include <Wire.h>
#include <Sodaq_SHT2x.h>

#define CONSOLE_BAUD_RATE              115200

//SHT21 Temperature & Rh sensor connected to I2C
//I2C defines
#define I2C_SDA                        21
#define I2C_SCL                        22

//SHT21 defines
#define SHT21_POLLING_INTERVAL         10000
#define SHT21_ADDR                     0x40

//Energy Meter connected to Modbus RTU
//UART defines
#define UART_TX                        27
#define UART_RX                        26
#define UART_RTS                       25
#define UART_BAUD_RATE                 9600
#define UART_DATA_BIT                  8
#define UART_PARITY                    EVEN
#define UART_STOP_BIT                  1
#define UART_CONFIG                    SERIAL_8E1

//Energy meter defines
#define EM_POLLING_INTERVAL            10000
#define EM_ADDRESS                     0x01
#define EM_REG_ADDR                    100
#define EM_REG_LEN                     60

#define EM_IDX_WATTS_R_PHASE           1
#define EM_IDX_VAR_R_PHASE             5
#define EM_IDX_PF_R_PAHSE              9
#define EM_IDX_VA_R_PHASE              14
#define EM_IDX_V_R_PHASE               21
#define EM_IDX_A_R_PHASE               25
#define EM_IDX_FREQUENCY               28
#define EM_IDX_VRY_PHASE               17

void setup_i2c();
void setup_modbus();
void sensor_temp_rh();
void energy_meter();

//Buffer t store Temperature & Humity values
float* sht21_buffer = new float[2];

//Buffer to store the Energy Meter values
float* em_buffer = new float[EM_REG_LEN / 2];

esp32ModbusRTU modbus(&Serial1, UART_RTS);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(CONSOLE_BAUD_RATE);

  Serial.printf("Starting I2C...");
  pinMode(I2C_SCL, INPUT_PULLUP);           // set pin to input
  pinMode(I2C_SDA, INPUT_PULLUP);           // set pin to input
  if (!Wire.begin(I2C_SDA, I2C_SCL, 100000)) {
    Serial.printf("Failed to start I2C!\n");
    while (1);
  }
  setup_i2c();
  Serial.printf("Success!\n");


  Serial.printf("Start Modbus RTU...");
  // start the Modbus RTUs
  Serial1.begin(UART_BAUD_RATE, UART_CONFIG, UART_RX, UART_TX);
  setup_modbus();
  Serial.printf("Done!\n");
}

void loop() {
  // put your main code here, to run repeatedly:
  sensor_temp_rh();
  energy_meter();
}

void setup_i2c() {
  //i2c setup
  Wire.beginTransmission(SHT21_ADDR);
  Wire.endTransmission();
  delay(300);

}

void setup_modbus() {
  //modbus setup
  modbus.onData([](uint8_t serverAddress, esp32Modbus::FunctionCode fc, uint8_t* data, size_t length) {
    //Serial.printf("id 0x%02x fc 0x%02x len %u: 0x", serverAddress, fc, length);

    uint8_t* buffer = new uint8_t[length];
    for (size_t i = 0; i < length; ++i) {
      (i % 2 == 0) ? buffer[i] = data[i + 1] : buffer[i] = data[i - 1];
      //Serial.printf(" d:%02x", data[i]);
      //Serial.printf(" b:%02x", buffer[i]);
    }
    //Serial.printf(" DONE\n");

    em_buffer = (float*) buffer;
    Serial.printf("EM_IDX_V_R_PHASE: %.2f", em_buffer[EM_IDX_V_R_PHASE]);
    Serial.printf(", EM_IDX_A_R_PHASE: %.2f", em_buffer[EM_IDX_A_R_PHASE]);
    Serial.printf(", EM_IDX_FREQUENCY: %.2f", em_buffer[EM_IDX_FREQUENCY]);
    Serial.printf(", EM_IDX_VRY_PHASE: %.2f\n", em_buffer[EM_IDX_VRY_PHASE]);
    Serial.printf("EM_IDX_WATTS_R_PHASE: %.2f", em_buffer[EM_IDX_WATTS_R_PHASE]);
    Serial.printf(", EM_IDX_VAR_R_PHASE: %.2f", em_buffer[EM_IDX_VAR_R_PHASE]);
    Serial.printf(", EM_IDX_PF_R_PAHSE: %.2f", em_buffer[EM_IDX_PF_R_PAHSE]);
    Serial.printf(", EM_IDX_VA_R_PHASE: %.2f\n", em_buffer[EM_IDX_VA_R_PHASE]);
  });
  
  modbus.onError([](esp32Modbus::Error error) {
    Serial.printf("error: 0x%02x\n", static_cast<uint8_t>(error));
  });
  
  modbus.begin();
}

void sensor_temp_rh() {
  static uint32_t lastMillisSH21 = 0;
  if (millis() - lastMillisSH21 > SHT21_POLLING_INTERVAL) {
    lastMillisSH21 = millis();
    sht21_buffer[0] = SHT2x.GetHumidity();
    sht21_buffer[1] = SHT2x.GetTemperature();
    Serial.printf("Humidity(%RH): %.2f, Temperature(C): %.2f\n", sht21_buffer[0], sht21_buffer[1]);
  }
}

void energy_meter() {
  static uint32_t lastMillisEM = 0;
  if (millis() - lastMillisEM > EM_POLLING_INTERVAL) {
    lastMillisEM = millis();
    //Serial.print("sending Modbus request...\n");
    modbus.readHoldingRegisters(EM_ADDRESS, EM_REG_ADDR, EM_REG_LEN);
  }
}
