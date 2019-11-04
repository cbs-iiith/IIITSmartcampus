#include <WiFi.h>
#include <Arduino.h>

#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <stdlib.h>

#include "esp32ModbusRTU.h"
//#include <esp32ModbusTypeDefs.h>
//#include <ModbusMessage.h>

#include <Wire.h>

#define CONSOLE_BAUD_RATE              115200

//Water Flow Sensor 
#define SENSOR  25
#define interval 1000

//SHT21 Temperature & Rh sensor connected to I2C
//I2C defines
#define I2C_SDA                        27 
#define I2C_SCL                        14 
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



char* ssid = "ChandlerBing";
char* password = "smellycatgeller"; 

//const char* ssid = "CBS Beam";
//const char* ssid = "CBS Beam 5GHz";   //not working, unable to connect ti 5GHz n/w
//const char* password = "Bsrc12#$";

void setup_wifi();
void scanNetworks();
void connectToNetwork();
String translateEncryptionType(wifi_auth_mode_t encryptionType);
void setup_i2c();
void setup_modbus();
void sensor_temp_rh();
void energy_meter();
void flow();

//Buffer t store Temperature & Humity values
float* sht21_buffer = new float[2];

//Buffer to store the Energy Meter values
float* em_buffer = new float[EM_REG_LEN / 2];

//Buffer to store Flow Meter values
//float* flow_buffer = new float[2];
float flow_buffer[2];
esp32ModbusRTU modbus(&Serial1, UART_RTS);

//Water Flow
long currentMillis = 0;
long previousMillis = 0;
float calibrationFactor = 4.5;
volatile byte pulseCount;
byte pulse1Sec = 0;
float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;

//---------------------------------------------------------------------------------------------------------------//

////////////////////////////////////////

String cse_ip = "139.59.42.21";
String cse_port = "8080";
String server = "http://"+cse_ip+":"+cse_port+"/~/in-cse/in-name/";

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
  
  Serial.print("result:");
  Serial.println(code);
  delay(300);
}


////////////////////////////////////////////////////////////////////
/*RECEIVING DATA FROM ONEM2M FOR ACTUATOR */

String sendGET(String url)
{
    StaticJsonBuffer<300> jsonBuffer;
    HTTPClient http;  //Declare an object of class HTTPClient
    http.begin(url);  //Specify request destination
    http.addHeader("X-M2M-Origin", "admin:admin");
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.GET();                                                                  //Send the request
    String payload = "";
    char json[300];
    const char* value = 0;
   
    if (httpCode > 0) 
    { 
      payload = http.getString();   //Get the request response payload
      Serial.printf("Payload\n"); 
      Serial.print(payload);
      payload.toCharArray(json, 300);
      JsonObject& root = jsonBuffer.parseObject(json);
//    Test if parsing /succeeds.
      if (!root.success()) 
      {
        Serial.println("parseObject() failed");
      }
      const char* state = root["m2m:cin"]["con"];
      Serial.printf("State\n");
      Serial.print(state);
//      Serial.println(value);
      http.end();   //Close connection
      return state;
    }
    http.end();   //Close connection
}

/////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------------------------------------//

//Water Flow Sensor ISR
void IRAM_ATTR pulseCounter()
{
  pulseCount++;
}

//-------------------
float t,h,rate,total,voltage,current,power;
//-------------------

//*****************************************************************************************************************************************************

void setup() {
  pinMode(12,OUTPUT); //test
  pinMode(17,OUTPUT);
  digitalWrite(12,HIGH); //test
  pinMode(SENSOR, INPUT_PULLUP);
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
  pulseCount = 0;
  flowRate = 0.0;
  flowMilliLitres = 0;
  totalMilliLitres = 0;
  previousMillis = 0;
  attachInterrupt(digitalPinToInterrupt(SENSOR), pulseCounter, FALLING);
  t=0; h=0; rate=0; total = 0;
  voltage=0; power=0; current=0;
  Serial.println("###################################### Endof Setup #################################################");
  Serial.println("###################################### Starting Loop ###############################################");
}

//************************************************************************************************************************************************************************

void loop() {
  // put your main code here, to run repeatedly:
  sensor_temp_rh();
  energy_meter();
  reflow();
  
  Serial.printf("t %.2f,h %.2f,rate %.2f",t,h,flowRate);
  Serial.printf(" totalflow %ld",totalMilliLitres);
  Serial.printf(" voltage %.2f power %.2f current %.2f ",voltage,power,current);
  
  //String sensor1 = "(" + String(t) + "," + String(h) + "," + String(flowRate) + "," + String(totalMilliLitres) + "," + String(voltage) + "," + String(power) + "," + String(current) + ")";  
  //String sensor_string = sensor1;
  //sensor_string = "\"" + sensor_string + "\""; // DO NOT CHANGE THIS LINE
  
//  createCI(server, "Team15_Hostel_washing_machine_utomation", "pr_3_esp32_1", sensor_string);


  //FOR THE ACTUATOR
  String value = sendGET(server + "Team15_Hostel_washing_machine_automation" + "/project_description/la");

    if(value=="1")
    {
      digitalWrite(17, HIGH);
    }
    else if(value=="0")
    {
      digitalWrite(17, LOW);
    }
    else
    {
      Serial.println("Invalid");
    }
//  Serial.println("after createCI");
  delay(15000); // DO NOT CHAN/GE THIS LINE/
  Serial.println("delay");
}

