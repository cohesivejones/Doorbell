# Doorbell
This project is for using Adafruit Feather ESP32 board to interface with a standard 3 channel doorbell/intercom making the doorbell an IoT device 


You will need to add a config.h header file to get things going:

```C
/* config.h */

#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

#define MQTT_SERVER "YOUR_MQQT_HOST"     /* Ex. "driver.cloudmqtt.com" */
#define MQTT_PORT YOUR_MQQT_PORT         /* Ex. 18846 */
#define MQTT_USER "YOUR_MQQT_USERNAME"
#define MQTT_PASSWORD "YOUR_MQQT_PASSWORD"
```

