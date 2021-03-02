#include <PubSubClient.h>
#include <WiFi.h>
#include "config.h"

WiFiClient espClient;
PubSubClient client(espClient);

const char *INACTIVE = "doorbell/inactive";
const char *ACTIVE = "doorbell/active";
const char *BUZZER = "doorbell/buzzer";

int THIRTY_SECONDS = 30000;
unsigned long time_now = 0;

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
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(clientId().c_str(), MQTT_USER, MQTT_PASSWORD))
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

void mqttSendActive()
{
  Serial.println("Send Active");
  mqttConnect();
  client.publish(ACTIVE, "test");
  time_now = millis();
  client.subscribe(BUZZER);
  while (millis() < time_now + THIRTY_SECONDS)
  {
    client.loop();
  }
  if (client.publish(INACTIVE, "test"))
  {
    Serial.println("Send Inactive");
  }
}

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.println("Recieved Buzzer");
  digitalWrite(GPIO_NUM_13, HIGH);
  delay(2000);
  digitalWrite(GPIO_NUM_13, LOW);
}

void GPIO_wake_up()
{
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  if (wakeup_reason != ESP_SLEEP_WAKEUP_EXT0)
  {
    return;
  }
  wifiConnect();
  mqttSendActive();
  client.disconnect();
  WiFi.disconnect();
}

void setup()
{
  Serial.begin(9600);
  while (!Serial)
    ;

  pinMode(GPIO_NUM_13, OUTPUT);
  digitalWrite(GPIO_NUM_13, LOW);
  client.setServer(MQTT_SERVER, MQTT_PORT);
  client.setCallback(callback);
  GPIO_wake_up();
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_4, HIGH);
  Serial.println("Going to sleep...");
  esp_deep_sleep_start();
}

void loop()
{
}