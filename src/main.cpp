#include <Arduino.h>
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32) || defined(ARDUINO_ARCH_RP2040)
#include <WiFi.h>
#endif

#include "SinricPro.h"
#include "SinricProSwitch.h"

#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASS "YOUR_WIFI_PASSWORD"
#define APP_KEY "6aa9e2f4-ffcf-421f-a70d-e1ecf4f42212"
#define APP_SECRET "ab1c35e4-2e54-4c07-b051-c8a761579138-ef2885db-54e8-4b32-8464-29cd283101e3"
#define SWITCH_ID "679aada6f25540068f8cad02"

#if defined(ESP8266)
#define RELAYPIN_1 12
#elif defined(ESP32)
#define RELAYPIN_1 16
#elif (ARDUINO_ARCH_RP2040)
#define RELAYPIN_1 6
#endif

#define BAUD_RATE 115200 // Change baudrate to your need

bool onPowerState1(const String &deviceId, bool &state)
{
  Serial.printf("Device 1 turned %s", state ? "on" : "off");
  digitalWrite(RELAYPIN_1, state ? LOW : HIGH);

  return true; // request handled properly
}

// setup function for WiFi connection
void setupWiFi()
{
  Serial.printf("\r\n[Wifi]: Connecting");

#if defined(ESP8266)
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.setAutoReconnect(true);
#elif defined(ESP32)
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);
#endif

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.printf(".");
    delay(250);
  }

  Serial.printf("connected!\r\n[WiFi]: IP-Address is %s\r\n", WiFi.localIP().toString().c_str());
}

// setup function for SinricPro
void setupSinricPro()
{
  // add devices and callbacks to SinricPro
  pinMode(RELAYPIN_1, OUTPUT);

  SinricProSwitch &mySwitch1 = SinricPro[SWITCH_ID];
  mySwitch1.onPowerState(onPowerState1);

  // setup SinricPro
  SinricPro.onConnected([]()
                        { Serial.printf("Connected to SinricPro\r\n"); });
  SinricPro.onDisconnected([]()
                           { Serial.printf("Disconnected from SinricPro\r\n"); });

  SinricPro.begin(APP_KEY, APP_SECRET);
}

// main setup function
void setup()
{
  Serial.begin(BAUD_RATE);
  Serial.printf("\r\n\r\n");
  setupWiFi();
  setupSinricPro();
}

void loop()
{
  SinricPro.handle();
}