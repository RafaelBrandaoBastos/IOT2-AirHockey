#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "esp_log.h"

#define I2C_MASTER_SCL_IO 22
#define I2C_MASTER_SDA_IO 21
#define I2C_MASTER_PORT I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 100000
#define APDS9960_ADDR 0x39

static const char *TAG = "APDS9960";

// Função para escrever 1 byte em um registrador
esp_err_t apds9960_write(uint8_t reg, uint8_t value)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (APDS9960_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_write_byte(cmd, value, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_PORT, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    return ret;
}

// Função para ler 1 byte de um registrador
esp_err_t apds9960_read(uint8_t reg, uint8_t *data)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (APDS9960_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (APDS9960_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd, data, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_PORT, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    return ret;
}

void apds9960_init()
{
    ESP_LOGI(TAG, "Inicializando APDS9960...");

    // Habilita o sensor (PON + AEN + PEN)
    apds9960_write(0x80, 0x0F); // ENABLE register
    vTaskDelay(pdMS_TO_TICKS(10));

    // Define ganho de proximidade
    apds9960_write(0x8F, 0x02); // Proximity Gain = 4x

    // Define LED drive
    apds9960_write(0x83, 0x20); // LED Drive = 100 mA

    ESP_LOGI(TAG, "APDS9960 inicializado.");
}

void apds9960_read_proximity()
{
    uint8_t prox;
    esp_err_t ret = apds9960_read(0x9C, &prox);
    if (ret == ESP_OK)
    {
        ESP_LOGI(TAG, "Proximidade: %d", prox);
    }
    else
    {
        ESP_LOGE(TAG, "Falha na leitura de proximidade");
    }
}

void app_main(void)
{
    // Configura I2C
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    i2c_param_config(I2C_MASTER_PORT, &conf);
    i2c_driver_install(I2C_MASTER_PORT, conf.mode, 0, 0, 0);

    apds9960_init();

    while (1)
    {
        apds9960_read_proximity();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
