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
#define TOUCH_SENSOR 22
#define LED_2 16
#define SOUND_SENSOR 23

int estadoSensor = 0;
int estadoLed = 0;

void setProxSensor()
{

    char mensagemled[50];
    sprintf(mensagemled, "{\"ledProxy\": %d}", 1);
    mqtt_envia_mensagem("v1/devices/me/attributes", mensagemled);
    vTaskDelay(100 / portTICK_PERIOD_MS);
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

void touchSensor()
{
    char mensagemledToque[50];
    sprintf(mensagemledToque, "{\"ledTouch\": %d}", 1);
    mqtt_envia_mensagem("v1/devices/me/attributes", mensagemledToque);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    while (true)
    {
        if (gpio_get_level(TOUCH_SENSOR) == 1)
        {
            gpio_set_level(LED_4, 1);
        }
        else
        {
            gpio_set_level(LED_4, 0);
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void setSoundSensor()
{
    char mensagemledSom[50];
    sprintf(mensagemledSom, "{\"ledSom\": %d}", 1);
    mqtt_envia_mensagem("v1/devices/me/attributes", mensagemledSom);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    while (true)
    {
        if (gpio_get_level(SOUND_SENSOR) == 1)
        {
            gpio_set_level(LED_4, 1);
        }
        else
        {
            gpio_set_level(LED_4, 0);
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void verificaTemperatura()
{
    esp_rom_gpio_pad_select_gpio(TEMP_SENSOR);
    gpio_set_direction(TEMP_SENSOR, GPIO_MODE_INPUT);
    DHT11_init(TEMP_SENSOR);

    char mensagem[50];
    char humidade_msg[50];

    while (true)
    {

        sprintf(mensagem, "{\"temperatura1\": %d}", DHT11_read().temperature);
        mqtt_envia_mensagem("v1/devices/me/telemetry", mensagem);

        sprintf(humidade_msg, "{\"humidade\": %d}", DHT11_read().humidity);
        mqtt_envia_mensagem("v1/devices/me/telemetry", humidade_msg);

        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}

void setLed()
{
    if (estadoLed == 0)
    {
        estadoLed = 1;
    }
    else
    {
        estadoLed = 0;
    }
    gpio_set_level(LED_2, estadoLed);
    vTaskDelay(100 / portTICK_PERIOD_MS);

    char mensagemSetLed[50];
    sprintf(mensagemSetLed, "{\"LedDash\": %d}", estadoLed);
    mqtt_envia_mensagem("v1/devices/me/attributes", mensagemSetLed);
    vTaskDelay(100 / portTICK_PERIOD_MS);
}
