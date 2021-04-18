#include <BLEDevice.h>
#include <BLEAdvertisedDevice.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "config.h"

/* Doorbell Service: 00001523-1212-EFDE-1523-785FEABCD123
 * Buzzer : 00001524-1212-EFDE-1523-785FEABCD123
 * openDoor : 00001525-1212-EFDE-1523-785FEABCD123
 */
static BLEUUID serviceUUID("00001523-1212-EFDE-1523-785FEABCD123");
static BLEUUID buzzerUUID("00001524-1212-EFDE-1523-785FEABCD123");
static BLEUUID openDoorUUID("00001525-1212-EFDE-1523-785FEABCD123");

BLEAdvertisedDevice *device;

BLEClient *pClient;
WiFiClient espClient;
PubSubClient client(espClient);

bool wifiConnect()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.println("Attempting WiFi connection");
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
  if (pClient)
  {
    BLERemoteService *pRemoteService = pClient->getService(serviceUUID);
    BLERemoteCharacteristic *pRemoteCharacteristic = pRemoteService->getCharacteristic(openDoorUUID);
    pRemoteCharacteristic->writeValue(1);
  }
}

bool mqttConnect()
{
  while (!client.connected())
  {
    client.setServer(MQTT_SERVER, MQTT_PORT).setCallback(callback);
    Serial.print("Attempting MQTT connection...");
    if (client.connect(clientId().c_str(), MQTT_USER, MQTT_PASSWORD))
    {
      Serial.println("Connected");
      return true;
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.println(client.state());
      delay(500);
    }
  }
  return client.connected();
}

class MyClientCallback : public BLEClientCallbacks
{
  void onConnect(BLEClient *pclient)
  {
  }

  void onDisconnect(BLEClient *pclient)
  {
    client.publish(DOORBELL_INACTIVE, EMPTY_MESSAGE);
    client.unsubscribe(DOORBELL_BUZZER);
    delete device;
  }
};

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
  void onResult(BLEAdvertisedDevice advertisedDevice)
  {
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID))
    {
      BLEDevice::getScan()->stop();
      device = new BLEAdvertisedDevice(advertisedDevice);
    }
  }
};

void Scan()
{
  BLEDevice::init("");
  BLEScan *pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
}

void setup()
{
  Serial.begin(9600);
  while (!Serial)
    delay(10);

  Serial.println("Doorbel Client");
  Serial.println("------------------------------\n");

  wifiConnect();
  mqttConnect();
  Scan();
}

void loop()
{
  if (device)
  {
    if (!pClient)
    {
      Serial.print("Device found: ");
      Serial.println(device->getName().c_str());
      pClient = BLEDevice::createClient();
      pClient->setClientCallbacks(new MyClientCallback());
      pClient->connect(device);
      BLERemoteService *pRemoteService = pClient->getService(serviceUUID);
      BLERemoteCharacteristic *pRemoteCharacteristic = pRemoteService->getCharacteristic(buzzerUUID);
      notify_callback cb = [](BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify) {
        client.publish(DOORBELL_ACTIVE, EMPTY_MESSAGE);
        delay(3000);
      };
      pRemoteCharacteristic->registerForNotify(cb);
      client.subscribe(DOORBELL_BUZZER);
    }
  }
  client.loop();
}