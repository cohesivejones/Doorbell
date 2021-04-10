#include <BLEDevice.h>
#include <BLEAdvertisedDevice.h>
#include <WiFi.h>
#include "config.h"

static BLEUUID serviceUUID("00001523-1212-EFDE-1523-785FEABCD123");
static BLEUUID charUUID("00001525-1212-EFDE-1523-785FEABCD123");
BLERemoteCharacteristic *pCharacteristic;

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

class MyClientCallback : public BLEClientCallbacks
{
  void onConnect(BLEClient *pclient)
  {
    BLERemoteService *pService = pclient->getService(serviceUUID);
    pCharacteristic = pService->getCharacteristic(charUUID);
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
      BLEDevice::getScan()->stop();

      BLEClient *pClient = BLEDevice::createClient();
      pClient->setClientCallbacks(new MyClientCallback());
      pClient->connect(&advertisedDevice);
    }
  }
};

void setup()
{
  Serial.begin(9600);
  while (!Serial)
    ;

  wifiConnect();

  BLEDevice::init("");
  BLEScan *pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
}

void loop()
{
}