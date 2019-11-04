#include "WiFi.h"
#include "HTTPClient.h"
#include "ArduinoJson.h"

//  =======================================================================================================

String pushCurrent = "0 ";
String pushVoltage = "0 ";
String pushPower = "0 ";
String pushTemp = "0 ";
String pushRH = "0 ";
String pushFlow = "0 ";
int flag=0;
//  =======================================================================================================
//  ---------------------------------------------- Energy Meter -------------------------------------------
//  =======================================================================================================

#include <Arduino.h>
#include "esp32ModbusRTU.h"
#include <algorithm>  // for std::reverse

String energyBuff = "NULL-Value";
esp32ModbusRTU modbus(&Serial1, 25);  // use Serial1 and pin 16 as RTS

/*
 * 
  SUCCES                = 0x00,
  ILLEGAL_FUNCTION      = 0x01,
  ILLEGAL_DATA_ADDRESS  = 0x02,
  ILLEGAL_DATA_VALUE    = 0x03,
  SERVER_DEVICE_FAILURE = 0x04,
  ACKNOWLEDGE           = 0x05,
  SERVER_DEVICE_BUSY    = 0x06,
  NEGATIVE_ACKNOWLEDGE  = 0x07,
  MEMORY_PARITY_ERROR   = 0x08,
  TIMEOUT               = 0xE0,
  INVALID_SLAVE         = 0xE1,
  INVALID_FUNCTION      = 0xE2,
  CRC_ERROR             = 0xE3,  // only for Modbus-RTU
  COMM_ERROR            = 0xE4  // general communication error
 */


String energyMeasure() {

//    Serial.print("sending Modbus request...\n");
    String retstr = "";
    
    //readHoldingRegisters(uint8_t slaveAddress, uint16_t address, uint16_t numberRegisters)
//    Serial.print("current\n");
    retstr += "Current: ";
    modbus.readHoldingRegisters(0x01, 148, 2);
    retstr += energyBuff + "A\n";
    pushCurrent = energyBuff;
    Serial.println(retstr);
    
    delay(1000); 
//    Serial.print("voltage\n");
    retstr += "Power: ";
    modbus.readHoldingRegisters(0x01, 140, 2); 
    retstr += energyBuff + "W\n";
    pushVoltage = energyBuff;
    Serial.println(retstr);
    
    delay(1000);
//    Serial.print("power(watts)\n");
    retstr += "Voltage: ";
    modbus.readHoldingRegisters(0x01, 140, 2); 
    retstr += energyBuff + "V\n";
    pushPower = energyBuff;
    Serial.println(retstr);

    energyBuff = "NULL-Value";
    return retstr;
}

//  -------------------------------------------------------------------------------------------------------
//  =======================================================================================================

//  =======================================================================================================
//  ---------------------------------------------- Temp sensor --------------------------------------------
//  =======================================================================================================
#include <Wire.h>;

// sda 21
// scl 22

#define si7021Addr 0x40
String tempStr;

void tempMeasure()
{
  unsigned int data[2];
  
  Wire.beginTransmission(si7021Addr);
  //Send humidity measurement command
  Wire.write(0xF5);
  Wire.endTransmission();
  delay(500);
    
  // Request 2 bytes of data
  Wire.requestFrom(si7021Addr, 2);
  // Read 2 bytes of data to get humidity
  if(Wire.available() == 2)
  {
    data[0] = Wire.read();
    data[1] = Wire.read();
  }
    
  // Convert the data
  float humidity  = ((data[0] * 256.0) + data[1]);
  humidity = ((125 * humidity) / 65536.0) - 6;

  Wire.beginTransmission(si7021Addr);
  // Send temperature measurement command
  Wire.write(0xF3);
  Wire.endTransmission();
  delay(500);
    
  // Request 2 bytes of data
  Wire.requestFrom(si7021Addr, 2);
  
  // Read 2 bytes of data for temperature
  if(Wire.available() == 2)
  {
    data[0] = Wire.read();
    data[1] = Wire.read();
  }

  // Convert the data
  float temp  = ((data[0] * 256.0) + data[1]);
  float celsTemp = ((175.72 * temp) / 65536.0) - 46.85;
  float fahrTemp = celsTemp * 1.8 + 32;
   
  // Output data to serial monitor
//  Serial.print("Humidity : ");
//  Serial.print(humidity);
//  Serial.println(" % RH");
//  Serial.print("Celsius : ");
//  Serial.print(celsTemp);
//  Serial.println(" C");
//  Serial.print("Fahrenheit : ");
//  Serial.print(fahrTemp);
//  Serial.println(" F");
  
  tempStr = "Humidity : " + (String)humidity + " % RH" + "\n";
  tempStr += "Celsius : " + (String)celsTemp + " C" + "\n" + "Fahrenheit : " + (String)fahrTemp + " F" + "\n";
  pushRH = (String)humidity;
  pushTemp = (String)celsTemp;
}
//  -------------------------------------------------------------------------------------------------------
//  ========================================================================================================



