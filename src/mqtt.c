#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_system.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include <inttypes.h>
#include "esp_log.h"
#include "mqtt_client.h"

#include "include/mqtt.h"
#include "include/gpio.h"
#include "include/cJSON.h"

#define TAG "MQTT"

extern SemaphoreHandle_t conexaoMQTTSemaphore;
esp_mqtt_client_handle_t client;
int sensorProxyLigado = 0;
int sensorToqueLigado = 0;
int sensorSomLigado = 0;
TaskHandle_t tidSensor;
TaskHandle_t tidSensorToque;
TaskHandle_t tidSensorSom;

void mqtt_recebe_mensagem(char *payload)
{
    cJSON *root = cJSON_Parse(payload);
    char *method = cJSON_GetObjectItem(root, "method")->valuestring;

    printf("Metodo: %s\r\n", method);

    if (strcmp(method, "ligarSensorProx") == 0)
    {

        if (!sensorProxyLigado)
        {

            tidSensor = xTaskCreate(&setProxSensor, "Ligar Sensor Proximidade", 4096, NULL, 1, NULL);
            sensorProxyLigado = 1;
        }
        else
        {
            sensorProxyLigado = 0;
            char mensagemledDesliga[50];
            sprintf(mensagemledDesliga, "{\"ledProxy\": %d}", 0);
            mqtt_envia_mensagem("v1/devices/me/attributes", mensagemledDesliga);
            vTaskDelay(100 / portTICK_PERIOD_MS);
            vTaskDelete(tidSensor);
        }
    }
    else if (strcmp(method, "ligarSensorToque") == 0)
    {
        if (!sensorToqueLigado)
        {
            printf("%s", method);
            tidSensorToque = xTaskCreate(&touchSensor, "Ligar Sensor Toque", 4096, NULL, 1, NULL);
            sensorToqueLigado = 1;
        }
        else
        {
            sensorToqueLigado = 0;
            char mensagemledToqueDesliga[50];
            sprintf(mensagemledToqueDesliga, "{\"ledTouch\": %d}", 0);
            mqtt_envia_mensagem("v1/devices/me/attributes", mensagemledToqueDesliga);
            vTaskDelay(100 / portTICK_PERIOD_MS);
            vTaskDelete(tidSensorToque);
        }
    }

    else if (strcmp(method, "ligarSensorSom") == 0)
    {
        if (!sensorSomLigado)
        {
            printf("%s", method);
            tidSensorSom = xTaskCreate(&setSoundSensor, "Ligar Sensor Som", 4096, NULL, 1, NULL);
            sensorSomLigado = 1;
        }
        else
        {
            sensorSomLigado = 0;
            char mensagemledSomDesliga[50];
            sprintf(mensagemledSomDesliga, "{\"ledSom\": %d}", 0);
            mqtt_envia_mensagem("v1/devices/me/attributes", mensagemledSomDesliga);
            vTaskDelay(100 / portTICK_PERIOD_MS);
            vTaskDelete(tidSensorSom);
        }
    }
}

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0)
    {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, (int)event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        xSemaphoreGive(conexaoMQTTSemaphore);
        msg_id = esp_mqtt_client_subscribe(client, "v1/devices/me/rpc/request/+", 0);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);

        mqtt_recebe_mensagem(event->data);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
        {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno", event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

void mqtt_start()
{
    esp_mqtt_client_config_t mqtt_config = {
        .broker.address.uri = "mqtt://164.41.98.25",
        .credentials.username = "XZkk5qkWryi5DZ2iEaDL"};
    client = esp_mqtt_client_init(&mqtt_config);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}

void mqtt_envia_mensagem(char *topico, char *mensagem)
{
    int message_id = esp_mqtt_client_publish(client, topico, mensagem, 0, 1, 0);
    ESP_LOGI(TAG, "Mesnagem enviada, ID: %d", message_id);
}
