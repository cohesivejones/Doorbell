#include <WiFi.h>
#include <HTTPClient.h>
#include "config.h"

String serverName = "http://192.168.86.30:8080/visit";
unsigned long lastTime = 0;
unsigned long timerDelay = 5000;

void connectToWifi()
{
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("Connected, IP address: ");
  Serial.println(WiFi.localIP());
}

void setup()
{
  Serial.begin(9600);
  while (!Serial)
    ;
  connectToWifi();
}

void loop()
{
  if ((millis() - lastTime) > timerDelay)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      HTTPClient http;

      http.begin(serverName.c_str());
      int httpResponseCode = http.GET();
      if (httpResponseCode > 0)
      {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
      }
      else
      {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      http.end();
    }
    else
    {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}