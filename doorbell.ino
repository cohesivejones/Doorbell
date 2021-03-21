#include <PubSubClient.h>
#include <WiFi.h>
#include "config.h"

WiFiClient espClient;
PubSubClient client(espClient);

const char *INACTIVE = "doorbell/inactive";
const char *ACTIVE = "doorbell/active";
const char *BUZZER = "doorbell/buzzer";
const char *BATTERY = "doorbell/battery";

const float PIN_RESOLUTION = 4095.0;
const float VOLTAGE_DIVIDED = 2.0;
const float VOLTAGE_LOW = 3.67;
const float VOLTAGE_HIGH = 4.20;
const float VOLTAGE_GRADIENT = (100.0 / (VOLTAGE_HIGH - VOLTAGE_LOW));
int THIRTY_SECONDS = 30000;
int TWO_SECONDS = 2000;
unsigned long time_now = 0;

void wifiConnect()
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
  time_now = millis();
  while (millis() < time_now + TWO_SECONDS)
  {
  }
  digitalWrite(GPIO_NUM_13, LOW);
}

void mqttConnect()
{
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(callback);
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(clientId().c_str(), MQTT_USER, MQTT_PASSWORD, INACTIVE, 1, 0, "Ungracefull disconect"))
    {
      Serial.println("connected");
      return;
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.println(client.state());
      delay(1000);
    }
  }
}

float voltage()
{
  float vIn = analogRead(A13) / PIN_RESOLUTION;
  float voltage = (vIn * VOLTAGE_DIVIDED * 3.3 * 1.1);
  return (voltage - VOLTAGE_LOW) * VOLTAGE_GRADIENT;
}

void mqttSend()
{
  mqttConnect();

  Serial.println("Send Active");
  client.publish(ACTIVE, "test");

  Serial.println("Send Battery status");
  client.publish(BATTERY, String(voltage()).c_str());

  Serial.println("Wait for Buzzer");
  time_now = millis();
  client.subscribe(BUZZER);
  while (millis() < time_now + THIRTY_SECONDS)
  {
    client.loop();
  }

  Serial.println("Send Inactive");
  client.publish(INACTIVE, "test");
  time_now = millis();
  while (millis() < time_now + TWO_SECONDS)
  {
    client.loop();
  }

  Serial.println("Disconnect MQTT");
  client.disconnect();
}

void GPIO_wake_up()
{
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  if (wakeup_reason != ESP_SLEEP_WAKEUP_EXT0)
  {
    return;
  }
  wifiConnect();
  mqttSend();
  WiFi.disconnect();
}

void setup()
{
  Serial.begin(9600);
  while (!Serial)
    ;

  pinMode(GPIO_NUM_13, OUTPUT);
  digitalWrite(GPIO_NUM_13, LOW);
  GPIO_wake_up();
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_4, HIGH);
  Serial.println("Going to sleep...");
  esp_deep_sleep_start();
}

void loop()
{
}