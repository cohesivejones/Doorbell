#include <PubSubClient.h>
#include <WiFi.h>
#include "config.h"

WiFiClient espClient;
PubSubClient client(espClient);

void wifiConnect()
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

String clientId()
{
  String clientId = "ESP32_CLIENT-";
  clientId += String(random(0xffff), HEX);
  return clientId;
}

void mqttConnect()
{
  client.setServer(MQTT_SERVER, MQTT_PORT);
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(clientId().c_str(), MQTT_USER, MQTT_PASSWORD))
    {
      Serial.println("connected");
      client.publish("doorbell/active", "");
      delay(30000);
      client.publish("doorbell/inactive", "");
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

void GPIO_wake_up()
{
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  if (wakeup_reason != ESP_SLEEP_WAKEUP_EXT0)
  {
    return;
  }
  wifiConnect();
  mqttConnect();
  client.disconnect();
  WiFi.disconnect();
}

void setup()
{
  Serial.begin(9600);
  while (!Serial)
    ;

  GPIO_wake_up();
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_4, HIGH);
  Serial.println("Going to sleep...");
  esp_deep_sleep_start();
}

void loop()
{
}