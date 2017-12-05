/*************************************************************
  Download latest Blynk library here:
    https://github.com/blynkkk/blynk-library/releases/latest

  Blynk is a platform with iOS and Android apps to control
  Arduino, Raspberry Pi and the likes over the Internet.
  You can easily build graphic interfaces for all your
  projects by simply dragging and dropping widgets.

    Downloads, docs, tutorials: http://www.blynk.cc
    Sketch generator:           http://examples.blynk.cc
    Blynk community:            http://community.blynk.cc
    Follow us:                  http://www.fb.com/blynkapp
                                http://twitter.com/blynk_app

  Blynk library is licensed under MIT license
  This example code is in public domain.

 *************************************************************

  This example shows how value can be pushed from Arduino to
  the Blynk App.

  WARNING :
  For this example you'll need Adafruit DHT sensor libraries:
    https://github.com/adafruit/Adafruit_Sensor
    https://github.com/adafruit/DHT-sensor-library

  App project setup:
    Value Display widget attached to V5
    Value Display widget attached to V6
 *************************************************************/

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
#include "config.h"

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
//char auth[] = "Put your token here"; 

// Your WiFi credentials.
// Set password to "" for open networks.
//char ssid[] = "Put your SSID here";
//char pass[] = "Put your password here";

#define DHTPIN 2          // What digital pin we're connected to  (SDA pin on SkarkFun EPS8266 Thing)
#define ledPin 5          // ESP8266 SparrkFun thing's on board LED

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11     // DHT 11
#define DHTTYPE DHT22   // DHT 22, AM2302, AM2321
//#define DHTTYPE DHT21   // DHT 21, AM2301

#define BLYNK_GREEN     "#23C48E"
#define BLYNK_BLUE      "#04C0F8"
#define BLYNK_YELLOW    "#ED9D00"
#define BLYNK_RED       "#D3435C"
#define BLYNK_DARK_BLUE "#5F7CD8"
WidgetLED led1(V13);

DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;
int   FLG_Simulation;
float Sim_Temperature;
float Sim_Humidity;
float Temperature;
float Humidity;
float OverTemperature;
float UnderTemperature;
int   TotalUpdates = 0;
int   FLG_SendNewFanOn  = 0;
int   FLG_SendNewFanOff = 0;
 
// update Temperature and Hmumidity
void sendSensor()
{
  // blank line to show new data easily to user
  Serial.println();
  Serial.println("Updating Server..." );

  if ( FLG_Simulation == 0 ) {
    Temperature = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit
    Humidity    = dht.readHumidity();

    if (isnan(Temperature) || isnan(Humidity)) {
      Serial.println("Failed to read from DHT sensor...   Using Random Numbers");
      Temperature = random(10, 50 );
      Humidity    = random(80, 100);
    }
  } else {
    Temperature = Sim_Temperature;
    Humidity    = Sim_Humidity;
  }

  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(V1, Temperature);
  Blynk.virtualWrite(V2, Humidity   );

  if ( FLG_Simulation == 1 ) {
    Serial.print  ("Simuation Active: ");
  }

  // tell user what the values are
  Serial.print  ("New Temperature: "  );
  Serial.print  (Temperature          );
  Serial.print  (", New Humidity: "   );
  Serial.println(Humidity             );

  // assume Temperature is OK
  int TemperatureOK = 1;

  if ( Temperature > OverTemperature ) {
    TemperatureOK = 0;
    digitalWrite(ledPin, 1);
    led1.setColor(BLYNK_RED);
    Serial.println("Temperature has gone too High...");

    // ONLY send new requests
    if ( FLG_SendNewFanOn == 0 ) {
      FLG_SendNewFanOn  = 1;
      FLG_SendNewFanOff = 0;   // next time send it
    
      // trigger a webhook to tell IFTTT to turn on the DLink Smart plug connected to the fan
      //  --> This will tell the Smart plug EVERY time through here... thus if the plug is OFFLINE, then it will get told over and over until it gets the message
      Blynk.virtualWrite(V15, Temperature);
      Serial.println    ("Write to Webhook V15 to control DLink Smart Plug");
    }
  }

  if ( Temperature < UnderTemperature ) {
    TemperatureOK = 0;
    digitalWrite(ledPin, 0);
    led1.setColor(BLYNK_BLUE);
    Serial.println("Temperature has gone too Low");

    if ( FLG_SendNewFanOff == 0 ) {
      FLG_SendNewFanOff = 1;
      FLG_SendNewFanOn  = 0;   // next time send it
      
      // trigger a webhook to tell IFTTT to turn off the DLink Smart plug connected to the fan
      //  --> This will tell the Smart plug EVERY time through here... thus if the plug is OFFLINE, then it will get told over and over until it gets the message
      Blynk.virtualWrite(V16, Temperature);
      Serial.println    ("Write to Webhook V16 to control DLink Smart Plug");
    }
  }

  if ( TemperatureOK == 1 ) {
    // Control LED Based on temperature
    led1.setColor(BLYNK_GREEN);
    Serial.println("Temperature is Normal");
  }

  // wifi RSSI
  Serial.print("Wifi Signal Strength: ");
  Serial.println(WiFi.RSSI());

  // total updates
  TotalUpdates++;
  Serial.print("Total Server Updates: ");
  Serial.println(TotalUpdates);
}

BLYNK_CONNECTED() {
  // Request Blynk server to re-send latest values for all pins
  Blynk.syncAll();
}

// This function will be called every time Slider Widget in Blynk app writes values to the Virtual Pin
BLYNK_WRITE(V8)
{
  OverTemperature = param.asFloat(); // assigning incoming value from pin to a variable

  Serial.print  ("New Over Limit: ");
  Serial.println(OverTemperature   );
}

// This function will be called every time Slider Widget in Blynk app writes values to the Virtual Pin
BLYNK_WRITE(V9)
{
  UnderTemperature = param.asFloat(); // assigning incoming value from pin to a variable

  Serial.print  ("New Under Limit: ");
  Serial.println(UnderTemperature   );
}

// This function will be called every time Slider Widget in Blynk app writes values to the Virtual Pin
BLYNK_WRITE(V10)
{
  FLG_Simulation = param.asInt(); // assigning incoming value from pin to a variable

  Serial.print("Simulaton is now: ");
  if ( FLG_Simulation == 1 ) {
    Serial.println("ON");
  } else {
    Serial.println("OFF");
  }
}

// This function will be called every time Slider Widget in Blynk app writes values to the Virtual Pin
BLYNK_WRITE(V11)
{
  Sim_Temperature = param.asFloat(); // assigning incoming value from pin to a variable

  Serial.print("Simuated Temperaute is: ");
  Serial.println(Sim_Temperature);
}

// This function will be called every time Slider Widget in Blynk app writes values to the Virtual Pin
BLYNK_WRITE(V12)
{
  Sim_Humidity = param.asFloat(); // assigning incoming value from pin to a variable

  Serial.print("Simulated Humidity: ");
  Serial.println(Sim_Humidity);
}

void setup()
{
  // Debug console
  Serial.begin(115200);

  Blynk.begin(auth, ssid, pass);
  // You can also specify server:
  //Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 8442);
  //Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8442);

  // setp Sensor
  dht.begin();

  // setup LED pin
  pinMode(ledPin, OUTPUT);

  // Turn LED on, so colors are visible
  led1.on();

  // Setup a function to be called every so often
  timer.setInterval(10000L, sendSensor);
}

void loop()
{
  Blynk.run();
  timer.run();
}

