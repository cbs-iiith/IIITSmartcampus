#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <Arduino.h>
#include <time.h>
#include "esp32ModbusRTU.h"
#include "esp32ModbusTypeDefs.h"
#include "ModbusMessage.h"
#include <WiFiUdp.h>
#include "NTPClient.h"
#include <Wire.h>

#define CONSOLE_BAUD_RATE              115200

//SHT21 Temperature & Rh sensor connected to I2C
//I2C defines
#define I2C_SDA                        21
#define I2C_SCL                        22

#define si7021Addr 0x40

//SHT21 defines
#define SHT21_POLLING_INTERVAL         10000
#define SHT21_ADDR                     0x40

//Energy Meter connected to Modbus RTU
//UART 
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

const char* ssid = "moto g(6)";
const char* password = "123890765";
String cse_ip = "139.59.42.21";
String cse_port = "8080";
String server = "http://" + cse_ip + ":" + cse_port + "/~/in-cse/in-name/";
String send_temperature;
String send_power;
String send_humidity;
String formattedDate;
String dayStamp;
String timeStamp;

int meter = 1;
float power1, power2;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
 

void setup_wifi();
void scanNetworks();
void connectToNetwork();
String translateEncryptionType(wifi_auth_mode_t encryptionType);
void setup_i2c();
void setup_modbus();
void sensor_temp_rh();
void energy_meter();

//Buffer t store Temperature & Humity values
float* sht21_buffer = new float[2];

//Buffer to store the Energy Meter values
float* em_buffer = new float[EM_REG_LEN / 2];

esp32ModbusRTU modbus(&Serial1, UART_RTS);

String createCI(String server, String ae, String cnt, String val)
{
  HTTPClient http;
  http.begin(server + ae + "/" + cnt + "/");
  http.addHeader("X-M2M-Origin", "admin:admin");
  http.addHeader("Content-Type", "application/json;ty=4");
  http.addHeader("Content-Length", "100");
  http.addHeader("Connection", "close");
  int code = http.POST("{\"m2m:cin\": {\"cnf\": \"text/plain:0\",\"con\": " + String(val) + "}}");
  http.end();
  Serial.print(code);
  delay(300);
}

void setup() {
  // put your setup code here, to run once:
  Serial.println("###################################### Starting Setup ##############################################");
  Serial.begin(CONSOLE_BAUD_RATE);
  Serial.println("Connecting WiFI...");
  setup_wifi();
  Serial.printf("Done!\n");
  Serial.println("####################################################################################################");
  Serial.printf("Starting I2C...");
  setup_i2c();
  Serial.printf("Success!\n");
  Serial.println("####################################################################################################");
  Serial.printf("Start Modbus RTU...");
//  start the Modbus RTUs
  setup_modbus();
  Serial.printf("Done!\n");
  Serial.println("###################################### Endof Setup #################################################");
  Serial.println("###################################### Starting Loop ###############################################");
  timeClient.begin();
  timeClient.setTimeOffset(19800);
}

void loop() {
  energy_meter();
  delay(10001);
  sensor_temp_rh();
  delay(5001);
}


/*
   Start of Temp/Rh Sensors functions
*/
void setup_i2c() {
  //i2c setup
  pinMode(I2C_SCL, INPUT_PULLUP);           // set pin to input
  pinMode(I2C_SDA, INPUT_PULLUP);           // set pin to input
  if (!Wire.begin(I2C_SDA, I2C_SCL, 100000)) {
    Serial.printf("Failed to start I2C!\n");
    while (1);
  }
  Wire.beginTransmission(SHT21_ADDR);
  Wire.endTransmission();
  delay(300);
}

