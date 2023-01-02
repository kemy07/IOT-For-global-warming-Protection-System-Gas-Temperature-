#include <Arduino.h>
#include <DHT.h>
#define DHTPIN D2
#define DHTTYPE DHT22
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
DHT dht(DHTPIN, DHTTYPE);
//Enter your network credentials
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

//Enter Firebase web API Key
#define API_KEY "AIzaSyDXXYo-7C_F8dSPQLD2hZx4AT-nCI01WQI"

// Enter Authorized Email and Password
#define USER_EMAIL "Supersuperadmin@gmail.com"
#define USER_PASSWORD "159875321"

// Enter Realtime Database URL
#define DATABASE_URL "gas-tem-humidity-default-rtdb.firebaseio.com/"

FirebaseData Firebase_dataObject;
FirebaseAuth authentication;
FirebaseConfig config;

String UID;
// Database main path 
String database_path;
String tempPath = "/temperature";
String humPath = "/humidity";
String gasPath = "/gas";
String time_path = "/epoch_time";

//Updated in every loop
String parent_path;

int epoch_time;
float temperature;
float humidity;
float gaz;
FirebaseJson json;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
//send new readings every 5 minutes
unsigned long previous_time = 0;
unsigned long Delay = 300000;
unsigned long sendDataPrevMillis = 0;
unsigned long timerDelay = 180000;
//get current epoch time
unsigned long Get_Epoch_Time() {
  timeClient.update();
  unsigned long now = timeClient.getEpochTime();
  return now;
}

void setup(){
  Serial.begin(115200);
  dht.begin();
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
  Serial.println();
  
  timeClient.begin();

  config.api_key = API_KEY;
  authentication.user.email = USER_EMAIL;
  authentication.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);
  Firebase_dataObject.setResponseSize(4096);

  config.token_status_callback = tokenStatusCallback; 
  config.max_token_generation_retry = 5;

  Firebase.begin(&config, &authentication);

  Serial.println("Getting User UID...");
  while ((authentication.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }
  UID = authentication.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.println(UID);
  database_path = "/Data/" + UID + "Readings";
}

void loop(){
  gaz = analogRead(A0);
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  
  if (Firebase.ready() && (millis() - previous_time > Delay || previous_time == 0))
{
  previous_time = millis();
  epoch_time = Get_Epoch_Time();
  Serial.print ("time: ");
  Serial.println (epoch_time);
  parent_path= database_path + "/" + String(epoch_time);
  json.set(time_path, String(epoch_time));
  json.set(tempPath.c_str(), float(dht.readTemperature()));
  json.set(humPath.c_str(), float(dht.readHumidity()));
  json.set(gasPath.c_str(), String(gaz));
  Serial.printf("Set json...%s\n",Firebase.RTDB.setJSON(&Firebase_dataObject, parent_path.c_str(), &json) ? "ok" : Firebase_dataObject.errorReason().c_str());
  
    
  }
  
    
}
