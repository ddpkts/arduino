/*
 * esp8266 + dht22 on GPIO0
 */
#include <ESP8266WiFi.h>
#include "DHT.h"

#define DHTPIN 0     // GPIO 0 pin of ESP8266
#define DHTTYPE DHT22   // DHT 22  (AM2302)

const char* ssid     = "";
const char* password = "";

const char* host = "";
const char* url = "";
const char* deviceId = "";

DHT dht(DHTPIN, DHTTYPE,30); // 30 is for cpu clock of esp8266 80Mhz

ADC_MODE(ADC_VCC);

void setup() {
  Serial.begin(9600);

  dht.begin();
  
  delay(10);

  Serial.println();
  Serial.println();
  Serial.print("ESP8266 Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {  
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius
  float t = dht.readTemperature();
  // vcc
  float vcc = ESP.getVcc();
  
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || (h == 0 && t == 0)) {
    Serial.println("Failed to read from DHT sensor!");
    delay(1000);
    return;
  }

  Serial.print("Temperature: "); 
  Serial.print(t);
  Serial.print("C\t");
  Serial.print("Humidity: "); 
  Serial.print(h);
  Serial.println("%\t");
  Serial.println(vcc);

  Serial.print("Connecting to ");
  Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }  
  
  String content = "[{\"deviceId\": \"" + String(deviceId) + "\",\"temperature\": " + t + ",\"humidity\": " + h + ",\"vcc\": " + vcc + "}]";  
  String post = "POST " + String(url) + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Content-Length:" + content.length() + "\r\n\r\n" + 
               content;
  Serial.println(post);
  
  // This will send the request to the server
  client.print(post);
  delay(1000);
  
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  
  Serial.println();
  Serial.println("closing connection");
  delay(55000); // Send data every 1 minute
}
