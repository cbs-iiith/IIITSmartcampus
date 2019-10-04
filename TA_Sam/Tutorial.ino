#include <Arduino.h>

#include <esp32ModbusRTU.h>
#include <esp32ModbusTypeDefs.h>
#include <ModbusMessage.h>

#include <algorithm>  // for std::reverses

#include <dummy.h>

#include <Wire.h>
#include <Sodaq_SHT2x.h>

#define CONSOLE_BAUD_RATE              115200

//SHT21 Temperature & Rh sensor connected to I2C
//I2C defines
#define I2C_SCL                        22
#define I2C_SDA                        21

//SHT21 defines
#define SHT21_POLLING_INTERVAL         10000

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
#define EM_REG_VOLT                    142
#define EM_REG_CURR                    144
#define EM_REG_APWR                    146

void setup_i2c();
void setup_modbus();
void sensor_temp_rh();
void energy_meter();

esp32ModbusRTU modbus(&Serial1, UART_RTS);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(CONSOLE_BAUD_RATE);

  Serial.println("Starting I2C");
  setup_i2c();
  if (!Wire1.begin(I2C_SDA, I2C_SCL)) {
    Serial.println("Failed to start I2C!");
    while (1);
  }

  Serial.println("Start Modbus RTU");
  // start the Modbus RTUs
  Serial1.begin(UART_BAUD_RATE, UART_CONFIG, UART_RX, UART_TX);
  setup_modbus();

}

void loop() {
  // put your main code here, to run repeatedly:
  sensor_temp_rh();
  energy_meter();
}

void setup_i2c() {
  //i2c setup
  pinMode(I2C_SCL, INPUT_PULLUP);           // set pin to input
  pinMode(I2C_SDA, INPUT_PULLUP);           // set pin to input
}

void setup_modbus() {
  //modbus setup
  modbus.onData([](uint8_t serverAddress, esp32Modbus::FunctionCode fc, uint8_t* data, size_t length) {
    Serial.printf("id 0x%02x fc 0x%02x len %u: 0x", serverAddress, fc, length);
    for (size_t i = 0; i < length; ++i) {
      Serial.printf("%02x", data[i]);
    }
    //std::reverse(data, data + 4);  // fix endianness
    uint8_t data2[4];
    data2[0] = data[1];
    data2[1] = data[0];
    data2[2] = data[3];
    data2[3] = data[2];
    Serial.printf("\nval: %.2f\n", *(float*)data2);//*reinterpret_cast<float*>(data2));
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
    float rel_humidity = SHT2x.GetHumidity();
    float temperature = SHT2x.GetTemperature();
    Serial.printf("Humidity(%RH): %.2f, Temperature(C): %.2f\n", rel_humidity, temperature);
  }
}

void energy_meter() {
  static uint32_t lastMillisEM = 0;
  if (millis() - lastMillisEM > EM_POLLING_INTERVAL) {
    lastMillisEM = millis();
    Serial.print("sending Modbus request...\n");
    modbus.readHoldingRegisters(EM_ADDRESS, EM_REG_VOLT, 2);
    //digitalWrite(UART_RTS, HIGH);
    //Serial1.print("Hello");
    //delay(1);
    //digitalWrite(UART_RTS, LOW);
  }
}
