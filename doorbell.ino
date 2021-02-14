#include <WiFi.h>
#include <HTTPClient.h>
#include "config.h"

int buttonState = 0;

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

  pinMode(PIN_4, INPUT_PULLDOWN);
}

void loop()
{
  buttonState = digitalRead(PIN_4);
  if (buttonState == HIGH)
  {
    Serial.println("Doorbell ON");
  }
}