//  ========================================================================================================
//  ---------------------------------------------- Flow meter ---------------------------------------------- 
//  ========================================================================================================
#define LED_BUILTIN 2
#define SENSOR  14

long currentMillis = 0;
long previousMillis = 0;
int interval = 1000;
boolean ledState = LOW;
float calibrationFactor = 4.5;
volatile byte pulseCount;
byte pulse1Sec = 0;
float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;
String flowStr;

void IRAM_ATTR pulseCounter()
{
  pulseCount++;
}

int flowMeasure()
{
  currentMillis = millis();
  if (currentMillis - previousMillis > interval) 
  {
    pulse1Sec = pulseCount;
    pulseCount = 0;

    flowRate = ((1000.0 / (millis() - previousMillis)) * pulse1Sec) / calibrationFactor;

    flowMilliLitres = (flowRate / 60) * 1000;

    // Add the millilitres passed in this second to the cumulative total
    totalMilliLitres += flowMilliLitres;
    
    // Print the flow rate for this second in litres / minute
//    Serial.print("Flow rate: ");
//    Serial.print(int(flowRate));  // Print the integer part of the variable
//    Serial.print("L/min");
//    Serial.print("\t");       // Print tab space
//
//    // Print the cumulative total of litres flowed since starting
//    Serial.print("Output Liquid Quantity: ");
//    Serial.print(totalMilliLitres);
//    Serial.print("mL / ");
//    Serial.print(totalMilliLitres / 1000);
//    Serial.println("L");

    flowStr = "Flow rate: " + (String)((float)(flowRate)) + "L/min" + "\t";
    pushFlow = (String)((float)(flowRate));
    flowStr += "Output Liquid Quantity: " + (String)totalMilliLitres + "mL / " + (String)(totalMilliLitres / 1000) + "L" + "\n";
    return 1;
  }
  return 0;
}

//  -------------------------------------------------------------------------------------------------------
//  =======================================================================================================

char* wifi_ssid = "ChandlerBing";
char* wifi_pwd = "smellycatgeller";
char* wifi_ssid2="";
char* wifi_pwd2="";

String cse_ip = "139.59.42.21";

int ledFlag=0;
int LED_R = 4;
int LED_G = 5; 

//  =======================================================================================================
//  ---------------------------------------------- One M2M ------------------------------------------------
//  =======================================================================================================

StaticJsonBuffer<200> jsonBuffer;
//JsonObject& user_data = jsonBuffer.createObject();
//JsonObject& temp_user_data = jsonBuffer.createObject();
//JsonObject& sensor_data = jsonBuffer.createObject();

String cse_port = "8080";
String server = "http://"+cse_ip+":"+cse_port+"/~/in-cse/in-name/";
void ledUpdate(){
  if(ledFlag==1){
    digitalWrite(LED_R,0);
    digitalWrite(LED_G,1);
  }
  else{
    digitalWrite(LED_R,1);
    digitalWrite(LED_G,0);
  }
}

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
    ledFlag=0;
    ledUpdate();
  }
  delay(300);
}

void pushMyData(String pathh, String val){
  val = "\"" + val + "\"";
  pathh = "pr_3_esp32_1/"+pathh;
  //pathh = "pr_3_esp32_1";
  createCI(server, "Team15_Hostel_washing_machine_automation", pathh, val);  
}
//  -------------------------------------------------------------------------------------------------------
//  =======================================================================================================


