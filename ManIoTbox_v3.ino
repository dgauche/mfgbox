#define DHTTYPE DHT22 
#define Acel_X A3
#define Acel_Y A4
#define Acel_Z A5
#define ProxPin 34
#define ProxPinTwo 35
#define OneWirePin 18
#define PIRPIN 5
#define DHTTOPPIN 26
#define BATTLEVEL A18
#define ORANGE 27
#define GREEN 14
#define WIFI_BLUE 12
#define MQTT_BLUE 13
#define BATT_LOW_LEVEL 3000

#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <PubSubClient.h>
#include "DHT.h"




// CHANGE WIFI HERE
const char* ssid = "XRX";
const char* password = "52265226!!";
const char* mqtt_server = "mqtt.1worx.co";





WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

int minVal = 265;
int maxVal = 402;
float battery_val = 0;
float temp_val = 0;
float dht_h = 0;
float dht_t = 0;

double x;
double y;
double z;

volatile float prox1_val = 0;
volatile float prox2_val = 0;
volatile float pir_val = 0;

DHT dht(DHTTOPPIN, DHTTYPE);
OneWire oneWire(OneWirePin);
DallasTemperature sensors(&oneWire);

String BatteryLevel() {
  battery_val = analogRead(BATTLEVEL);

  if (battery_val < BATT_LOW_LEVEL) {
    digitalWrite(ORANGE, HIGH);
    digitalWrite(GREEN, LOW);
  }
  else {
    digitalWrite(ORANGE, LOW);
    digitalWrite(GREEN, HIGH);
  }
  String buf;
  buf += F(",\"batt\":");
  buf += String((battery_val / 4095.0) * 100.0, 0);
  
  
  return buf;
}

void ProxHandle1() {
    prox1_val += 1;
}

void ProxHandle2() {
    prox2_val += 1;
}


void PirHandle() {
  if (digitalRead(PIRPIN) == LOW) {
    pir_val = 1;
  } else {
    pir_val = 0;
  }
}


void setup() {
  Serial.begin(9600);
  pinMode(ORANGE, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(WIFI_BLUE, OUTPUT);
  pinMode(MQTT_BLUE, OUTPUT);
  pinMode(OneWirePin, INPUT);
  pinMode(ProxPin, INPUT);   
  pinMode(ProxPinTwo, INPUT);
  pinMode(PIRPIN, INPUT); 
  pinMode(BATTLEVEL, INPUT); 
  pinMode(Acel_X, INPUT); 
  pinMode(Acel_Y, INPUT); 
  pinMode(Acel_Z, INPUT); 
  attachInterrupt(digitalPinToInterrupt(ProxPin), ProxHandle1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ProxPinTwo), ProxHandle2, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIRPIN), PirHandle, CHANGE);
  dht.begin();
  
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(WIFI_BLUE, HIGH);
    delay(500);
    digitalWrite(WIFI_BLUE, LOW);
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  digitalWrite(WIFI_BLUE, HIGH);
}

void callback(char* topic, byte* message, unsigned int length) {
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
  
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      
      client.subscribe("esp32/output");
      digitalWrite(MQTT_BLUE, HIGH);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      
      delay(5000);
    }
  }
}

void loop() {
  

  if (!client.connected()) {
    digitalWrite(MQTT_BLUE, LOW);
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 100) {
    lastMsg = now;

    String Result = "";
    
  
    
    Result ="{"+prox()+prox2()+pir()+acc()+dhti()+temp()+BatteryLevel()+"}";



    Serial.println(Result);
    
   


     char charBuf[200];
     Result.toCharArray(charBuf, 200);
     uint64_t chipid = ESP.getEfuseMac();
     String chipid_s = String((unsigned long)((chipid & 0xFFFF0000) >> 16), HEX) + String((unsigned long)((chipid & 0x0000FFFF)), HEX);
     String res = String("esp32/" + chipid_s);
     char buff2[200];
     res.toCharArray(buff2, 200);
    client.publish(buff2, charBuf);


  }

 



}

String acc()
{
         
  x = analogRead(Acel_X);
  y = analogRead(Acel_Y);
  z = analogRead(Acel_Z);



  String buf;
  buf += F(",\"x\":");
  buf += String(x, 0);
  buf += F(",\"y\":");
  buf += String(y, 0);
  buf += F(",\"z\":");
  buf += String(z, 0);
  
  
  return buf;

  
}

String dhti()
{

  
  float h = dht.readHumidity();
 
  float t = dht.readTemperature();

  
  if ((!isnan(h)) && (!isnan(t))) {
      dht_t = t;
      dht_h = h;
  }


  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");


    String buf;
    buf += F(",\"DUTTemp\":");
    buf += String(dht_t, 2);
    buf += F(",\"DHTHum\":");
    buf += String(dht_h, 2);
  
  
    return buf;
  }

 
String temp()
{

   
   sensors.requestTemperatures(); 
   


   float Current_Temp = sensors.getTempCByIndex(0); 
   if (Current_Temp != -127.00) {
       temp_val = Current_Temp;
   }
 
   Serial.print("Temp: ");
   
   Serial.println(Current_Temp);

    String buf;
    buf += F(",\"Temp\":");
    buf += String(temp_val, 2);

   return buf;
}

String prox()
{
    String buf;
    buf += F("\"Prox\":");
    buf += String(prox1_val, 0);
    prox1_val = 0;
   return buf;
}

String prox2()
{
   String buf;
    buf += F(",\"Prox2\":");
    buf += String(prox2_val, 0);
    prox2_val = 0;
   return buf;
}

String pir(){

  String buf;
    buf += F(",\"Pir\":");
    buf += String(pir_val, 0);

   return buf;
  }

