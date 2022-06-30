#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <FastLED.h>
#include <bits/stdc++.h>
#include "config.h"

WiFiClient espClient;
PubSubClient client(espClient);

std::map<std::string, CRGB> colorMap{
        {"green", CRGB::Green},
        {"blue", CRGB::Blue},
        {"red", CRGB::Red},
        {"orange", CRGB::OrangeRed},
        {"magenta", CRGB::Magenta}
};

CRGB leds[NUM_LEDS];
const CRGB defaultColor = CRGB::Blue;
CRGB currentColor = defaultColor;

void callback(char* topic, byte* payload, unsigned int length) {

    // Print the topic/payload to the Serial
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i=0;i<length;i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();

    // Handle the topic and payload
    if (strcmp(topic, MQTT_IN_TOPIC) == 0) {

        // convert the byte array to a char array
        char color[length + 1];
        memcpy(color, payload, length);
        color[length] = '\0';

        if (colorMap.count(color)) {
            currentColor = colorMap[color];
        } else {
            currentColor = defaultColor;
        }
        client.publish(MQTT_OUT_TOPIC, color);
    }
}

void reconnect() {
    // Loop until we're reconnected
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        if (client.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS)) {
            Serial.println("connected");
            // Once connected, publish an announcement...
            client.publish(MQTT_OUT_TOPIC,"LedStrip connected");
            // ... and resubscribe
            client.subscribe(MQTT_IN_TOPIC);
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

void setupWiFi() {
    Serial.print("Connecting to: ");
    Serial.println(SECRET_SSID);

    WiFi.begin(SECRET_SSID, SECRET_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
    Serial.println();
    Serial.println("WiFi connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

void setupMQTT() {
    client.setServer(MQTT_SERVER, MQTT_PORT);
    client.setCallback(callback);
}

void setup() {
    Serial.begin(115200);
    setupWiFi();
    setupMQTT();
    FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    EVERY_N_MILLISECONDS(1000) {
        for(auto & led : leds) {
            led = currentColor;
        }
        FastLED.show();
    }
}