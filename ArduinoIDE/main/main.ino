#include <Wire.h>
#include <APDS9930.h>
#include "display_utils.h"

#include <WiFi.h>
#include <time.h>

// CONFIG WIFI
const char *ssid = "motorafael";
const char *password = "1234567890";
// CONFIG NTP
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -3 * 3600; // UTC-3 Brasil
const int daylightOffset_sec = 0;     // Sem horário de verão

// Sensores
APDS9930 sensor1;
APDS9930 sensor2;
TwoWire I2C_1 = TwoWire(0);
TwoWire I2C_2 = TwoWire(1);

// Ventiladores
const int FAN1_PIN = 23;
const int FAN2_PIN = 32;
const int PWM_FREQ = 25000;
const int PWM_RESOLUTION = 8;
const int FAN_SPEED = 153;

// Placar
int score1 = 0;
int score2 = 0;

// Configuração gol
const uint16_t GOL_THRESHOLD = 900;
const uint16_t GOL_HOLD_TIME = 1000;

// Controle de tempo por sensor
unsigned long startHigh1 = 0;
unsigned long startHigh2 = 0;
bool waitingRecovery1 = false;
bool waitingRecovery2 = false;

// Botão reset
const int BUTTON_PIN = 13;
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50; // ms

// -------------------------------------------------------------------
void setup()
{
    Serial.begin(115200);

    Serial.println("Dual Fan Controller Starting...");

    ledcAttach(FAN1_PIN, PWM_FREQ, PWM_RESOLUTION);
    ledcAttach(FAN2_PIN, PWM_FREQ, PWM_RESOLUTION);
    ledcWrite(FAN1_PIN, FAN_SPEED);
    ledcWrite(FAN2_PIN, FAN_SPEED);

    pinMode(BUTTON_PIN, INPUT_PULLUP); // botão com pull-up

    // --------- CONECTAR WIFI ---------
    WiFi.begin(ssid, password);
    Serial.print("Conectando ao WiFi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(500);
    }
    Serial.println("\nWiFi conectado!");
    // --------- SINCRONIZAR TEMPO NTP ---------
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    // Esperar sincronização
    struct tm timeinfo;
    while (!getLocalTime(&timeinfo))
    {
        Serial.println("Aguardando tempo via NTP...");
        delay(500);
    }
    Serial.println("Hora sincronizada!");

    // Sensor 1
    I2C_1.begin(18, 19);
    Wire = I2C_1;
    if (sensor1.init())
        sensor1.enableProximitySensor(false);

    // Sensor 2
    I2C_2.begin(21, 22);
    Wire = I2C_2;
    if (sensor2.init())
        sensor2.enableProximitySensor(false);

    // Display
    initDisplays();
    showPlacar();

    // =========================
    // INICIAR PARTIDA AUTOMÁTICA
    // =========================
    startNewMatch();
}

// -------------------------------------------------------------------
void startNewMatch()
{
    score1 = 0;
    score2 = 0;
    showPlacar();

    struct tm timeinfo;
    if (getLocalTime(&timeinfo))
    {
        Serial.print("Nova partida iniciada com o token: ");
        Serial.printf("%02d/%02d/%04d %02d:%02d\n",
                      timeinfo.tm_mday,
                      timeinfo.tm_mon + 1,
                      timeinfo.tm_year + 1900,
                      timeinfo.tm_hour,
                      timeinfo.tm_min);
    }
    else
    {
        Serial.println("Erro ao obter hora! Token sem data/hora.");
    }
}

// -------------------------------------------------------------------
void showPlacar()
{
    int value = (score1 * 100) + score2;
    showNumberOnDisplays(value);
}

// -------------------------------------------------------------------
void processandoGol(APDS9930 &sensor, TwoWire &i2c, uint16_t &prox)
{
    bool estado = false;
    bool requisicaoExecutada = false;

    while (prox > GOL_THRESHOLD)
    {
        if (estado)
            showTextOnDisplays("----");
        else
            showTextOnDisplays(" ");

        estado = !estado;
        delay(200);

        if (!requisicaoExecutada)
        {
            requisicaoExecutada = true;
            // ==========================
            // requisição aqui (UMA VEZ)
            // ==========================
        }

        Wire = i2c;
        sensor.readProximity(prox);
    }

    showPlacar();
}

// -------------------------------------------------------------------
void loop()
{
    // ======================
    // Botão de reset
    // ======================
    int reading = digitalRead(BUTTON_PIN);

    if (reading != lastButtonState)
    {
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > debounceDelay)
    {
        if (reading == LOW)
        { // botão pressionado
            while (reading == LOW)
            {
                showTextOnDisplays("----");
                reading = digitalRead(BUTTON_PIN);
                Serial.println("Esperando soltar botão");
                delay(100);
                showTextOnDisplays(" ");
                delay(100);
            }
            startNewMatch(); // reinicia a partida
        }
    }

    lastButtonState = reading;

    // ======================
    // Leitura sensores
    // ======================
    uint16_t prox1 = 0, prox2 = 0;
    unsigned long now = millis();

    Wire = I2C_1;
    sensor1.readProximity(prox1);

    Wire = I2C_2;
    sensor2.readProximity(prox2);

    // ---------------------
    // Sensor 1 → Gol Time 1
    // ---------------------
    if (!waitingRecovery1)
    {
        if (prox1 > GOL_THRESHOLD)
        {
            if (startHigh1 == 0)
                startHigh1 = now;

            if ((now - startHigh1) >= GOL_HOLD_TIME)
            {
                processandoGol(sensor1, I2C_1, prox1);
                score1++;
                waitingRecovery1 = true;
                startHigh1 = 0;
                Serial.println("⚽ GOL TIME 1!");
                showPlacar();
            }
        }
        else
        {
            startHigh1 = 0;
        }
    }
    else if (prox1 < GOL_THRESHOLD)
    {
        waitingRecovery1 = false;
    }

    // ---------------------
    // Sensor 2 → Gol Time 2
    // ---------------------
    if (!waitingRecovery2)
    {
        if (prox2 > GOL_THRESHOLD)
        {
            if (startHigh2 == 0)
                startHigh2 = now;

            if ((now - startHigh2) >= GOL_HOLD_TIME)
            {
                processandoGol(sensor2, I2C_2, prox2);
                score2++;
                waitingRecovery2 = true;
                startHigh2 = 0;
                Serial.println("🏒 GOL TIME 2!");
                showPlacar();
            }
        }
        else
        {
            startHigh2 = 0;
        }
    }
    else if (prox2 < GOL_THRESHOLD)
    {
        waitingRecovery2 = false;
    }

    delay(50);
}