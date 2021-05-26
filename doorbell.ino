#include <BLEDevice.h>
#include <BLEAdvertisedDevice.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"

/* Doorbell Service: 00001523-1212-EFDE-1523-785FEABCD123
 * outsideButton : 00001524-1212-EFDE-1523-785FEABCD123
 * openDoor : 00001525-1212-EFDE-1523-785FEABCD123
 */
static BLEUUID serviceUUID("00001523-1212-EFDE-1523-785FEABCD123");
static BLEUUID outsideButtonUUID("00001524-1212-EFDE-1523-785FEABCD123");
static BLEUUID openDoorUUID("00001525-1212-EFDE-1523-785FEABCD123");

BLEAdvertisedDevice *device;
BLEClient *pClient;
WiFiClient espClient;
PubSubClient client(espClient);
BLEScan *pBLEScan;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int MAX_RETIES = 5;
int attempts = 0;

bool wifiConnect()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("WiFi -- attempting connection");
    while (WiFi.status() != WL_CONNECTED && attempts < MAX_RETIES)
    {
      delay(500);
      Serial.print(".");
      attempts++;
    }
    Serial.print("connected, IP address: ");
    Serial.println(WiFi.localIP());
  }
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi -- failed to connect");
    display.println("WiFi -- failed to connect");
    display.display();
  }
  else
  {
    display.println("WiFi -- connected");
    display.display();
  }
  return (WiFi.status() == WL_CONNECTED);
}

String clientId()
{
  String clientId = APP_NAME;
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
    client.publish(DOORBELL_OPEN_DOOR_SUCCESS, EMPTY_MESSAGE);
  }
}

bool mqttConnect()
{
  attempts = 0;
  while (!client.connected() && attempts < MAX_RETIES)
  {
    client.setServer(MQTT_SERVER, MQTT_PORT).setCallback(callback);
    Serial.print("MQTT -- attempting connection...");
    if (client.connect(clientId().c_str(), MQTT_USER, MQTT_PASSWORD))
    {
      Serial.println("connected");
      display.println("MQTT -- connected");
      display.display();
      return true;
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.println(client.state());
      attempts++;
      delay(500);
    }
  }
  Serial.println("MQTT -- failed to connect");
  display.println("MQTT -- failed to connect");
  display.display();
  return false;
}

class MyClientCallback : public BLEClientCallbacks
{
  void onConnect(BLEClient *pclient)
  {
  }

  void onDisconnect(BLEClient *pclient)
  {
  }
};

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
  void onResult(BLEAdvertisedDevice advertisedDevice)
  {
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID))
    {
      device = new BLEAdvertisedDevice(advertisedDevice);
      pBLEScan->stop();
    }
  }
};

void StartScan()
{
  Serial.println("BLE -- Scanning...");
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
}

void setupDisplay()
{
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    ESP.restart();
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F(APP_NAME));
  display.println(F("---------------------\n"));
  display.display();
}

void setup()
{
  Serial.begin(9600);
  while (!Serial)
    delay(10);

  Serial.println(APP_NAME);
  Serial.println("------------------------------\n");
  setupDisplay();

  if (wifiConnect() && mqttConnect())
  {
    client.publish(DOORBELL_INACTIVE, EMPTY_MESSAGE);
    StartScan();
  }
  else
  {
    ESP.restart();
  }
}

void loop()
{
  try
  {
    if (device && pClient)
    {
      if (!pClient->isConnected())
      {
        Serial.println("Device disconnected -- Restarting");
        display.println("BLE -- disconnected");
        display.display();
        ESP.restart();
      }
      client.loop();
      return;
    }
    if (device)
    {
      Serial.println("Device found -- Connecting");
      display.println("BLE -- device found");
      display.display();
      pClient = BLEDevice::createClient();
      pClient->setClientCallbacks(new MyClientCallback());
      pClient->connect(device);
      BLERemoteService *pRemoteService = pClient->getService(serviceUUID);
      BLERemoteCharacteristic *pRemoteCharacteristic = pRemoteService->getCharacteristic(outsideButtonUUID);
      notify_callback cb = [](BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length, bool isNotify) {
        Serial.println("Doorbell button pressed");
        client.publish(DOORBELL_PRESSED, EMPTY_MESSAGE);
      };
      pRemoteCharacteristic->registerForNotify(cb);
      client.subscribe(DOORBELL_OPEN_DOOR);
      client.publish(DOORBELL_ACTIVE, EMPTY_MESSAGE);
      return;
    }
    Serial.println("Device not found -- Restarting");
    display.println("BLE -- not found");
    display.display();
    ESP.restart();
  }
  catch (int e)
  {
    display.clearDisplay();
    display.println("Error -- restarting");
    display.display();
    ESP.restart();
  }
}