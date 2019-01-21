#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <MQTTClient.h>
#include "DHTesp.h"

DHTesp dht;
int sensorPin = 2;
const char* ssid     = "";
const char* password = "";
const char* mqtt_server = "";
const char* mqtt_id = "TemperatureSensor";
char* mqtt_temp_topic = "Home/Temperature";
char* mqtt_humi_topic = "Home/Humidity";
WiFiClient net;
MQTTClient client;

// Firmware update setup
const char* update_path = "/firmware";
const char* update_username = "admin";
const char* update_password = "admin";

// Main sensor variables;
float temperature = 0;
float humidity = 0;
unsigned long previousMillis = 0;
int interval;

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;

void setup() {
  Serial.begin(115200);
  Serial.print("\n\r \n\rWorking to connect");
  // Initialize sensor
  dht.setup(sensorPin,DHTesp::DHT22);
  
  // Connect to WiFi network
  WiFi.begin(ssid, password);
  
 
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("DHT Weather Reading Server");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Let's connect to MQTT
  client.begin(mqtt_server, net);
  while (!client.connect(mqtt_id)) 
  {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("MQTT Server connected!");
  
  server.on("/", handle_root);
  
  server.on("/info", [](){  // if you add this subdirectory to your webserver call, you get text below :)
    String webString=String(temperature)+" C/"+String(humidity)+"%";
    server.send(200, "text/plain", webString);                   // send to someones browser when asked
  });
  httpUpdater.setup(&server, update_path, update_username, update_password);
  server.begin();
  Serial.println("HTTP server started.");

  interval = dht.getMinimumSamplingPeriod();
   
}

void loop() {
  server.handleClient();
  
  if(!client.connected()) {
        connect();
  } 
   
  humidity = dht.getHumidity();
  temperature = dht.getTemperature();
  Serial.println("Current conditions: "+ String(temperature) +"º C / "+ String(humidity) +"% Humidity");
  client.publish(mqtt_temp_topic, String(temperature));
  client.publish(mqtt_humi_topic, String(humidity)+ "%");
  delay(5000);
}

void handle_root() {
  server.send(200, "text/plain", "Hello from the weather esp8266, read data from /info");
  delay(100);
}


//Connect to wifi and MQTT
void connect() {
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(1000);
    WiFi.begin(ssid, password);
  }

  
  while (!client.connect(mqtt_id)) 
  {
    delay(1000);
  }
  Serial.println("is connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}
