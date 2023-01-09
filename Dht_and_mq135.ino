#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <MQ135.h>
#include <DHT.h>
#include <ArduinoJson.h>

const char* SSID = "EXCESS Office_NTFiber_2.4GHz";
const char* PW = "excess_xtech";
const char* mqtt_server = "192.168.1.5";

#define PIN_MQ135 A0  // MQ135 Analog Input Pin
#define DHTPIN 2      // DHT Digital Input Pin
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
//defining float variables
float temperature, humidity;

long now = millis();
long lastMeasure = 0;

WiFiClient espClient;
PubSubClient client(espClient);
//callback
void callback(String topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("TRYING MQTT CONNECTION...");
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  WiFi.disconnect();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PW);

  // Try forever
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("...Connecting to WiFi");
    delay(1000);
  }
  Serial.println("Connected");
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    delay(2000);
    return;
  }

  if (!client.connected()) {
    reconnect();
  }
  if (!client.loop())
    client.connect("ESP8266Client");
  now = millis();
  char result[8];
  if (now - lastMeasure > 3000) {
    lastMeasure = now;
    MQ135 gasSensor = MQ135(A0, 76.63, 2.2);
    float air_quality = gasSensor.getCorrectedPPM(temperature, humidity);
    // Buffer big enough for 7-character float
    dtostrf(air_quality, 6, 2, result);
    // client.publish("/emergency/gas", result);
    Serial.print("Air Quality: ");
    Serial.print(air_quality);
    Serial.println("  PPM");
    Serial.println();
  }
  DynamicJsonBuffer JSONbuffer;
  JsonObject& mainData = JSONbuffer.createObject();

  mainData["temperature"] = temperature;
  mainData["humidity"] = humidity;
  mainData["airGas"] = result;

  char JSONmessageBuffer[100];
  mainData.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
  Serial.println("Sending message to MQTT topic..");
  Serial.println(JSONmessageBuffer);
  client.publish("/emergency/gas", JSONmessageBuffer);
  delay(2000);
}