#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "esp_err.h"

// ====================== APDS9960 ======================
#define APDS9960_ADDR 0x39

// --------- I2C 0 (Sensor 1) ----------
#define SDA_0 21
#define SCL_0 22
#define PORT_0 I2C_NUM_0

// --------- I2C 1 (Sensor 2) ----------
#define SDA_1 18
#define SCL_1 19
#define PORT_1 I2C_NUM_1

static const char *TAG = "APDS9960";

// Funções genéricas para leitura/escrita I2C
esp_err_t apds9960_write(i2c_port_t port, uint8_t reg, uint8_t value)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (APDS9960_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_write_byte(cmd, value, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(port, cmd, pdMS_TO_TICKS(500));
    i2c_cmd_link_delete(cmd);
    return ret;
}

esp_err_t apds9960_read(i2c_port_t port, uint8_t reg, uint8_t *data)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (APDS9960_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (APDS9960_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd, data, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(port, cmd, pdMS_TO_TICKS(500));
    i2c_cmd_link_delete(cmd);
    return ret;
}

void apds9960_init(i2c_port_t port)
{
    ESP_LOGI(TAG, "Inicializando APDS9960 no I2C %d...", port);
    apds9960_write(port, 0x80, 0x0F); // ENABLE
    vTaskDelay(pdMS_TO_TICKS(10));
    apds9960_write(port, 0x8F, 0x02); // Prox gain
    apds9960_write(port, 0x83, 0x20); // LED drive
    ESP_LOGI(TAG, "APDS9960 no I2C %d inicializado.", port);
}

void read_proximity(i2c_port_t port, const char *name)
{
    uint8_t prox;
    if (apds9960_read(port, 0x9C, &prox) == ESP_OK)
        ESP_LOGI(TAG, "%s → Proximidade: %d", name, prox);
    else
        ESP_LOGE(TAG, "%s → Falha ao ler proximidade", name);
}

void i2c_init_channel(i2c_port_t port, int sda, int scl)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = sda,
        .scl_io_num = scl,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000};

    i2c_param_config(port, &conf);
    i2c_driver_install(port, conf.mode, 0, 0, 0);
}

// ====================== PWM (Coolers) ======================

// Cooler 1
#define PWM1_PIN 23
#define PWM1_FREQ_HZ 25000
#define PWM1_RES LEDC_TIMER_8_BIT
#define PWM1_DUTY 155
#define PWM1_MODE LEDC_LOW_SPEED_MODE
#define PWM1_CHANNEL LEDC_CHANNEL_0
#define PWM1_TIMER LEDC_TIMER_0

// Cooler 2
#define PWM2_PIN 32
#define PWM2_FREQ_HZ 25000
#define PWM2_RES LEDC_TIMER_8_BIT
#define PWM2_DUTY 155
#define PWM2_MODE LEDC_LOW_SPEED_MODE
#define PWM2_CHANNEL LEDC_CHANNEL_1
#define PWM2_TIMER LEDC_TIMER_1

void pwm_init()
{
    printf("Inicializando PWM dos ventiladores...\n");

    // Timer 1
    ledc_timer_config_t timer1 = {
        .speed_mode = PWM1_MODE,
        .duty_resolution = PWM1_RES,
        .timer_num = PWM1_TIMER,
        .freq_hz = PWM1_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK};
    ESP_ERROR_CHECK(ledc_timer_config(&timer1));

    // Canal 1
    ledc_channel_config_t ch1 = {
        .speed_mode = PWM1_MODE,
        .channel = PWM1_CHANNEL,
        .timer_sel = PWM1_TIMER,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = PWM1_PIN,
        .duty = PWM1_DUTY,
        .hpoint = 0};
    ESP_ERROR_CHECK(ledc_channel_config(&ch1));

    // Timer 2
    ledc_timer_config_t timer2 = {
        .speed_mode = PWM2_MODE,
        .duty_resolution = PWM2_RES,
        .timer_num = PWM2_TIMER,
        .freq_hz = PWM2_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK};
    ESP_ERROR_CHECK(ledc_timer_config(&timer2));

    // Canal 2
    ledc_channel_config_t ch2 = {
        .speed_mode = PWM2_MODE,
        .channel = PWM2_CHANNEL,
        .timer_sel = PWM2_TIMER,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = PWM2_PIN,
        .duty = PWM2_DUTY,
        .hpoint = 0};
    ESP_ERROR_CHECK(ledc_channel_config(&ch2));

    // Aplicar duty
    ledc_set_duty(PWM1_MODE, PWM1_CHANNEL, PWM1_DUTY);
    ledc_update_duty(PWM1_MODE, PWM1_CHANNEL);

    ledc_set_duty(PWM2_MODE, PWM2_CHANNEL, PWM2_DUTY);
    ledc_update_duty(PWM2_MODE, PWM2_CHANNEL);
}

// ====================== APP MAIN ======================

void app_main()
{
    // Inicializa PWM
    pwm_init();

    // Inicializa I2C
    i2c_init_channel(PORT_0, SDA_0, SCL_0);
    i2c_init_channel(PORT_1, SDA_1, SCL_1);

    // Inicializa sensores APDS9960
    apds9960_init(PORT_0);
    apds9960_init(PORT_1);

    while (1)
    {
        read_proximity(PORT_0, "Sensor 1");
        read_proximity(PORT_1, "Sensor 2");
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
