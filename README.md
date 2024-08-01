# ESP-MQTT example

### Hardware 
* ESP32-C3-DevKitM-1
* On board button (GPIO 9)
* Using built-in temperature sensor


### Configuration in sdkconfig 
Insert Wifi username and passwordfile:
- CONFIG_EXAMPLE_WIFI_SSID="myssid"
- CONFIG_EXAMPLE_WIFI_PASSWORD="mypassword"

Insert MQTT broker url:
- CONFIG_BROKER_URL="mqtt://192.168.xxx.xxx:1883"

### MQTT  username and password
- user: marko 
- pw: 12345

## Mosquitto commands

### subscribe to temperature topic
```mosquitto_sub -t "temperature" -u "marko" -P "12345"```


### publish temperature request

```mosquitto_pub -t "tempRequest" -m "1" -u "marko" -P "12345"``` 

### Build and Flash

Build the project and flash it to the board, then run monitor tool to view serial output:

```
idf.py -p PORT flash monitor
```


## Expected output 
```
timer 5s, temp = 34.1

timer 5s, temp = 34.1

timer 5s, temp = 34.1

timer 5s, temp = 34.1

btn pressed, temp = 34.1

btn pressed, temp = 34.1

timer 5s, temp = 34.1

mqtt req., temp = 35.1

timer 5s, temp = 34.1

timer 5s, temp = 34.1
```
