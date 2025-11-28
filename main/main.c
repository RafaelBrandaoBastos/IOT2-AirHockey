#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define BOTAO_GPIO 13
#define TAG "BOTAO"

void app_main(void)
{
    // Configura o GPIO como entrada com pull-up
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BOTAO_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};
    gpio_config(&io_conf);

    ESP_LOGI(TAG, "Iniciado. Pressione o botão...");

    while (1)
    {
        int estado = gpio_get_level(BOTAO_GPIO);

        if (estado == 0)
        { // Botão apertado (ligado ao GND)
            ESP_LOGI(TAG, "Botão APERTADO");
        }
        else
        {
            ESP_LOGI(TAG, "Botão solto");
        }

        vTaskDelay(pdMS_TO_TICKS(300));
    }
}
