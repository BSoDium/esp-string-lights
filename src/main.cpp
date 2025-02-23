#include <WiFi.h>
#include <PubSubClient.h>
#include <vector>
#include <algorithm>
#include "config.h"

#define RELAY_PIN 16
#define LED_AND_BUTTON_PIN 2
#define TRANSISTOR_BASE_PIN 4

WiFiClient espClient;
PubSubClient client(espClient);

const char *availabilityTopic = "home/livingroom/string-light/availability";
const char *powerTopic = "home/livingroom/string-light/power";
const char *effectTopic = "home/livingroom/string-light/effect";

bool currentPowerState = false;

std::vector<std::string> effects = {"COMBINATION", "IN_WAVES", "SEQUENTIAL", "SLO_GLO", "CHASING", "FADE", "TWINKLE", "STATIC"};
int currentEffectIndex = 0;

int lastButtonState = LOW;

/** Setup WiFi connection */
void setup_wifi()
{
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("WiFi connected");
}

/** Load an effect by cycling through the effects */
void load_effect(std::string effect)
{
    const int effectIndex = std::find(effects.begin(), effects.end(), effect) - effects.begin();
    if (effectIndex == currentEffectIndex)
    {
        Serial.println("Effect already loaded: " + String(effect.c_str()));
        return;
    }

    const int circularIndexDelta = (effectIndex > currentEffectIndex) ? effectIndex - currentEffectIndex : effects.size() - currentEffectIndex + effectIndex;
    Serial.println("Effect index: " + String(effectIndex) + ", current index: " + String(currentEffectIndex) + ", delta: " + String(circularIndexDelta));

    pinMode(LED_AND_BUTTON_PIN, OUTPUT);
    for (int i = 0; i < circularIndexDelta; i++)
    {
        digitalWrite(TRANSISTOR_BASE_PIN, HIGH);
        digitalWrite(LED_AND_BUTTON_PIN, HIGH);
        delay(75);
        digitalWrite(TRANSISTOR_BASE_PIN, LOW);
        digitalWrite(LED_AND_BUTTON_PIN, LOW);
        delay(25);
    }
    pinMode(LED_AND_BUTTON_PIN, INPUT_PULLUP);
    Serial.println("Effect loaded: " + String(effect.c_str()));
}

/** Callback function for MQTT messages */
void callback(char *topic, byte *payload, unsigned int length)
{
    payload[length] = '\0'; // Ensure null termination
    String message = String((char *)payload);
    String topicStr = String(topic);

    if (topicStr.equals(powerTopic))
    {
        if (message.equals("ON"))
        {
            Serial.println("Turning light on");
            digitalWrite(RELAY_PIN, HIGH);
            currentPowerState = true;

            delay(500);

            // If the effect has been changed while the light was off, load the new effect
            if (currentEffectIndex != 0)
            {
                const int targetEffectIndex = currentEffectIndex;
                Serial.println("Loading effect: " + String(effects[targetEffectIndex].c_str()));
                currentEffectIndex = 0;
                load_effect(effects[targetEffectIndex]);
                currentEffectIndex = targetEffectIndex;
            }
        }
        else if (message.equals("OFF"))
        {
            Serial.println("Turning light off");
            digitalWrite(RELAY_PIN, LOW);
            currentPowerState = false;
        }
    }
    else if (topicStr.equals(effectTopic))
    {
        std::string effectStr = message.c_str();
        if (std::find(effects.begin(), effects.end(), effectStr) != effects.end() && effectStr != effects[currentEffectIndex])
        {
            if (currentPowerState)
            {
                Serial.println("Loading effect: " + message);
                load_effect(effectStr);
            }
            else
            {
                Serial.println("Light is off, effect will be loaded when light is turned on");
            }
            currentEffectIndex = std::find(effects.begin(), effects.end(), effectStr) - effects.begin();
        }
        else if (effectStr != effects[currentEffectIndex])
        {
            Serial.println("Invalid effect received: " + message);
        }
        else
        {
            Serial.println("Effect already loaded: " + message);
        }
    }
}

/** Reconnect to MQTT server */
void reconnect()
{
    int attempts = 0;
    while (!client.connected() && attempts < 10)
    {
        if (client.connect("ESP32-String-Lights", availabilityTopic, 0, true, "OFF"))
        {
            client.subscribe(powerTopic);
            client.subscribe(effectTopic);

            client.publish(availabilityTopic, "ON", true);
        }
        else
        {
            delay(5000);
            attempts++;
        }
    }
}

/** Setup function */
void setup()
{
    Serial.begin(115200);

    pinMode(RELAY_PIN, OUTPUT);
    pinMode(LED_AND_BUTTON_PIN, INPUT_PULLUP);
    pinMode(TRANSISTOR_BASE_PIN, OUTPUT);

    setup_wifi();

    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);

    Serial.println("Setup complete, MQTT client connected");
}

/** Main loop */
void loop()
{
    // Reconnect if necessary
    if (!client.connected())
    {
        reconnect();
    }
    client.loop();

    // Read button state and check for changes
    int currentButtonState = digitalRead(LED_AND_BUTTON_PIN);
    if (currentButtonState != lastButtonState)
    {
        if (currentButtonState == HIGH)
        {
            // Publish the new effect
            Serial.println("Button pressed, cycling effects");
            const int targetEffectIndex = (currentEffectIndex + 1) % effects.size();
            client.publish(effectTopic, effects[targetEffectIndex].c_str(), true);
        }
        lastButtonState = currentButtonState;
    }
}