//*******************************************************************************************************************************************************************

//                                                                                      TEMP & RH SENSOR
/*
   Start of Temp/Rh Sensors functions
*/
void setup_i2c() {
  //i2c setup
  
  //TESTING: input pullup is not required
  //pinMode(I2C_SCL, INPUT_PULLUP);           // set pin to input
  //pinMode(I2C_SDA, INPUT_PULLUP);           // set pin to input
  
  if (!Wire.begin(I2C_SDA, I2C_SCL, 100000)) { //100000 is the clock frequency, according to sht21 datasheet there is no minimum freq
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
     h = Humidity();
    t = Temperature();
    Serial.printf("\nHumidity(%RH): %.2f, Temperature(C): %.2f\n", h,t);
  }
}
float Humidity()
{
  unsigned int data[2]={0};
  Wire.beginTransmission(SHT21_ADDR);
  //Send humidity measurement command
  Wire.write(0xF5);
  Wire.endTransmission();
  delay(500);
     
  // Request 2 bytes of data
  Wire.requestFrom(SHT21_ADDR, 2);
  // Read 2 bytes of data to get humidity
  if(Wire.available() == 2)
  {
    data[0] = Wire.read();
    data[1] = Wire.read();
  }
     
  // Convert the data
  float hum  = ((data[0] * 256.0) + data[1]);
  hum = ((125 * hum) / 65536.0) - 6;
  return hum; 
}

float Temperature()
{
  unsigned int data[2]={0};
  Wire.beginTransmission(SHT21_ADDR);
  // Send temperature measurement command
  Wire.write(0xF3);
  Wire.endTransmission();
  delay(500);
     
  // Request 2 bytes of data
  Wire.requestFrom(SHT21_ADDR, 2);
   
  // Read 2 bytes of data for temperature
  if(Wire.available() == 2)
  {
    data[0] = Wire.read();
    data[1] = Wire.read();  
  }
  // Convert the data
  float temperature  = ((data[0] * 256.0) + data[1]);
  temperature = ((175.72 * temperature) / 65536.0) - 46.85;
  return temperature;
}
/*
   Endof Temp/Rh Sensors functions
*/

//*************************************************************************************************************************************************************************

//                                                                                  ENERGY METER



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
    Serial.printf("error: 0x%02x\n", static_cast<uint8_t>(error));
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
   //Endof Energy Meter functions
*/


//***************************************************************************************************************************************************************************
//                                                                                  FLOW METER

/*
 Start of Flow Meter function
*/

void reflow()
{
  currentMillis = millis();
  if (currentMillis - previousMillis > interval)
  {  
    pulse1Sec = pulseCount;
    pulseCount = 0;

    // Because this loop may not complete in exactly 1 second intervals we calculate
    // the number of milliseconds that have passed since the last execution and use
    // that to scale the output. We also apply the calibrationFactor to scale the output
    // based on the number of pulses per second per units of measure (litres/minute in
    // this case) coming from the sensor.
    flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;


    //COMMENTING THIS TEMPORARILY *******************************************************************************************
    //previousMillis = millis();
    // ***********************************************************************************************************************

    
    // Divide the flow rate in litres/minute by 60 to determine how many litres have
    // passed through the sensor in this 1 second interval, then multiply by 1000 to
    // convert to millilitres.
    flowMilliLitres = (flowRate / 60) * 1000;

    // Add the millilitres passed in this second to the cumulative total
    totalMilliLitres += flowMilliLitres;
    
    // Print the flow rate for this second in litres / minute
    Serial.print("Flow rate: ");
    Serial.print(int(flowRate));  // Print the integer part of the variable
    Serial.print("L/min");
    Serial.print("\t");       // Print tab space

    // Print the cumulative total of litres flowed since starting
    Serial.print("Output Liquid Quantity: ");
    Serial.print(totalMilliLitres);
    Serial.print("mL / ");
    Serial.print(totalMilliLitres / 1000);
    Serial.println("L");
  }
 
}
/*  
 End of Flow meter function
*/

//****************************************************************************************************************************************************************************
//                                                                                            WIFI CONNECT

/*
   Start wifi functions
*/
void setup_wifi() {
//  scanNetworks();
  connectToNetwork();

//  Serial.println(WiFi.macAddress());
//  Serial.println(WiFi.localIP());
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
  //set wifi to station mode and disconnect from an AP if it was previously connected //test
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  
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
