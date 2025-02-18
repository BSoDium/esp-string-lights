#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* mqtt_server = "openhabian.local";

WiFiClient espClient;
PubSubClient client(espClient);

const char* topic = "home/livingroom/light";
const int ledPin = 2;  // Adjust based on your setup

void setup_wifi() {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("WiFi connected");
}

void callback(char* topic, byte* payload, unsigned int length) {
    payload[length] = '\0';  // Ensure null termination
    String message = String((char*)payload);

    if (message.equals("ON")) {
        Serial.println("Turning light on");
        digitalWrite(ledPin, HIGH);
    } else if (message.equals("OFF")) {
        Serial.println("Turning light off");
        digitalWrite(ledPin, LOW);
    }
}

void reconnect() {
    int attempts = 0;
    while (!client.connected() && attempts < 10) {
        if (client.connect("ESP32Light")) {
            client.subscribe(topic);
        } else {
            delay(5000);
            attempts++;
        }
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(ledPin, OUTPUT);
    setup_wifi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
    Serial.println("Setup complete");
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();
}