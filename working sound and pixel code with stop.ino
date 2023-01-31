#include "ESP8266WiFi.h"
#include "PubSubClient.h"
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include "AudioFileSourceICYStream.h"
#include "AudioFileSourceBuffer.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2SNoDAC.h"
#include "ESP8266SAM.h"
#include<PubSubClient.h>

#define NEOPIXEL_PIN 4   

//for button input(D5) in and taking value
int in = 14;
int val = 1;
int md = 0; //for sound command

//making the audio source
AudioFileSourceBuffer *buff;
AudioOutputI2SNoDAC *out;

char demessage[50];   //char for playing the text

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(2, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
//defining SSID and Password and mqtt server
const char* ssid = "computer";
const char* password = "prolink@123";
const char* mqtt_server = "0.tcp.in.ngrok.io";
//for pubsub client
WiFiClient espClient22;
PubSubClient client(espClient22);
//setting up Wifi
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.print(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - NodeMCU IP address: ");
  Serial.println(WiFi.localIP());
}
int i = 0, j = 1; //for blinking led
void red() {
  pixels.setPixelColor(0, pixels.Color(i, 0, 0));
  pixels.show();
  Serial.print("red");
  if (i == 0) {
    i = 255;
  } else if (i == 255) {
    i = 0;
  }
  delay(200);
}
//callback function for mqtt
void callback(String topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageInfo;
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageInfo += (char)message[i];
  }
  Serial.println();
  // If a message is received on the topic emergency/rgb, We can toggle the Led based upon the message.
  if (topic == "/emergency/rgb") {
    Serial.print("Changing Room Light to ");
    if (messageInfo == "red") {
      j = 0;
    } 
    if (messageInfo == "white") {
      pixels.setPixelColor(0, pixels.Color(255, 255, 255));
      pixels.show();
      Serial.print("white");
      j = 1;
    }
  }
  if ( topic == "/emergency/say") {
      md = 1;
      Serial.print("hello");
      messageInfo.toCharArray(demessage, 50);
      Serial.print(md);
    }
    if (topic == "/emergency/stop"){
    md = 0;
    j = 1;
    pixels.setPixelColor(0, pixels.Color(255, 255, 255));
    pixels.show();
    Serial.print("white");
    Serial.println("stopimng emergency alert");
    }
  Serial.println();
}
void reconnect() {
  // Loop until we're reconnected to the client
  while (!client.connected()) {
    Serial.print("Trying MQTT CONNECTION ");
    if (client.connect("ESP8266Client1")) {
      Serial.println("connected");
      //subscribing to a topic emergency/rgb
      client.subscribe("/emergency/rgb");
      client.subscribe("/emergency/say");
      client.subscribe("/emergency/stop");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

//playing the speech
void play_sound(){
  ESP8266SAM *sam = new ESP8266SAM;
    Serial.println(demessage);
    sam->Say(out, demessage);
  Serial.println("playing"); 
  delay(2000);   
  delete sam;
}

void setup() {
  pixels.begin();
  Serial.begin(115200);
  pinMode(in,INPUT_PULLUP);
  setup_wifi();  //setting the wifi
  //setting up the mqtt
  client.setServer(mqtt_server, 15305);
  client.setCallback(callback);
  //making the audio source
  out = new AudioOutputI2SNoDAC();
  out->SetGain(0.8);  //setting the sound gain(increasing the volume)
  out->begin();
}
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  if (!client.loop())
    client.connect("ESP8266Client1");
  if (j == 0)
    red();
  val = digitalRead(in);
  // Serial.println(md);
  if(val == 1 && md == 1){
    // Serial.print("playing");
    play_sound();
  }
  if(val == 0){
    md = 0;
    pixels.setPixelColor(0, pixels.Color(255, 255, 255));
    pixels.show();
    Serial.print("white");
    j = 1;
  }
  // delay(1000);
}
