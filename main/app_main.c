/*
 * SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include <inttypes.h>
#include "driver/gpio.h"
#include "driver/temperature_sensor.h"

#define ESP_INTR_FLAG_DEFAULT 0
#define GPIO_INPUT_IO_0     9
#define GPIO_INPUT_PIN_SEL  (1ULL<<GPIO_INPUT_IO_0) 

temperature_sensor_handle_t temp_sensor = NULL;


typedef enum  {
  TRIGGER_BTN,
  TRIGGER_TIMER,
  TRIGGER_MQTT_REQUEST
}triggerSrc_t;

static QueueHandle_t evt_queue = NULL;

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    triggerSrc_t trigger = TRIGGER_BTN;
    xQueueSendFromISR(evt_queue, &trigger, NULL);
}


static const char *TAG = "mqtt5_example";

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

static esp_mqtt5_user_property_item_t user_property_arr[] = {
        {"board", "esp32"},
        {"u", "user"},
        {"p", "password"}
    };

#define USE_PROPERTY_ARR_SIZE   sizeof(user_property_arr)/sizeof(esp_mqtt5_user_property_item_t)

static esp_mqtt5_publish_property_config_t publish_property = {
    .payload_format_indicator = 1,
    .message_expiry_interval = 1000,
    .topic_alias = 0,
    .response_topic = "/topic/test/response",
    .correlation_data = "123456",
    .correlation_data_len = 6,
};

static esp_mqtt5_subscribe_property_config_t subscribe_property = {
    .subscribe_id = 25555,
    .no_local_flag = false,
    .retain_as_published_flag = false,
    .retain_handle = 0,
    .is_share_subscribe = true,
    .share_name = "group1",
};

static esp_mqtt5_subscribe_property_config_t subscribe1_property = {
    .subscribe_id = 25555,
    .no_local_flag = true,
    .retain_as_published_flag = false,
    .retain_handle = 0,
};

static esp_mqtt5_unsubscribe_property_config_t unsubscribe_property = {
    .is_share_subscribe = true,
    .share_name = "group1",
};

static esp_mqtt5_disconnect_property_config_t disconnect_property = {
    .session_expiry_interval = 60,
    .disconnect_reason = 0,
};

static void print_user_property(mqtt5_user_property_handle_t user_property)
{
    if (user_property) {
        uint8_t count = esp_mqtt5_client_get_user_property_count(user_property);
        if (count) {
            esp_mqtt5_user_property_item_t *item = malloc(count * sizeof(esp_mqtt5_user_property_item_t));
            if (esp_mqtt5_client_get_user_property(user_property, item, &count) == ESP_OK) {
                for (int i = 0; i < count; i ++) {
                    esp_mqtt5_user_property_item_t *t = &item[i];
                    ESP_LOGI(TAG, "key is %s, value is %s", t->key, t->value);
                    free((char *)t->key);
                    free((char *)t->value);
                }
            }
            free(item);
        }
    }
}

 esp_mqtt_client_handle_t client;


TimerHandle_t tmr;
int id = 1;
int interval = 5000;
void ping( TimerHandle_t xTimer )
{
    triggerSrc_t trigger = TRIGGER_TIMER;
    xQueueSendFromISR(evt_queue, &trigger, NULL);
}


/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void mqtt5_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32, base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;

    ESP_LOGD(TAG, "free heap size is %" PRIu32 ", minimum %" PRIu32, esp_get_free_heap_size(), esp_get_minimum_free_heap_size());
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        print_user_property(event->property->user_property);
        esp_mqtt5_client_set_user_property(&publish_property.user_property, user_property_arr, USE_PROPERTY_ARR_SIZE);
        esp_mqtt5_client_set_publish_property(client, &publish_property);
        msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data_3", 0, 1, 1);
        esp_mqtt5_client_delete_user_property(publish_property.user_property);
        publish_property.user_property = NULL;
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

        esp_mqtt5_client_set_user_property(&subscribe_property.user_property, user_property_arr, USE_PROPERTY_ARR_SIZE);
        esp_mqtt5_client_set_subscribe_property(client, &subscribe_property);
        msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
        esp_mqtt5_client_delete_user_property(subscribe_property.user_property);
        subscribe_property.user_property = NULL;
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        esp_mqtt5_client_set_user_property(&subscribe1_property.user_property, user_property_arr, USE_PROPERTY_ARR_SIZE);
        esp_mqtt5_client_set_subscribe_property(client, &subscribe1_property);
        msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 2);
        esp_mqtt5_client_delete_user_property(subscribe1_property.user_property);
        subscribe1_property.user_property = NULL;
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        esp_mqtt5_client_set_user_property(&unsubscribe_property.user_property, user_property_arr, USE_PROPERTY_ARR_SIZE);
        esp_mqtt5_client_set_unsubscribe_property(client, &unsubscribe_property);
        msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos0");
        ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
        esp_mqtt5_client_delete_user_property(unsubscribe_property.user_property);
        unsubscribe_property.user_property = NULL;
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        print_user_property(event->property->user_property);
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        print_user_property(event->property->user_property);
        esp_mqtt5_client_set_publish_property(client, &publish_property);
        msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        print_user_property(event->property->user_property);
        esp_mqtt5_client_set_user_property(&disconnect_property.user_property, user_property_arr, USE_PROPERTY_ARR_SIZE);
        esp_mqtt5_client_set_disconnect_property(client, &disconnect_property);
        esp_mqtt5_client_delete_user_property(disconnect_property.user_property);
        disconnect_property.user_property = NULL;
        //esp_mqtt_client_disconnect(client);
        break;
    case MQTT_EVENT_PUBLISHED:
        //ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        //print_user_property(event->property->user_property);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        print_user_property(event->property->user_property);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        print_user_property(event->property->user_property);
        ESP_LOGI(TAG, "MQTT5 return code is %d", event->error_handle->connect_return_code);
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

static void mqtt5_app_start(void)
{
    esp_mqtt5_connection_property_config_t connect_property = {
        .session_expiry_interval = 10,
        .maximum_packet_size = 1024,
        .receive_maximum = 65535,
        .topic_alias_maximum = 2,
        .request_resp_info = true,
        .request_problem_info = true,
        .will_delay_interval = 10,
        .payload_format_indicator = true,
        .message_expiry_interval = 10,
        .response_topic = "/test/response",
        .correlation_data = "123456",
        .correlation_data_len = 6,
    };

    esp_mqtt_client_config_t mqtt5_cfg = {
        .broker.address.uri = CONFIG_BROKER_URL,
        .session.protocol_ver = MQTT_PROTOCOL_V_5,
        .network.disable_auto_reconnect = true,
        .credentials.username = "mojca",
        .credentials.authentication.password = "123",
        .session.last_will.topic = "/topic/will",
        .session.last_will.msg = "i will leave",
        .session.last_will.msg_len = 12,
        .session.last_will.qos = 1,
        .session.last_will.retain = true,
    };

#if CONFIG_BROKER_URL_FROM_STDIN
    char line[128];

    if (strcmp(mqtt5_cfg.uri, "FROM_STDIN") == 0) {
        int count = 0;
        printf("Please enter url of mqtt broker\n");
        while (count < 128) {
            int c = fgetc(stdin);
            if (c == '\n') {
                line[count] = '\0';
                break;
            } else if (c > 0 && c < 127) {
                line[count] = c;
                ++count;
            }
            vTaskDelay(10 / portTICK_PERIOD_MS);
        }
        mqtt5_cfg.broker.address.uri = line;
        printf("Broker url: %s\n", line);
    } else {
        ESP_LOGE(TAG, "Configuration mismatch: wrong broker url");
        abort();
    }
#endif /* CONFIG_BROKER_URL_FROM_STDIN */

    client = esp_mqtt_client_init(&mqtt5_cfg);

    /* Set connection properties and user properties */
    esp_mqtt5_client_set_user_property(&connect_property.user_property, user_property_arr, USE_PROPERTY_ARR_SIZE);
    esp_mqtt5_client_set_user_property(&connect_property.will_user_property, user_property_arr, USE_PROPERTY_ARR_SIZE);
    esp_mqtt5_client_set_connect_property(client, &connect_property);

    /* If you call esp_mqtt5_client_set_user_property to set user properties, DO NOT forget to delete them.
     * esp_mqtt5_client_set_connect_property will malloc buffer to store the user_property and you can delete it after
     */
    esp_mqtt5_client_delete_user_property(connect_property.user_property);
    esp_mqtt5_client_delete_user_property(connect_property.will_user_property);

    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt5_event_handler, NULL);
    esp_mqtt_client_start(client);

    esp_mqtt_client_subscribe(client, "tempRequest", 0);
}



