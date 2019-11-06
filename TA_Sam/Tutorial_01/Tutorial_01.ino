#include <HTTPClient.h>

#include <WiFi.h>

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

//const char* ssid = "CBS Beam";
//const char* password = "Bsrc12#$";
//const char* ssid = "CBS Beam 5GHz";   //not working, unable to connect to 5GHz n/w
//const char* password = "Bsrc12#$";
const char* ssid = "esw-m19@iiith";
const char* password = "e5W-eMai@3!20hOct";

//String cse_ip = "139.59.42.21";
String cse_ip = "10.4.21.131";
String cse_port = "8080";
String server = "http://" + cse_ip + ":" + cse_port + "/~/in-cse/in-name/";

void setup_wifi();
void scanNetworks();
void connectToNetwork();
String translateEncryptionType(wifi_auth_mode_t encryptionType);
void setup_i2c();
void setup_modbus();
void sensor_temp_rh();
void energy_meter();

String createCI(String server, String ae, String cnt, String val);

//Buffer t store Temperature & Humity values
float* sht21_buffer = new float[2];

//Buffer to store the Energy Meter values
float* em_buffer = new float[EM_REG_LEN / 2];

esp32ModbusRTU modbus(&Serial1, UART_RTS);

bool publish_once = true;

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
  // start the Modbus RTUs
  setup_modbus();
  Serial.printf("Done!\n");
  Serial.println("###################################### Endof Setup #################################################");
  Serial.println("###################################### Starting Loop ###############################################");
}

void loop() {
  // put your main code here, to run repeatedly:
  sensor_temp_rh();
  energy_meter();
  if (publish_once == true) { 
    String pathh = "pr_5_esp32_1/em/em_1_vll_avg";
    Serial.printf("Publishing to server...");
    createCI(server, "Team43_UPS_performance_monitoring", pathh, "16");  
    Serial.printf("Done\n");
    publish_once = false;
  }
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
  static uint32_t lastMillisSH21 = 0;
  if (millis() - lastMillisSH21 > SHT21_POLLING_INTERVAL) {
    lastMillisSH21 = millis();
    sht21_buffer[0] = SHT2x.GetHumidity();
    sht21_buffer[1] = SHT2x.GetTemperature();
    Serial.printf("Humidity(%RH): %.2f, Temperature(C): %.2f\n", sht21_buffer[0], sht21_buffer[1]);
  }
}
/*
   Endof Temp/Rh Sensors functions
*/

/*
   Start of Energy Meter functions
*/
void setup_modbus() {
  //modbus setup
  Serial1.begin(UART_BAUD_RATE, UART_CONFIG, UART_RX, UART_TX);
  
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
    Serial.printf("EM error: 0x%02x\n", static_cast<uint8_t>(error));
  });

  modbus.begin();
}

void energy_meter() {
  static uint32_t lastMillisEM = 0;
  if (millis() - lastMillisEM > EM_POLLING_INTERVAL) {
    lastMillisEM = millis();
    //Serial.print("sending Modbus request...\n");
    modbus.readHoldingRegisters(EM_ADDRESS, EM_REG_ADDR, EM_REG_LEN);
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

String createCI(String server, String ae, String cnt, String val)
{
  HTTPClient http;
  http.begin(server + ae + "/" + cnt + "/");
  http.addHeader("X-M2M-Origin", "admin:admin");
  http.addHeader("Content-Type", "application/json;ty=4");
  http.addHeader("Content-Length", "100");
  http.addHeader("Connection", "close");
  int code = http.POST("{\"m2m:cin\": {\"cnf\": \"text/plain:0\",\"con\": "+ String(val) +"}}");
  http.end();
  Serial.println(code);
  if(code==-1){
    Serial.println("UNABLE TO CONNECT TO THE SERVER");
    //ledFlag=0;
    //ledUpdate();
  }
  delay(300);

}