void sensor_temp_rh() {
  unsigned int data[2];

  Wire.beginTransmission(si7021Addr);
  Wire.write(0xF5);                                   //Send humidity measurement command
  Wire.endTransmission();
  delay(500);

  Wire.requestFrom(si7021Addr, 2);                    // Request 2 bytes of data

  if (Wire.available() == 2)                          // Read 2 bytes of data to get humidity
  {
    data[0] = Wire.read();
    data[1] = Wire.read();
  }

  float humidity  = ((data[0] * 256.0) + data[1]);    // Convert the data
  humidity = ((125 * humidity) / 65536.0) - 6;

  Wire.beginTransmission(si7021Addr);
  Wire.write(0xF3);                                   // Send temperature measurement command
  Wire.endTransmission();
  delay(500);

  Wire.requestFrom(si7021Addr, 2);                    // Request 2 bytes of data

  if (Wire.available() == 2)                          // Read 2 bytes of data for temperature
  {
    data[0] = Wire.read();
    data[1] = Wire.read();
  }
  
  while(!timeClient.update()) {
    timeClient.forceUpdate();
  }
  formattedDate = timeClient.getFormattedDate();
  int splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);
  timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
  
  float temp  = ((data[0] * 256.0) + data[1]);        // Convert the data
  float celsTemp = ((175.72 * temp) / 65536.0) - 46.85;
  float fahrTemp = celsTemp * 1.8 + 32;
  
  // Output data to serial monitor
  Serial.print("Humidity : ");
  Serial.print(humidity);
  Serial.println(" % RH");
  Serial.print("Celsius : ");
  Serial.print(celsTemp);
  Serial.println(" C");
  Serial.print("Fahrenheit : ");
  Serial.print(fahrTemp);
  Serial.println(" F");
  
  String sensor_temperature = "(" + String(celsTemp) + "," + String(fahrTemp) + ")" + ";" + String(dayStamp) + " " + String(timeStamp);
  String sensor_humidity = String(humidity) + ";" + String(dayStamp) + " " + String(timeStamp);
  
  send_temperature = "\"" + sensor_temperature + "\"";
  send_humidity = "\"" + sensor_humidity + "\"";
  
  createCI(server, "Team40_DG_performance_monitoring", "pr_2_esp32_1/oe/oe_1_temperature", send_temperature);
  createCI(server, "Team40_DG_performance_monitoring", "pr_2_esp32_1/oe/oe_1_rh", send_humidity);
  createCI(server, "Team40_DG_performance_monitoring", "pr_2_esp32_1/em/em_1_watts_total", send_power);            
  delay(3000);
}
/*
   Endof Temp/Rh Sensors functions
*/

/*
   Start of Energy Meter functions
*/
void setup_modbus() {
  //modbus setup
  Serial1.begin(9600, SERIAL_8E1, 26, 27);  // Modbus connection

  modbus.onData([](uint8_t serverAddress, esp32Modbus::FunctionCode fc, uint8_t* data, size_t length) {
    Serial.printf("id 0x%02x fc 0x%02x len %u: 0x", serverAddress, fc, length);
    for (size_t i = 0; i < length; ++i) {
      Serial.printf("%02x", data[i]);
    }
    
    while(!timeClient.update()) {
      timeClient.forceUpdate();
    }
    formattedDate = timeClient.getFormattedDate();
    int splitT = formattedDate.indexOf("T");
    dayStamp = formattedDate.substring(0, splitT);
    timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1);
    
    uint8_t data2[4];
    data2[0] = data[1];
    data2[1] = data[0];
    data2[2] = data[3];
    data2[3] = data[2];
    
    float power = *reinterpret_cast<float*>(data2);
    Serial.printf("\nval: %.2f", power);
    Serial.print("\n\n");
    String meter_power = String(power) + ";" + String(dayStamp) + " " + String(timeStamp);
    send_power = "\"" + meter_power + "\"";
    
  });
  modbus.onError([](esp32Modbus::Error error) {
    Serial.printf("Got error: 0x%02x\n\n", static_cast<uint8_t>(error));
  });
  modbus.begin();
}

void energy_meter() {
  static uint32_t lastMillis = 0;
  if (millis() - lastMillis > 10000) {
    lastMillis = millis();
    Serial.print("sending Modbus request...\n");
    Serial.print("power(watts)\n");
    modbus.readHoldingRegisters(0x01, 158, 2);     
  }
}
/*
   Endof Energy Meter functions
*/
 
/*
   Start wifi functions
*/
void setup_wifi() {
  scanNetworks();
  connectToNetwork();

  Serial.println(WiFi.macAddress());
  Serial.println(WiFi.localIP());
}

String translateEncryptionType(wifi_auth_mode_t encryptionType) {

  switch (encryptionType) {
    case (WIFI_AUTH_OPEN):
      return "Open";
    case (WIFI_AUTH_WEP):
      return "WEP";
    case (WIFI_AUTH_WPA_PSK):
      return "WPA_PSK";
    case (WIFI_AUTH_WPA2_PSK):
      return "WPA2_PSK";
    case (WIFI_AUTH_WPA_WPA2_PSK):
      return "WPA_WPA2_PSK";
    case (WIFI_AUTH_WPA2_ENTERPRISE):
      return "WPA2_ENTERPRISE";
  }
}

void scanNetworks() {
  int numberOfNetworks = WiFi.scanNetworks();
  Serial.print("Number of networks found: ");
  Serial.println(numberOfNetworks);

  for (int i = 0; i < numberOfNetworks; i++) {

    Serial.print("Network name: ");
    Serial.println(WiFi.SSID(i));

    Serial.print("Signal strength: ");
    Serial.println(WiFi.RSSI(i));

    Serial.print("MAC address: ");
    Serial.println(WiFi.BSSIDstr(i));

    Serial.print("Encryption type: ");
    String encryptionTypeDescription = translateEncryptionType(WiFi.encryptionType(i));
    Serial.println(encryptionTypeDescription);
    Serial.println("----------------------------------------------");

  }
}

void connectToNetwork() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Establishing connection to WiFi..");
  }
  Serial.println("Connected to network");
}
/*
   EndOF wifi functions
*/