void  initBtnGpio(){
   //zero-initialize the config structure.
    gpio_config_t io_conf = {};
    //interrupt of rising edge
    io_conf.intr_type = GPIO_INTR_POSEDGE;
    //bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    //change gpio interrupt type for one pin
    gpio_set_intr_type(GPIO_INPUT_IO_0, GPIO_INTR_POSEDGE);
}

static void myTask(void* arg)
{
    uint32_t prevTick=0;
    triggerSrc_t src;
    float tsens_value;
    temperature_sensor_config_t temp_sensor_config = TEMPERATURE_SENSOR_CONFIG_DEFAULT(10, 50);
    ESP_ERROR_CHECK(temperature_sensor_install(&temp_sensor_config, &temp_sensor));
    ESP_LOGI(TAG, "Enable temperature sensor");
    ESP_ERROR_CHECK(temperature_sensor_enable(temp_sensor));
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "My task started\n");
    printf("%s",buffer);
    for (;;) {
        if (xQueueReceive(evt_queue, &src, portMAX_DELAY)) {
            temperature_sensor_get_celsius(temp_sensor, &tsens_value);
            switch(src){
                case TRIGGER_BTN:                   
                    snprintf(buffer, sizeof(buffer), "btn pressed, temp = %.1f\n", tsens_value);
                    break;
                case TRIGGER_TIMER:
                    snprintf(buffer, sizeof(buffer), "timer 5s, temp = %.1f\n", tsens_value);
                    break;
                case TRIGGER_MQTT_REQUEST:   
                    snprintf(buffer, sizeof(buffer), "mqtt req., temp = %.1f\n", tsens_value);
                    break;
                default:
                    snprintf(buffer, sizeof(buffer), "unknown trigger, temp = %.1f\n", tsens_value);
                    break;
            }
            printf("%s",buffer);
            esp_mqtt_client_publish(client, "temperature", buffer, 0, 1, 1);
        }
    }
}

void app_main(void)
{
 
    // initialize Btn input
    initBtnGpio();

    //create a queue to handle gpio event from isr
    evt_queue = xQueueCreate(10, sizeof(triggerSrc_t));
    //start task
    xTaskCreate(myTask, "myTask", 2048, NULL, 10, NULL);

        //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_INPUT_IO_0, gpio_isr_handler, (void*) GPIO_INPUT_IO_0);

    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    mqtt5_app_start();

    tmr = xTimerCreate("MyTimer", pdMS_TO_TICKS(interval), pdTRUE, ( void * )id, &ping);
    if( xTimerStart(tmr, 10 ) != pdPASS ) {
     printf("Timer start error");
    }
}



