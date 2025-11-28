#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "esp_log.h"

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

// Função genérica para escrever
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

// Função genérica para ler
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

void app_main()
{
    // Inicializa canais I2C
    i2c_init_channel(PORT_0, SDA_0, SCL_0);
    i2c_init_channel(PORT_1, SDA_1, SCL_1);
    // Inicializa ambos sensores
    apds9960_init(PORT_0);
    apds9960_init(PORT_1);
    while (1)
    {
        read_proximity(PORT_0, "Sensor 1");
        read_proximity(PORT_1, "Sensor 2");
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
