#include <tokens.h>
#include "car_actuator.h"

// ---------------------------------------------------------------------------
// Construtor — inicializa pinos e fita WS2812
// ---------------------------------------------------------------------------
Car::Car() : strip(QUANTIDADE_LEDS, WS2812_DIN, NEO_GRB + NEO_KHZ800) {
    Serial.println(F("[CARRO] Inicializando hardware e pinos..."));

    // Motores
    pinMode(MOTOR_ESQ_IN1, OUTPUT);
    pinMode(MOTOR_ESQ_IN2, OUTPUT);
    pinMode(MOTOR_DIR_IN1, OUTPUT);
    pinMode(MOTOR_DIR_IN2, OUTPUT);

    // Mantém motores desligados durante a inicialização
    digitalWrite(MOTOR_ESQ_IN1, LOW);
    digitalWrite(MOTOR_ESQ_IN2, LOW);
    digitalWrite(MOTOR_DIR_IN1, LOW);
    digitalWrite(MOTOR_DIR_IN2, LOW);

    // Botão (conectado entre 3,3 V e GPIO4 — pull-down interno)
    pinMode(BOTAO, INPUT_PULLDOWN);

    // Buzzer
    pinMode(BUZZER, OUTPUT);
    digitalWrite(BUZZER, LOW);

    // Sensor ultrassônico HC-SR04
    pinMode(ULTRASSONICO_TRIG, OUTPUT);
    digitalWrite(ULTRASSONICO_TRIG, LOW);
    pinMode(ULTRASSONICO_ECHO, INPUT);

    // Fita WS2812 — apaga todos os LEDs na inicialização
    strip.begin();
    strip.clear();
    strip.show();
}

// ---------------------------------------------------------------------------
// Frear — desliga os dois motores (todos os pinos em LOW)
// ---------------------------------------------------------------------------
void Car::Brake() {
    Serial.println(F("[CARRO] Acao executada: Frear"));
    digitalWrite(MOTOR_ESQ_IN1, LOW);
    digitalWrite(MOTOR_ESQ_IN2, LOW);
    digitalWrite(MOTOR_DIR_IN1, LOW);
    digitalWrite(MOTOR_DIR_IN2, LOW);
}

// ---------------------------------------------------------------------------
// Acelerar — aciona os dois motores para frente
// IN1=HIGH / IN2=LOW em ambos os lados
// ---------------------------------------------------------------------------
void Car::Accelerate() {
    Serial.println(F("[CARRO] Acao executada: Acelerar"));
    digitalWrite(MOTOR_ESQ_IN1, HIGH);
    digitalWrite(MOTOR_ESQ_IN2, LOW);
    digitalWrite(MOTOR_DIR_IN1, HIGH);
    digitalWrite(MOTOR_DIR_IN2, LOW);
}

// ---------------------------------------------------------------------------
// Buzinar — emite três tons via tone() no pino BUZZER
// ---------------------------------------------------------------------------
void Car::Honk() {
    Serial.println(F("[CARRO] Acao executada: Buzinar"));
    tone(BUZZER, 1000, 150);
    delay(200);
    tone(BUZZER, 1500, 150);
    delay(200);
    tone(BUZZER, 2000, 150);
    delay(200);
}

// ---------------------------------------------------------------------------
// LEDs WS2812 — acende toda a fita na cor correspondente
// ---------------------------------------------------------------------------
void Car::RedLed() {
    Serial.println(F("[CARRO] Acao executada: Acender LED Vermelho"));
    strip.fill(strip.Color(255, 0, 0));
    strip.show();
}

void Car::GreenLed() {
    Serial.println(F("[CARRO] Acao executada: Acender LED Verde"));
    strip.fill(strip.Color(0, 255, 0));
    strip.show();
}

void Car::BlueLed() {
    Serial.println(F("[CARRO] Acao executada: Acender LED Azul"));
    strip.fill(strip.Color(0, 0, 255));
    strip.show();
}

// ---------------------------------------------------------------------------
// Proximidade — HC-SR04: retorna true se objeto detectado < PROXIMIDADE_LIMIAR_CM
// ---------------------------------------------------------------------------
bool Car::Proximity() {
    Serial.println(F("[CARRO] Leitura de Sensor: Proximidade"));

    // Pulso de disparo: LOW → HIGH por 10 µs → LOW
    digitalWrite(ULTRASSONICO_TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(ULTRASSONICO_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(ULTRASSONICO_TRIG, LOW);

    // Mede duração do eco (timeout 30 ms = ~5 m máx.)
    unsigned long duracao = pulseIn(ULTRASSONICO_ECHO, HIGH, 30000UL);
    float distanciaCm = duracao / 58.0f;

    Serial.print(F("[CARRO] Distancia medida (cm): "));
    Serial.println(distanciaCm);

    return (distanciaCm > 0.0f && distanciaCm < PROXIMIDADE_LIMIAR_CM);
}
