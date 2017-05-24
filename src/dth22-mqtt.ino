/*
  ESP8266 + DHT22 + Photocell > MQTT
  Adapted from: https://github.com/projetsdiy/esp8266-dht22-mqtt-home-assistant
*/
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

#define WIFI_SSID           "xxx"
#define WIFI_PASSWORD       "xxx"

#define MQTT_SERVER         "xxx.xxx.xxx.xxx"
#define MQTT_CLIENT_NAME    "esp8266-1"
#define MQTT_TOPIC_TEMP     "sensor/esp8266-1/temperature"
#define MQTT_TOPIC_HUMID    "sensor/esp8266-1/humidity"
#define MQTT_TOPIC_LIGHT    "sensor/esp8266-1/light"

#define PHOTO_PIN            A0     // board pin of the photocell
#define DHT_PIN              D2     // board pin of the sensor
#define DHT_TYPE             DHT22  // sensor type : DHT 22  (AM2302)

#define PUBLISH_RATE         60     // publishing rate in seconds
#define DEEP_SLEEP           false  // deep sleep then reset or idling (D0 must be connected to RST) https://github.com/nodemcu/nodemcu-devkit-v1.0

#define DEBUG                false  // debug to serial port

// --- LIBRARIES INIT ---
WiFiClient    wifi;
PubSubClient  mqtt(MQTT_SERVER, 1883, wifi);
DHT           dht(DHT_PIN, DHT_TYPE);

// --- SETUP ---
void setup() {
  if (DEBUG) Serial.begin(9600);

  // wifi connexion
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while ( WiFi.status() != WL_CONNECTED ) {
    if (DEBUG) Serial.println("connecting Wifi");
    delay(500);
  }

  // sensor connexion
  dht.begin();
  if (DEBUG) Serial.println("connecting DHT");
}

// --- MAIN LOOP ---
void loop() {

  // reading DHT sensors
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if ( isnan(h) || isnan(t) ) {
    if (DEBUG) Serial.println(".");
    delay(2000);
    return;
  }

  // reading Photocell
  int p = analogRead(PHOTO_PIN);

  // debug
  if (DEBUG) {
    Serial.print("temp: ");
    Serial.print(t);
    Serial.print(" | humid: ");
    Serial.print(h);
    Serial.print(" | light: ");
    Serial.println(p);
  }

  // mqtt (re)connexion
  while ( !mqtt.connected() ) {
    if (DEBUG) Serial.println("connecting MQTT");
    if ( !mqtt.connect(MQTT_CLIENT_NAME) ) delay(500);
  }

  // publish to mqtt
  mqtt.publish(MQTT_TOPIC_TEMP,  String(t).c_str(), true);
  mqtt.publish(MQTT_TOPIC_HUMID, String(h).c_str(), true);
  mqtt.publish(MQTT_TOPIC_LIGHT, String(p).c_str(), true);

  // deep sleep (then reset) or just wait (then loop)
  if (DEEP_SLEEP) {
    ESP.deepSleep(PUBLISH_RATE * 1e6);
  } else {
    delay(PUBLISH_RATE * 1e3);
  }
}
