# Doorbell Bridge
This project is hosted on a [Adafruit Feather ESP32](https://www.adafruit.com/product/3405) board. The executing code behaves like a WiFi/BLE brige interface between an MQTT server hosted in the cloud and a standard 4 channel doorbell/intercom allowing the doorbell to act as an IoT device. 

The doorbell circuit is connected to a BLE server hosted on a [Adafruit Feather nRF52 Bluefruit](https://www.adafruit.com/product/3406). The code for the server can be found [here](https://github.com/cohesivejones/doorbell-server).

You will need to add a config.h header file to get things going:

```C
/* config.h */

#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

#define MQTT_SERVER "YOUR_MQQT_HOST"     /* Ex. "driver.cloudmqtt.com" */
#define MQTT_PORT YOUR_MQQT_PORT         /* Ex. 18846 */
#define MQTT_USER "YOUR_MQQT_USERNAME"
#define MQTT_PASSWORD "YOUR_MQQT_PASSWORD"

/* List of MQTT topics accepted by server */
#define DOORBELL_INACTIVE "doorbell/inactive"
#define DOORBELL_ACTIVE "doorbell/active"
#define DOORBELL_BUZZER "doorbell/buzzer"
#define DOORBELL_PRESSED "doorbell/pressed"
#define DOORBELL_BATTERY "doorbell/battery"
#define EMPTY_MESSAGE "test
```

