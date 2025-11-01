#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "esp_err.h"

// Canal 1 - GPIO 25
#define PWM1_PIN 25
#define PWM1_FREQ_HZ 25000
#define PWM1_RESOLUTION LEDC_TIMER_8_BIT
#define PWM1_DUTY 255
#define PWM1_MODE LEDC_LOW_SPEED_MODE
#define PWM1_CHANNEL LEDC_CHANNEL_0
#define PWM1_TIMER LEDC_TIMER_0

// Canal 2 - GPIO 27
#define PWM2_PIN 27
#define PWM2_FREQ_HZ 25000
#define PWM2_RESOLUTION LEDC_TIMER_8_BIT
#define PWM2_DUTY 255
#define PWM2_MODE LEDC_LOW_SPEED_MODE
#define PWM2_CHANNEL LEDC_CHANNEL_1
#define PWM2_TIMER LEDC_TIMER_1

void app_main(void)
{
    printf("Iniciando controle fixo de ventiladores PWM...\n");

    // Configuração do temporizador do canal 1
    ledc_timer_config_t ledc_timer1 = {
        .speed_mode = PWM1_MODE,
        .duty_resolution = PWM1_RESOLUTION,
        .timer_num = PWM1_TIMER,
        .freq_hz = PWM1_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK};
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer1));

    // Configuração do canal PWM 1
    ledc_channel_config_t ledc_channel1 = {
        .speed_mode = PWM1_MODE,
        .channel = PWM1_CHANNEL,
        .timer_sel = PWM1_TIMER,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = PWM1_PIN,
        .duty = PWM1_DUTY,
        .hpoint = 0};
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel1));

    // Configuração do temporizador do canal 2
    ledc_timer_config_t ledc_timer2 = {
        .speed_mode = PWM2_MODE,
        .duty_resolution = PWM2_RESOLUTION,
        .timer_num = PWM2_TIMER,
        .freq_hz = PWM2_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK};
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer2));

    // Configuração do canal PWM 2
    ledc_channel_config_t ledc_channel2 = {
        .speed_mode = PWM2_MODE,
        .channel = PWM2_CHANNEL,
        .timer_sel = PWM2_TIMER,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = PWM2_PIN,
        .duty = PWM2_DUTY,
        .hpoint = 0};
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel2));

    // Aplicar duty cycle fixos
    ESP_ERROR_CHECK(ledc_set_duty(PWM1_MODE, PWM1_CHANNEL, PWM1_DUTY));
    ESP_ERROR_CHECK(ledc_update_duty(PWM1_MODE, PWM1_CHANNEL));

    ESP_ERROR_CHECK(ledc_set_duty(PWM2_MODE, PWM2_CHANNEL, PWM2_DUTY));
    ESP_ERROR_CHECK(ledc_update_duty(PWM2_MODE, PWM2_CHANNEL));

    // Loop infinito
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
