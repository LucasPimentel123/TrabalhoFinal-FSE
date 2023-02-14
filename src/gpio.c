#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "include/gpio.h"
#include <dht11.h>
#include "include/mqtt.h"

#define PROXY_SENSOR 5
#define TEMP_SENSOR 21
#define ENABLE_SENSOR 19
#define TAG "MQTT"
#define LED_4 18

int estadoSensor = 0;

void setProxSensor()
{

    while (true)
    {
        if (gpio_get_level(PROXY_SENSOR) == 1)
        { // SE A LEITURA DO PINO FOR IGUAL A HIGH, FAZ
            gpio_set_level(LED_4, 0);
            // ACENDE O LED
        }
        else
        { // SENÃO, FAZ
            gpio_set_level(LED_4, 1);

            // vTaskDelay(10000 / portTICK_PERIOD_MS); // APAGA O LED
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void verificaTemperatura()
{
    // esp_rom_gpio_pad_select_gpio(TEMP_SENSOR);
    // gpio_set_direction(TEMP_SENSOR, GPIO_MODE_INPUT);
    // DHT11_init(TEMP_SENSOR);

    char mensagem[50];
    char humidade_msg[50];
    char JsonAtributos[200];
    while (true)
    {
        float temperatura = 20.0 + (float)rand() / (float)(RAND_MAX / 10.0);
        sprintf(mensagem, "{\"temperatura1\": %f}", temperatura);
        mqtt_envia_mensagem("v1/devices/me/telemetry", mensagem);

        float humidade = 20.0 + (float)rand() / (float)(RAND_MAX / 80.0);
        sprintf(humidade_msg, "{\"humidade\": %f}", humidade);
        mqtt_envia_mensagem("v1/devices/me/telemetry", humidade_msg);

        
        // sprintf(mensagem, "{\"temperatura1\": %d}", DHT11_read().temperature);
        // mqtt_envia_mensagem("v1/devices/me/telemetry", mensagem);

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    // printf("Temperature is %d \n", DHT11_read().temperature);
    // printf("Humidity is %d\n", DHT11_read().humidity);
    // printf("Status code is %d\n", DHT11_read().status);
}
