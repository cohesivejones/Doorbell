#include <PubSubClient.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include "config.h"

WiFiClient espClient;
PubSubClient client(espClient);

const char *INACTIVE = "doorbell/inactive";
const char *ACTIVE = "doorbell/active";
const char *BUZZER = "doorbell/buzzer";
const char *BATTERY = "doorbell/battery";
const char *EMPTY_MESSAGE = "test";

const float PIN_RESOLUTION = 4095.0;
const float VOLTAGE_DIVIDED = 2.0;
const float VOLTAGE_LOW = 3.67;
const float VOLTAGE_HIGH = 4.20;
const float VOLTAGE_GRADIENT = (100.0 / (VOLTAGE_HIGH - VOLTAGE_LOW));
const int THIRTY_SECONDS = 30000;
const int TWO_SECONDS = 2000;
const int MAX_RETRIES = 3;

const unsigned long DEEP_SLEEP_TIME = 15L;

unsigned long timeNow = 0;

bool wifiConnect()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.println("Connecting");
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      Serial.print(".");
    }

    Serial.print("Connected, IP address: ");
    Serial.println(WiFi.localIP());
  }
  return (WiFi.status() == WL_CONNECTED);
}

String clientId()
{
  String clientId = "ESP32_CLIENT-";
  clientId += String(random(0xffff), HEX);
  return clientId;
}

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.println("Recieved Buzzer");
  digitalWrite(GPIO_NUM_13, HIGH);
  timeNow = millis();
  while (millis() < timeNow + TWO_SECONDS)
  {
  }
  digitalWrite(GPIO_NUM_13, LOW);
}

bool mqttConnect()
{
  int tries = 0;
  client.setServer(MQTT_SERVER, MQTT_PORT)
    .setCallback(callback);
  Serial.print("Attempting MQTT connection...");
  while (!client.connected() && tries < MAX_RETRIES)
  {
    if (client.connect(clientId().c_str(), MQTT_USER, MQTT_PASSWORD))
    {
      Serial.println("connected");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.println(client.state());
      delay(500);
      tries++;
    }
  }
  return client.connected();
}

float voltage()
{
  float vIn = analogRead(A13) / PIN_RESOLUTION;
  float voltage = (vIn * VOLTAGE_DIVIDED * 3.3 * 1.1);
  return (voltage - VOLTAGE_LOW) * VOLTAGE_GRADIENT;
}

void mqttSendBatteryStatus() {
  if (wifiConnect() && mqttConnect()) {
    Serial.println("Send Battery status");
    client.publish(BATTERY, String(voltage()).c_str());
    Serial.println("Disconnect MQTT");
    client.disconnect();
  }
}

void mqttSendActiveStatus()
{
  if (wifiConnect() && mqttConnect()) {
    Serial.println("Send Active");
    client.publish(ACTIVE, EMPTY_MESSAGE);

    Serial.println("Wait for Buzzer");
    client.subscribe(BUZZER);
    timeNow = millis();
    while (millis() < timeNow + THIRTY_SECONDS)
    {
      client.loop();
    }

    Serial.println("Send Inactive");
    client.publish(INACTIVE, EMPTY_MESSAGE);
    timeNow = millis();
    while (millis() < timeNow + TWO_SECONDS)
    {
      client.loop();
    }

    Serial.println("Disconnect MQTT");
    client.disconnect();
  }
}

void goToDeepSleep()
{
  Serial.println("Going to sleep...");
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);

  // Configure the timer to wake us up!
  if(ESP_OK!=esp_sleep_enable_timer_wakeup(DEEP_SLEEP_TIME * 60L * 1000000L)) {
    Serial.println("Error: failed to set timer wakeup");
  }
  if(ESP_OK!=esp_sleep_enable_ext0_wakeup(GPIO_NUM_4, HIGH)) {
    Serial.println("Error: failed to set ext wakeup");
  }

  // Go to sleep! Zzzz
  esp_deep_sleep_start();
}

void configureWakeUp()
{
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  switch(wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0 :
      mqttSendActiveStatus();
      break;
    case ESP_SLEEP_WAKEUP_TIMER :
      mqttSendBatteryStatus();
      break;
    default :
      break;
  }
}

void setup()
{
  Serial.begin(9600);
  while (!Serial)
    ;

  // Set buzzer voltage low
  pinMode(GPIO_NUM_13, OUTPUT);
  digitalWrite(GPIO_NUM_13, LOW);

  configureWakeUp();

  goToDeepSleep();
}

void loop()
{
}