/*
   esp8266 + ds18b20 on GPIO0
*/
#include <ESP8266WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 0     // GPIO 0 pin of ds18b20
// The resolution of the temperature sensor is user-configurable to 9, 10, 11, or 12 bits, 
// corresponding to increments of 0.5°C, 0.25°C, 0.125°C, and 0.0625°C, respectively.
#define TEMPERATURE_PRECISION 11

const char* ssid     = "";
const char* password = "";

const char* host = "";
const char* url = "";
const char* deviceId = "";

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);
// Number of temperature devices found
int numberOfDevices;
// We'll use this variable to store a found device address
DeviceAddress tempDeviceAddress;

// to get Vcc
ADC_MODE(ADC_VCC);

void setup() {
  Serial.begin(9600);
  delay(10);
  setupWifi();
  setupDS();
}

void setupWifi()
{
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

void setupDS()
{
  Serial.println("Starting up Dallas Temperature IC");
  // Start up the library
  sensors.begin();

  // Grab a count of devices on the wire
  numberOfDevices = sensors.getDeviceCount();

  // locate devices on the bus
  Serial.println("Locating devices...");
  Serial.print("Found ");
  Serial.print(numberOfDevices, DEC);
  Serial.println(" devices.");

  if (numberOfDevices == 0) {
    delay(1000);
    setupDS();
  }

  // report parasite power requirements
  Serial.print("Parasite power is: ");
  if (sensors.isParasitePowerMode()) {
    Serial.println("ON");
  }
  else {
    Serial.println("OFF");
  }

  // Loop through each device, print out address
  for (int i = 0; i < numberOfDevices; i++)
  {
    // Search the wire for address
    if (sensors.getAddress(tempDeviceAddress, i))
    {
      Serial.print("Found device ");
      Serial.print(i, DEC);
      Serial.print(" with address: ");
      Serial.print(getAddress(tempDeviceAddress));
      Serial.println();

      Serial.print("Setting resolution to ");
      Serial.println(TEMPERATURE_PRECISION, DEC);

      // set the resolution to TEMPERATURE_PRECISION bit (Each Dallas/Maxim device is capable of several different resolutions)
      sensors.setResolution(tempDeviceAddress, TEMPERATURE_PRECISION);

      Serial.print("Resolution actually set to: ");
      Serial.print(sensors.getResolution(tempDeviceAddress), DEC);
      Serial.println();
    } else {
      Serial.print("Found ghost device at ");
      Serial.print(i, DEC);
      Serial.print(" but could not detect address. Check power and cabling");
    }
  }  
}

void loop()
{
  Serial.println();
  
  // read Vcc
  float vcc = ESP.getVcc();
  Serial.print("Vcc: ");
  Serial.println(vcc);
  
  // read temp from ds18b20
  Serial.println("Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures

  // content start 
  String content = "[";

  // Loop through each device, print out temperature data
  for (int i=0;i<numberOfDevices; i++) {
    // Search the wire for address
    if (sensors.getAddress(tempDeviceAddress, i)) {
      String deviceId = getAddress(tempDeviceAddress);
      float temp = sensors.getTempC(tempDeviceAddress);
      Serial.print("Temperature for device ");
      Serial.print(deviceId);
      Serial.print(": ");
      Serial.println(temp);

      if (i > 0) {
        content += ",";
      }
      content += "{\"deviceId\": \"" + deviceId + "\",\"temperature\": " + temp + ",\"vcc\": " + vcc + "}";
    } 
    //else ghost device! Check your power requirements and cabling  
  }

  // content end
  content += "]";
  postToAPI(content);

  delay(55000); // Send data every ~1 minute
}

// function to get a device address as a string
String getAddress(DeviceAddress deviceAddress)
{
  String address;
  
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) {
      address += "0";
    }
    address += String(deviceAddress[i], HEX);
  }

  address.toLowerCase();

  return address;
}

void postToAPI(String content)
{
  Serial.print("Connecting to ");
  Serial.println(host);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  String post = "POST " + String(url) + " HTTP/1.1\r\n" +
                "Host: " + host + "\r\n" +
                "Content-Length:" + content.length() + "\r\n\r\n" +
                content;
  Serial.println(post);

  // This will send the request to the server
  client.print(post);
  // wait for couple of seconds for response
  delay(2000);

  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  Serial.println();
  Serial.println("closing connection");
}

