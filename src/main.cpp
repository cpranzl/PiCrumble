/**
 * PiCrumble
 */

#include "Arduino.h"
#include "WiFi.h"
#include "OneWire.h"
#include "DallasTemperature.h"
#include "PubSubClient.h"

// Define used pins
#define LED_BUILTIN 13
#define TOUCHPAD 15
#define ONEWIRE 2

// Define version
#define VERSION "0.0.1"

// WiFi credentials
WiFiClient wifiCrumble;
const char* WIFI_SSID = "DLF_guest";
const char* WIFI_PASS = "wlan4guest";

// OneWire
OneWire onewire(ONEWIRE);

// DS18B20
DallasTemperature sensors(&onewire);
int sensorCount = 0;
float temperature;

// MQTT Broker
PubSubClient mqttCrumble;
const char* MQTT_SERVER = "10.0.0.105";
const int MQTT_PORT = 1883;

// Buttons
bool buttonPressed = false;
unsigned long timePressed;
unsigned long timeReleased;
unsigned long timeHold;

void setup()
{
  // Start serial
	Serial.begin(115200);

  delay(2000);

  Serial.print("Running PiCrumble ");
  Serial.println(VERSION);

  // Connect to WiFi
  connect_wifi();

  mqttCrumble.setServer(MQTT_SERVER, MQTT_PORT);
  mqttCrumble.setCallback(on_message);

  // Start up the DallasTemperature library
  sensors.begin();

  // Locate devices on the bus
  sensorCount = sensors.getDeviceCount();

  if (sensorCount > 0)
  {
    Serial.print(sensorCount);
    Serial.println(" devices found.");
  }
  else
  {
    Serial.println("Error 002: No devices found.");
  }
  // initialize LED digital pin as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  // initialize TOUCHPAD digital pin as an input
  pinMode(TOUCHPAD, INPUT);
}

void loop()
{

  // If touchpad is pressed, light the LED and write to the serial
  if (digitalRead(TOUCHPAD) && buttonPressed == false)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    buttonPressed = true;
    timePressed = millis();

    Serial.println("Button pressed");
  }

  // If Touchpad is released, deactivate LED and write to the serial
  if (!(digitalRead(TOUCHPAD)) && buttonPressed == true)
  {
    digitalWrite(LED_BUILTIN, LOW);
    buttonPressed = false;
    timeReleased = millis();
    timeHold = timeReleased - timePressed;
    
    Serial.println("Button released");
    Serial.print("Button was hold for ");
    Serial.print(timeHold);
    Serial.println(" milliseconds");
  }

  //
  if (sensorCount > 0)
  {
    // Issue global temperature request to all devices on the bus
    sensors.requestTemperatures();
    
    //
    for (size_t i = 0; i < sensorCount; i++)
    {
      temperature = sensors.getTempCByIndex(i);

      if (temperature != DEVICE_DISCONNECTED_C)
      {
        Serial.print("Temperature for device ");
        Serial.print(i);
        Serial.print(" is: ");
        Serial.print(temperature);
        Serial.println("degree Celsius.");
      }
      else
      {
        Serial.print("Error 003: Could not read temperature data for device ");
        Serial.println(i);
      }
    }
  }
}

void connect_wifi()
{
  // Connect to Wifi.
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.println("Connecting...");

  while (WiFi.status() != WL_CONNECTED)
  {
    // Check to see if connecting failed. This is due to incorrect credentials
    if (WiFi.status() == WL_CONNECT_FAILED)
    {
      Serial.print("Error 001: Failed to connect to WIFI. Please verify credentials: SSID: ");
      Serial.print(WIFI_SSID);
      Serial.print(" Password: ");
      Serial.println(WIFI_PASS);
    }

    delay(5000);
  }

  Serial.println("WiFi connected. IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect_wifi()
{
  // Loop until connected
  while(!wifiCrumble.connected())
  {

  }
}

void on_message(char* topic, byte* message, unsigned int length)
{
  Serial.print("Message arrived on topic ");
  Serial.print(topic);
  Serial.print(" with content: ");
  for (size_t j = 0; j < length; j++)
  {
    Serial.print((char)message[j]);
  }
}