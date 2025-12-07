#include <Wire.h>
#include <APDS9930.h>
#include "display_utils.h"

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

    // Criar token (data+hora fictícia)
    int day = 5;
    int month = 12;
    int year = 2025;
    int hour = 14;
    int minute = 0;
    Serial.print("Nova partida iniciada com o token: ");
    Serial.print(day);
    Serial.print("/");
    Serial.print(month);
    Serial.print("/");
    Serial.print(year);
    Serial.print("/");
    if (hour < 10)
        Serial.print("0");
    Serial.print(hour);
    Serial.print(":");
    if (minute < 10)
        Serial.print("0");
    Serial.println(minute);
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
            showTextOnDisplays("    ");
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
                showTextOnDisplays("    ");
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
            startHigh1 = 0;
    }
    else if (prox1 < GOL_THRESHOLD)
        waitingRecovery1 = false;

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
            startHigh2 = 0;
    }
    else if (prox2 < GOL_THRESHOLD)
        waitingRecovery2 = false;

    delay(50);
}