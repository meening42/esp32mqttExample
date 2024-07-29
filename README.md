# ESP-MQTT example

## How to use example

### Hardware 
* ESP32-C3-DevKitM-1
* On board button
* Using built-in temperature sensor


### Configuration 
Insert Wifi username and password here:
CONFIG_EXAMPLE_WIFI_SSID="myssid"
CONFIG_EXAMPLE_WIFI_PASSWORD="mypassword"


### Build and Flash

Build the project and flash it to the board, then run monitor tool to view serial output:

```
idf.py -p PORT flash monitor
```

(To exit the serial monitor, type ``Ctrl-]``.)

See the Getting Started Guide for full steps to configure and use ESP-IDF to build projects.

## Example Output

```
I (5119) esp_netif_handlers: example_connect: sta ip: 192.168.3.143, mask: 255.255.255.0, gw: 192.168.3.1
I (5119) example_connect: Got IPv4 event: Interface "example_connect: sta" address: 192.168.3.143
I (5619) example_connect: Got IPv6 event: Interface "example_connect: sta" address: fe80:0000:0000:0000:c64f:33ff:fe24:6645, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (5619) example_connect: Connected to example_connect: sta
I (5629) example_connect: - IPv4 address: 192.168.3.143
I (5629) example_connect: - IPv6 address: fe80:0000:0000:0000:c64f:33ff:fe24:6645, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (5649) MQTT5_EXAMPLE: Other event id:7
W (6299) wifi:<ba-add>idx:0 (ifx:0, 34:29:12:43:c5:40), tid:7, ssn:0, winSize:64
I (7439) MQTT5_EXAMPLE: MQTT_EVENT_CONNECTED
I (7439) MQTT5_EXAMPLE: sent publish successful, msg_id=53118
I (7439) MQTT5_EXAMPLE: sent subscribe successful, msg_id=41391
I (7439) MQTT5_EXAMPLE: sent subscribe successful, msg_id=13695
I (7449) MQTT5_EXAMPLE: sent unsubscribe successful, msg_id=55594
I (7649) mqtt5_client: MQTT_MSG_TYPE_PUBACK return code is -1
I (7649) MQTT5_EXAMPLE: MQTT_EVENT_PUBLISHED, msg_id=53118
I (8039) mqtt5_client: MQTT_MSG_TYPE_SUBACK return code is 0
I (8049) MQTT5_EXAMPLE: MQTT_EVENT_SUBSCRIBED, msg_id=41391
I (8049) MQTT5_EXAMPLE: sent publish successful, msg_id=0
I (8059) mqtt5_client: MQTT_MSG_TYPE_SUBACK return code is 2
I (8059) MQTT5_EXAMPLE: MQTT_EVENT_SUBSCRIBED, msg_id=13695
I (8069) MQTT5_EXAMPLE: sent publish successful, msg_id=0
I (8079) MQTT5_EXAMPLE: MQTT_EVENT_DATA
I (8079) MQTT5_EXAMPLE: key is board, value is esp32
I (8079) MQTT5_EXAMPLE: key is u, value is user
I (8089) MQTT5_EXAMPLE: key is p, value is password
I (8089) MQTT5_EXAMPLE: payload_format_indicator is 1
I (8099) MQTT5_EXAMPLE: response_topic is /topic/test/response
I (8109) MQTT5_EXAMPLE: correlation_data is 123456
I (8109) MQTT5_EXAMPLE: content_type is 
I (8119) MQTT5_EXAMPLE: TOPIC=/topic/qos1
I (8119) MQTT5_EXAMPLE: DATA=data_3
I (8129) mqtt5_client: MQTT_MSG_TYPE_UNSUBACK return code is 0
I (8129) MQTT5_EXAMPLE: MQTT_EVENT_UNSUBSCRIBED, msg_id=55594
I (8139) mqtt_client: Client asked to disconnect
I (9159) MQTT5_EXAMPLE: MQTT_EVENT_DISCONNECTED
```
