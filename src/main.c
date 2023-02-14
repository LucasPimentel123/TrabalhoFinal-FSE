// #include <stdio.h>
// #include <string.h>
// #include <pthread.h>
// #include "nvs_flash.h"
// #include "esp_wifi.h"
// #include "esp_event.h"
// #include "esp_http_client.h"
// #include "esp_log.h"
// #include "freertos/semphr.h"
// #include <inttypes.h>
// #include "include/wifi.h"
// #include "include/mqtt.h"
// #include "include/dht11.h"
// #include "include/gpio.h"

// #define LED_1 2
// #define LED_2 4
// #define LED_3 16
// #define LED_4 18
// #define ENABLE_SENSOR 19
// #define SENSOR 5
// #define BOTAO 0

// void app_main()
// {
//     // Configuração dos pinos dos LEDs
//     esp_rom_gpio_pad_select_gpio(LED_1);
//     esp_rom_gpio_pad_select_gpio(LED_2);
//     esp_rom_gpio_pad_select_gpio(LED_4);
//     // esp_rom_gpio_pad_select_gpio(LED_3);
//     // esp_rom_gpio_pad_select_gpio(LED_4);
//     //  Configura os pinos dos LEDs como Output
//     gpio_set_direction(LED_1, GPIO_MODE_OUTPUT);
//     gpio_set_direction(LED_2, GPIO_MODE_OUTPUT);
//     gpio_set_direction(LED_4, GPIO_MODE_OUTPUT);
//     gpio_set_direction(ENABLE_SENSOR, GPIO_MODE_OUTPUT);
//     gpio_set_direction(SENSOR, GPIO_MODE_INPUT);
//     // gpio_set_direction(LED_3, GPIO_MODE_OUTPUT);
//     // gpio_set_direction(LED_4, GPIO_MODE_OUTPUT);

//     // Configuração do pino do Botão
//     esp_rom_gpio_pad_select_gpio(BOTAO);
//     // Configura o pino do Botão como Entrada
//     gpio_set_direction(BOTAO, GPIO_MODE_INPUT);
//     // Configura o resistor de Pulldown para o botão (por padrão a entrada estará em Zero)
//     gpio_pulldown_en(BOTAO);
//     // Desabilita o resistor de Pull-up por segurança.
//     gpio_pullup_dis(BOTAO);

//     // Teste os LEDs Externos
//     // for (int i = 0; i <= 8; i++)
//     // {
//     //     gpio_set_level(LED_2, (i & 1) >> 0);
//     //     gpio_set_level(LED_3, (i & 2) >> 1);
//     //     gpio_set_level(LED_4, (i & 4) >> 2);

//     //     vTaskDelay(1000 / portTICK_PERIOD_MS);
//     // }

//     // Testa o Botão utilizando polling
//     // while (true)
//     // {
//     //     int estado_botao = gpio_get_level(BOTAO);
//     //     gpio_set_level(LED_1, !estado_botao);
//     //     gpio_set_level(LED_2, estado_botao);
//     //     gpio_set_level(LED_4, estado_botao);
//     //
//     // }

//     gpio_set_level(ENABLE_SENSOR, 1);

//     while (true)
//     {
//         if (gpio_get_level(SENSOR) == 1)
//         { // SE A LEITURA DO PINO FOR IGUAL A HIGH, FAZ
//             gpio_set_level(LED_4, 0);
//             gpio_set_level(LED_2, 1); // ACENDE O LED
//         }
//         else
//         { // SENÃO, FAZ
//             gpio_set_level(LED_4, 1);
//             gpio_set_level(LED_2, 0);
//             // vTaskDelay(10000 / portTICK_PERIOD_MS); // APAGA O LED
//         }
//         vTaskDelay(100 / portTICK_PERIOD_MS);

//         mqtt_start();
//     }
// }

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
    // char JsonAtributos[50];
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
    // xTaskCreate(&verificaLed, "Verifica estado led", 4096, NULL, 1, NULL);
}
