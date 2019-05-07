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

#define timeSeconds 5

// Set GPIOs for LED and PIR Motion Sensor
const int led = 27;
const int motionSensor = 26;
const int tempPin = 25;     //analog input pin constant
const int buzzerPin = 22;
int tempVal;    // temperature sensor raw readings

float volts;    // variable for storing voltage 

float temp;     // actual temperature variable

const int f = 349;
const int gS = 415;
const int a = 440;
const int cH = 523;
const int eH = 659;
const int fH = 698;
const int e6 = 1319;
const int g6 = 1568;
const int a6 = 1760;
const int as6 = 1865;
const int b6 = 1976;
const int c7 = 2093;
const int d7 = 2349;
const int e7 = 2637;
const int f7 = 2794;
const int g7 = 3136;




// Timer: Auxiliary variables
unsigned long now = millis();
unsigned long lastTrigger = 0;
boolean startTimer = false;


// Use hardware SPI
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

const char *wifiSsid = "AndroidAP"; // change this to the ssid of the network you want to connect to
const char *wifiPassword =  "1234567890"; // change this to the password of the network you want to connect to

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
// Check state of proximity
void parseData()
{   
    String response1 = returnFilteredResponse("http://julianlin.net/proximity");
    Serial.println(response1);
    if (response1 == "ON"){
      pinMode(buzzerPin, HIGH);  
    }
    //return response1;
}


// Checks if motion was detected, sets LED HIGH and starts a timer
void IRAM_ATTR detectsMovement() {
  Serial.println("MOTION DETECTED!!!");
  digitalWrite(led, HIGH);
  pinMode(buzzerPin, OUTPUT);
  startTimer = true;
  lastTrigger = millis();
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
  parseData();
  pinMode(motionSensor, INPUT_PULLUP);
  // Set motionSensor pin as interrupt, assign interrupt function and set RISING mode
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);

  // Set LED to LOW
  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);
  // Current time
  now = millis();
  // Turn off the LED after the number of seconds defined in the timeSeconds variable
  if(startTimer && (now - lastTrigger > (timeSeconds*1000))) {
    Serial.println("Motion stopped...");
    //pinMode(buzzerPin, OUTPUT);
    digitalWrite(led, LOW);
    startTimer = false;

  }
}
void beep(int tone, int duration)
{
  for (long i = 0; i < duration * 900L; i += tone * 1)
  {
    digitalWrite(buzzerPin, LOW);
    delayMicroseconds(tone*(.50));
    digitalWrite(buzzerPin, HIGH);
    delayMicroseconds(tone*(.50));
  }
  delay(30);
}

void  MarioTheme()
{
  beep(e7,150);
  beep(e7,150);
  delay(150);
  beep(e7,150);  
  delay(150);
  beep(c7,150);
  beep(e7,150);
  delay(150);
  beep(g7,150);
  delay(450);
  beep(g6,150);
  delay(450);
  beep(c7,150);
  delay(300);
  beep(g6,150);
  delay(300);
  beep(e6,150);
  delay(300);
  beep(a6,150);
  delay(150);
  beep(b6,150);
  delay(150);
  beep(as6,150);
  beep(a6,150);
  delay(150);
  beep(g6,112);
  beep(e7,112); 
  beep(g7,112);
  beep(a6,150);
  delay(150);
  beep(f7,150);
  beep(g7,150);
  delay(150);
  beep(e7,150);
  delay(150); 
  beep(c7,150);
  beep(d7,150);
  beep(b6,150);
}

void loop() {

    now = millis();
  // Turn off the LED after the number of seconds defined in the timeSeconds variable
  if(startTimer && (now - lastTrigger > (timeSeconds*1000))) {
    Serial.println("Motion stopped...");
    digitalWrite(led, LOW);
    MarioTheme();
    delay(3000);
    
         //read the temp sensor and store it in tempVal
 tempVal = analogRead(tempPin);
 volts = tempVal/1023.0;             // normalize by the maximum temperature raw reading range

 temp = (volts - 0.5) * 100 ;         //calculate temperature celsius from voltage as per the equation found on the sensor spec sheet.
Serial.print(tempVal);
Serial.print("\n");
Serial.print(" Temperature is:   ");
// print out the following string to the serial monitor
Serial.print(temp);                  // in the same line print the temperature
Serial.println (" degrees C");
Serial.print("\n"); 
//Serial.print(volts);
//beep(g7,150);// still in the same line print degrees C, then go to next line.

delay(100);                         // wait for 1 second or 1000 milliseconds before taking the next reading. 

    startTimer = false;
  }
  delay(30000);
}
