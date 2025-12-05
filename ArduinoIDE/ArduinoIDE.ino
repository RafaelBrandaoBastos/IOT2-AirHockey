#include <Wire.h>
#include <APDS9930.h>
#include "display_utils.h"

APDS9930 sensor1;
APDS9930 sensor2;
TwoWire I2C_1 = TwoWire(0);
TwoWire I2C_2 = TwoWire(1);

// Dual PWM Fan Fixed Speed Control
// Controla 2 fans de 4 pinos com velocidade fixa
// Seeeduino XIAO ESP32-S3
const int FAN1_PIN = 23;
const int FAN2_PIN = 32;
// Configuração PWM
const int PWM_FREQ = 25000;   // Frequência do PWM (25 kHz)
const int PWM_RESOLUTION = 8; // Resolução (0-255)
const int FAN_SPEED = 153;    // Velocidade fixa (~60%) -> ajuste como quiser!

//
int counter = 0;

void setup()
{

  Serial.begin(115200);

  // Configurar PWM nos dois canais automaticamente
  Serial.println("Dual Fan Controller Starting...");
  ledcAttach(FAN1_PIN, PWM_FREQ, PWM_RESOLUTION);
  ledcAttach(FAN2_PIN, PWM_FREQ, PWM_RESOLUTION);
  // Aplicar velocidade fixa
  ledcWrite(FAN1_PIN, FAN_SPEED);
  ledcWrite(FAN2_PIN, FAN_SPEED);
  Serial.print("Fans set to fixed speed: ");
  Serial.print(map(FAN_SPEED, 0, 255, 0, 100));
  Serial.println("%");

  // Barramento do sensor 1
  I2C_1.begin(18, 19);
  Wire = I2C_1; // <- HACK: força o APDS usar este I2C
  if (sensor1.init())
  {
    Serial.println("Sensor 1 OK!");
    sensor1.enableProximitySensor(false);
  }
  else
  {
    Serial.println("Sensor 1 falhou!");
  }

  // Barramento do sensor 2
  I2C_2.begin(21, 22);
  Wire = I2C_2; // <- troca o I2C antes do init do sensor 2
  if (sensor2.init())
  {
    Serial.println("Sensor 2 OK!");
    sensor2.enableProximitySensor(false);
  }
  else
  {
    Serial.println("Sensor 2 falhou!");
  }

  initDisplays();
}

void loop()
{

  uint16_t prox1, prox2;
  Wire = I2C_1;
  if (sensor1.readProximity(prox1))
  {
    Serial.print("S1: ");
    Serial.print(prox1);
  }
  Serial.print("  |  ");
  Wire = I2C_2;
  if (sensor2.readProximity(prox2))
  {
    Serial.print("S2: ");
    Serial.println(prox2);
  }

  // Mostra o número nos 2 displays, sempre com 4 dígitos
  showNumberOnDisplays(counter);
  counter++;
  if (counter > 9999)
  {
    counter = 0;
  }

  delay(500);
}
