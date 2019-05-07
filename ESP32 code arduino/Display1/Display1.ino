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
  tft.fillScreen(ILI9341_BLACK);
  unsigned long start = micros();
  tft.setCursor(0, 0);
    tft.setRotation(1);
    tft.setTextSize(3);
    tft.setTextColor(ILI9341_RED);  
    // Display Time
    tft.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");

    // Gmail
   String response = returnFilteredResponse("http://julianlin.net/gmail"); 
    StaticJsonBuffer<1000> jsonBuffer; 
    JsonObject& gmailJson = jsonBuffer.parseObject(response);
    tft.setTextSize(1.8);
    tft.setTextColor(ILI9341_WHITE);  
    tft.println("");
  
    const char* dateReceived = gmailJson["date_received"];
    const char* sender = gmailJson["sender"];
    const char* title = gmailJson["title"];
    char* rem1 = (char*) dateReceived;
    char* rem2 = (char*) sender;
    char* rem3 = (char*) title;
    tft.println("Date Received:");
    tft.println(rem1);
    tft.println("Sender:");
    tft.println(rem2);
    tft.println("Title:");
    tft.println(rem3);
    Serial.println(rem1);
    Serial.println(rem2);
    Serial.println(rem3);

// Calendar
  
    String response1 = returnFilteredResponse("http://julianlin.net/calendar");  
   
    JsonObject& calendarJson = jsonBuffer.parseObject(response1);
    Serial.println("Inside Parse Data");

    const char* startTime = calendarJson["start_time"];
    const char* eventName = calendarJson["title"];
    char* reminder2 = (char*) startTime;
    char* reminder3 = (char*) eventName;
    tft.println(""); 
    tft.println("UPCOMING EVENTS");
    
    tft.println("Event Time:"); 
    tft.println(reminder2);
    tft.println("Title:"); 
    tft.println(reminder3);
    Serial.println(reminder2);
    Serial.println(reminder3);

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
