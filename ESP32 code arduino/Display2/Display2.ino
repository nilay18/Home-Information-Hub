#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <WiFi.h>
#include <WiFiAP.h>
#include <HTTPClient.h>
//#include <WiFiClient.h>
#include "ArduinoJson.h"


// For the Adafruit shield, these are the default.
#define TFT_DC 2
#define TFT_CS 5
#define TFT_RST 4

// Use hardware SPI (on Uno, #13, #12, #11) and the above for CS/DC
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

const char *wifiSsid = "AndroidAP"; // change this to the ssid of the network you want to connect to
const char *wifiPassword =  "1234567890"; // change this to the password of the network you want to connect to

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;
struct tm timeinfo;
float temper;  
int tempVal;    // temperature sensor raw readings
float volts;    // variable for storing voltage 
const int tempPin = 25;     //analog input pin constant
void printLocalTime()
{
  
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");  
}

String returnFilteredResponse(String url) {
  HTTPClient http;
  
  http.begin(url);
  int httpCode = http.GET();

  Serial.println("httpcode");
  Serial.println(httpCode);
  
  if (httpCode > 0) { //Check for the returning code
       
      String payload = http.getString();
      http.end();
      Serial.println(payload);
      return payload;
  }
  http.end();
}

void parseData()
{
    tft.setRotation(1);
    tft.fillScreen(ILI9341_BLACK);
    unsigned long start = micros();
    tft.setCursor(0, 0);
    tft.setTextSize(3);
    tft.setTextColor(ILI9341_RED);  
    //Display Time
    tft.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");

    // Display Current Weather

   String response = returnFilteredResponse("http://julianlin.net/current_weather"); 
    StaticJsonBuffer<1000> jsonBuffer;   
    JsonObject& weatherJson = jsonBuffer.parseObject(response);
    Serial.println("Inside Parse Data");
    tft.setTextSize(3);
    tft.setTextColor(ILI9341_WHITE);  
  

    const char* temp = weatherJson["temp"];
    char* reminder2 = (char*) temp;
    tft.println("Current Temperature:");
    tft.println(reminder2);
    
// Display Room Temperature 

    tempVal = analogRead(tempPin);
    volts = tempVal/1023.0;             // normalize by the maximum temperature raw reading range
    temper = (volts - 0.5) * 100 ;         //calculate temperature celsius from voltage as per the equation found on the sensor spec sheet.
    
    tft.println("Room Temperature:");
    tft.println(temper);

//    Weather Forecast

    String response1 = returnFilteredResponse("http://julianlin.net/weather_forecast");  
   
    JsonObject& forecastJson = jsonBuffer.parseObject(response1);
    Serial.println("Inside Parse Data");

    const char* description = forecastJson["description"];
    char* remind2 = (char*) description;
    tft.println("Weather Forecast:"); 
    tft.println(remind2);
    Serial.println(remind2);

    //Maps showing ETA
    
    String response3 = returnFilteredResponse("http://julianlin.net/maps");  
    JsonObject& mapsJson = jsonBuffer.parseObject(response3);
    Serial.println("Inside Parse Data");
  
    const char* r = mapsJson["rows"][0]["elements"][0]["duration"]["text"];
    char* reminder = (char*) r;
    tft.println("Estimated time for arrival:");
    tft.println("");
    Serial.println(reminder);
}

WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  Serial.println("Seting up everything, Hang on!"); 
 
  tft.begin();
  WiFi.begin(wifiSsid, wifiPassword);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

}

void loop() {
  parseData();
  delay(30000);
}
