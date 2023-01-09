
#include "ESP8266WiFi.h"
#include "PubSubClient.h"
#include "Wire.h"
#include <LiquidCrystal_I2C.h>
#define NEOPIXEL_PIN 4  
LiquidCrystal_I2C lcd(0x27,16,2);
int len;
int in = 14;
int val = 1;

const char* ssid = "computer";  // Enter SSID here
const char* password = "prolink@123"; 
const char* mqtt_server = "172.16.12.139";

WiFiClient espClient22;
PubSubClient client(espClient22);
void ClearRow(int rowNum)
{
  lcd.setCursor(0,rowNum);
  lcd.print("                 ");
  lcd.setCursor(0,rowNum);
}

void setup_wifi() {
  delay(10);
  
  Serial.println();
 
  Serial.print("Connecting to ");
  Serial.println("EXCESS Office_NTFiber_2.4GHz");
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

// This functions is executed when some device publishes a message to a topic that your NodeMCU is subscribed to

String messageInfo;
int d =0;

void display(){
  for(int i=0; i<len; i++)
       {
         Serial.println("hi");
         ClearRow(0);
          lcd.print(messageInfo.substring(i,i+15));
         delay(500);
       }
}

void callback(String topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageInfo += (char)message[i];
  }
  Serial.println();

  // If a message is received on the topic room/lamp, you check if the message is either on or off. Turns the lamp GPIO according to the message
  if(topic=="/emergency/lcd"){
      Serial.print("Displaying message to users");
      // if(messageInfo == "allert"){
       len = messageInfo.length();
       d = 1;
      //  len = length;
      
       Serial.print("allert");
      // delay(1000);
      
      // else if(messageInfo == "safe"){
      //   lcd.setCursor(2,0);
      //  lcd.print("safe  ");
      //   lcd.setCursor(2,1);
      //  lcd.print("safe  "); 
      //   Serial.print("safe");
      // }
  }
  if(topic == "/emergency/stop"){
    d = 0;
    Serial.println("Stopping all emergency alert");
  }
  Serial.println();
}
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    
    if (client.connect("ESP8266Client22")) {
      Serial.println("connected");  
      // Subscribe or resubscribe to a topic
      // You can subscribe to more topics (to control more LEDs in this example)
      client.subscribe("/emergency/lcd");
      client.subscribe("/emergency/stop");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void setup() {
  lcd.init();
  lcd.clear();         
  lcd.backlight();
  Serial.begin(115200);
  pinMode(in,INPUT_PULLUP);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

}
void loop() {
  val = digitalRead(in);
  if (!client.connected()) {
    reconnect();
  }

if(!client.loop()){
    client.connect("ESP8266Client22");
      }
  if(d == 1 && val == 1){
    Serial.println("displaying");
    display();
  }
  if(d == 0 || val == 0){
    lcd.clear();
  }

}