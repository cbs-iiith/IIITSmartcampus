#include "WiFi.h"
#include "HTTPClient.h"
#include "ArduinoJson.h"
#include <stdlib.h>
#include "SHT21.h"

char wifi_ssid[20] = "ChandlerBing";
char wifi_pwd[20] = "smellycatgeller";

//String cse_ip = "192.168.43.59";/
String cse_ip = "139.59.42.21";
////////////////////////////////////////
//Sensor Setup
//------------------------------------//

#include <Wire.h>

#define I2C_SDA                        27
#define I2C_SCL                        14

//SHT21 defines
#define SHT21_POLLING_INTERVAL         10000
#define SHT21_ADDR                     0x40
void sensor_temp_rh();
void setup_i2c();
float* sht21_buffer = new float[2];

//------------------------------------//

////////////////////////////////////////

StaticJsonBuffer<200> jsonBuffer;
JsonObject& user_data = jsonBuffer.createObject();
JsonObject& temp_user_data = jsonBuffer.createObject();
JsonObject& sensor_data = jsonBuffer.createObject();

String cse_port = "8080";
String server = "http://"+cse_ip+":"+cse_port+"/~/in-cse/in-name/";


//String server = "http://"+cse_ip+":"+cse_port+"/~/in-cse/";


String createCI(String server, String ae, String cnt, String val)
{
  HTTPClient http;
  http.begin(server + ae + "/" + cnt + "/");
//  http.begin(server + ae + "/" + cnt);
//  String postRequest = String() + "POST " + url + " HTTP/1.1\r\n" +
//                       "Host: " + CSE_IP + ":" + CSE_HTTP_PORT + "\r\n" +
//                       "X-M2M-Origin: " + CSE_M2M_ORIGIN + "\r\n" +
//                       "Content-Type: application/json;ty=" + ty + "\r\n" +
//                       "Content-Length: " + rep.length() + "\r\n"
//                       "Connection: close\r\n\n" +
//                       rep;
//                       
  http.addHeader("X-M2M-Origin", "admin:admin");
  http.addHeader("Content-Type", "application/json;ty=4");
  http.addHeader("Content-Length", "100");
  http.addHeader("Connection", "close");
  int code = http.POST("{\"m2m:cin\": {\"cnf\": \"text/plain:0\",\"con\": "+ String(val) +"}}");
  http.end();
  Serial.print(code);
  delay(300);
}


//int keyIndex = 0;                                // your network key Index number
//int status = WL_IDLE_STATUS;  
void setup()
{
  
  Serial.begin(115200);
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  WiFi.begin(wifi_ssid, wifi_pwd);
//  WiFi.setSleep(false);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println("Connected to the WiFi network");
  //LED ON CODE INDICATING CONN TO WIFI

  //call begin to start DHT sensor
//  pinMode(DHTPIN, INPUT);
//  dht.begin();
  pinMode(12,OUTPUT);
  digitalWrite(12,HIGH);
  setup_i2c();
  sensor_temp_rh();

  Serial.println("Setup done");
}

void loop()
{
  //Serial.print(n);
  //Serial.println(" networks found");

  //Take DHT sensor state
  //float t = 0.2;

//  UNCOMMENT THIS
//  float t = dht.readTemperature();
//  Serial.print("Temperature: ");
//  Serial.print(t);
//  Serial.println("");


  
  //float h = 0.1;

//UNCOMMENT THIS  
//  float h = dht.readHumidity();
//  Serial.print("Humidity: ");
//  Serial.print(h);
//  Serial.println("");

  //user_data.printTo(Serial);


  //To GET values
  //http.begin("http://192.168.0.113:8080/~/in-cse/in-name/verte001/DATA/la");
  //http.begin("http://192.168.0.113:8080/~/in-cse");
  //int httpCode = http.GET();
//  sensor_temp_rh();
  float t = sht21_buffer[1];
  t = t/1000;
  float h = sht21_buffer[0];
  Serial.print("Temperature ");
  Serial.print(t);
  Serial.print("Humidity ");
  Serial.print(h);
  // when single sensor gives multiple values
  String sensor1 = "(" + String(t) + "," + String(h) + ")"; 

  //when single sensor gives single value
//  String sensor2 = String(10); 

  // Make it a single string
  String sensor_string = sensor1;

  // Make it OneM2M complaint
  sensor_string = "\"" + sensor_string + "\""; // DO NOT CHANGE THIS LINE

  // Send data to OneM2M server
  createCI(server, "Team15_Hostel_washing_machine_automation", "pr_3_esp32_1", sensor_string);
  
  delay(15000); // DO NOT CHANGE THIS LINE
}

void setup_i2c() {
  //i2c setup
  
  //TESTING: not sure if input pullup is required
  //pinMode(I2C_SCL, INPUT_PULLUP);           // set pin to input
  //pinMode(I2C_SDA, INPUT_PULLUP);           // set pin to input
  
  if (!Wire.begin(I2C_SDA, I2C_SCL, 100000)) { //100000 is the clock frequency, according to sht21 datasheet there is no minimum freq
    Serial.printf("Failed to start I2C!\n");
    while (1);
  }
  Serial.print("I2C setup");
  Wire.beginTransmission(SHT21_ADDR);
  Wire.endTransmission();
  delay(300);
}

void sensor_temp_rh() {
  static uint32_t lastMillisSH21 = 0;
  if (millis() - lastMillisSH21 > SHT21_POLLING_INTERVAL) {
    lastMillisSH21 = millis();
    sht21_buffer[0] = Humidity();
    sht21_buffer[1] = Temperature();
    Serial.printf("Humidity(%RH): %.2f, Temperature(C): %.2f\n", sht21_buffer[0], sht21_buffer[1]);
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
  return temperature;
}