void connect_to_WIFI(){
  int flag=1;
  WiFi.mode(WIFI_STA);// Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.disconnect();
  delay(100);
  WiFi.begin(wifi_ssid, wifi_pwd);
  Serial.println("Connecting to WiFi..");
  while (WiFi.status() != WL_CONNECTED || WiFi.status()==WL_CONNECT_FAILED){
    delay(500);
    Serial.print(".");
  }
  if(WiFi.status()==WL_CONNECTED){
    Serial.println("Connected to the WiFi network");
    ledFlag=1;
    ledUpdate();
  }
  else{
    
  }
  Serial.println("Connected to the WiFi network");
}

void setup()
{
  Serial.begin(115200);
  ledUpdate();
  connect_to_WIFI();

  pinMode(LED_R, OUTPUT);
  pinMode(LED_G,OUTPUT);
  Serial.println("Setup done");

//  ========================================================================================================
//  ---------------------------------------------- Energy Meter --------------------------------------------
//  ========================================================================================================

  Serial1.begin(9600, SERIAL_8E1, 26, 27);  // Modbus connection

  modbus.onData([](uint8_t serverAddress, esp32Modbus::FunctionCode fc, uint8_t* data, size_t length) {
//    Serial.printf("id 0x%02x fc 0x%02x len %u: 0x", serverAddress, fc, length);
    
    for (size_t i = 0; i < length; ++i) {
      //Serial.printf("%02x", data[i]);
//      Serial.printf("\n/%.2f\n",data[i]);
    }
    
    uint8_t data2[4];
    data2[0] = data[1];
    data2[1] = data[0];
    data2[2] = data[3];
    data2[3] = data[2];

//    Serial.printf("\nval: %.2f", *reinterpret_cast<float*>(data2));
    energyBuff = (String)(*reinterpret_cast<float*>(data2));
    flag=1;
    
  });
  modbus.onError([](esp32Modbus::Error error) {
    flag=0;
    Serial.printf("Got error: 0x%02x\n\n", static_cast<uint8_t>(error));
  });
  modbus.begin();

//  -------------------------------------------------------------------------------------------------------


//  ========================================================================================================
//  ---------------------------------------------- Flow meter ---------------------------------------------- 
//  ========================================================================================================

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(SENSOR, INPUT_PULLUP);

  pulseCount = 0;
  flowRate = 0.0;
  flowMilliLitres = 0;
  totalMilliLitres = 0;
  previousMillis = 0;

  attachInterrupt(digitalPinToInterrupt(SENSOR), pulseCounter, FALLING);

//  -------------------------------------------------------------------------------------------------------


//  =======================================================================================================
//  ---------------------------------------------- Temp sensor --------------------------------------------
//  =======================================================================================================
  Wire.begin();
  Wire.beginTransmission(si7021Addr);
  Wire.endTransmission();
  delay(300);
//  -------------------------------------------------------------------------------------------------------
//  =======================================================================================================

}

void loop()
{
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connection lost.. trying to reconnect");
    ledFlag=0;ledUpdate();
    connect_to_WIFI();
  }

pushCurrent = "NULL-Value";
pushVoltage = "NULL-Value";
pushPower = "NULL-Value";
pushTemp = "NULL-Value";
pushRH = "NULL-Value";
pushFlow = "NULL-Value";

  tempMeasure();
  Serial.println(tempStr);

  previousMillis = millis();
  while(!flowMeasure());

  String energyStr = energyMeasure();
  if(flag)
  {
    Serial.printf("Printing energyStr\n");
    Serial.print(energyStr);
  }
  else
  {
    Serial.println("Error in getting from meter");
  }
  // Send data to OneM2M server


  //String masterstr = "(" + pushTemp + "," + pushRH + "," + pushPower + "," + pushCurrent + ")";
//  pushMyData("em/em_1_vll_avg", masterstr);/

  pushMyData("em/em_1_vll_avg", pushPower);
 pushMyData("fm/fm_1_pump_flowrate", pushFlow);
  pushMyData("oe/oe_1_temperature", pushTemp);
  pushMyData("oe/oe_1_rh", pushRH);
  pushMyData("em/em_1_watts_total", pushVoltage);
  pushMyData("em/em_1_current_total", pushCurrent);
  Serial.printf("\nPushed\n");

  delay(10000); // DO NOT CHANGE THIS LINE 10 min delay
//  while(1);
  
}
