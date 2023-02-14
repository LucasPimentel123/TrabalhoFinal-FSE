#include <stdio.h>
#include <string.h>
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "include/gpio.h"

#include "include/wifi.h"
#include "include/mqtt.h"
#include "include/gpio.h"

SemaphoreHandle_t conexaoWifiSemaphore;
SemaphoreHandle_t conexaoMQTTSemaphore;
SemaphoreHandle_t ledSemaphore;

#define LED_4 18
#define LED_2 16
#define TOUCH_SENSOR 22
#define SOUND_SENSOR 23

void conectadoWifi(void *params)
{
    while (true)
    {
        if (xSemaphoreTake(conexaoWifiSemaphore, portMAX_DELAY))
        {
            // Processamento Internet
            mqtt_start();
        }
    }
}

void trataComunicacaoComServidor(void *params)
{
    if (xSemaphoreTake(conexaoMQTTSemaphore, portMAX_DELAY))
    {

        verificaTemperatura();
    }
}

void app_main(void)
{
    // Inicializa o NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    esp_rom_gpio_pad_select_gpio(LED_4);
    gpio_set_direction(LED_4, GPIO_MODE_OUTPUT);

    esp_rom_gpio_pad_select_gpio(LED_2);
    gpio_set_direction(LED_2, GPIO_MODE_OUTPUT);

    esp_rom_gpio_pad_select_gpio(TOUCH_SENSOR);
    gpio_set_direction(TOUCH_SENSOR, GPIO_MODE_INPUT);

    esp_rom_gpio_pad_select_gpio(SOUND_SENSOR);
    gpio_set_direction(SOUND_SENSOR, GPIO_MODE_INPUT);

    conexaoWifiSemaphore = xSemaphoreCreateBinary();
    conexaoMQTTSemaphore = xSemaphoreCreateBinary();
    wifi_start();

    xTaskCreate(&conectadoWifi, "Conexão ao MQTT", 4096, NULL, 1, NULL);
    xTaskCreate(&trataComunicacaoComServidor, "Comunicação com Broker", 4096, NULL, 1, NULL);
}
