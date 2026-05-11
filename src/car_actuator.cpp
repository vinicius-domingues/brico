#include "Controller.h"
#include <tokens.h>
#include "car_actuator.h"

Car::Car(){
    Serial.println(F("[CARRO] Inicializando hardware e pinos..."));
    pinMode(PIN_RED_LED, OUTPUT);
    pinMode(PIN_GREEN_LED, OUTPUT);
    pinMode(PIN_BLUE_LED, OUTPUT);
    
    pinMode(PIN_SENSOR, INPUT_PULLUP);
    
    digitalWrite(PIN_RED_LED, LOW);
    digitalWrite(PIN_GREEN_LED, LOW);
    digitalWrite(PIN_BLUE_LED, LOW);    
}

void Car::Brake() {
    Serial.println(F("[CARRO] Acao executada: Frear"));
    digitalWrite(PIN_MOTOR_A, LOW);
    digitalWrite(PIN_MOTOR_B, LOW);
}

void Car::Accelerate() {
    Serial.println(F("[CARRO] Acao executada: Acelerar"));
    digitalWrite(PIN_MOTOR_A, HIGH);
    digitalWrite(PIN_MOTOR_B, LOW);
}

void Car::Honk() {
    Serial.println(F("[CARRO] Acao executada: Buzinar"));
    digitalWrite(PIN_BUZZER, HIGH);
    delay(200); 
    digitalWrite(PIN_BUZZER, LOW);
}

void Car::RedLed() {
    Serial.println(F("[CARRO] Acao executada: Acender LED Vermelho"));
    digitalWrite(PIN_GREEN_LED, LOW);
    digitalWrite(PIN_BLUE_LED, LOW);
    digitalWrite(PIN_RED_LED, HIGH);
}

void Car::GreenLed() {
    Serial.println(F("[CARRO] Acao executada: Acender LED Verde"));
    digitalWrite(PIN_RED_LED, LOW);
    digitalWrite(PIN_BLUE_LED, LOW);
    digitalWrite(PIN_GREEN_LED, HIGH);
}

void Car::BlueLed() {
    Serial.println(F("[CARRO] Acao executada: Acender LED Azul"));
    digitalWrite(PIN_RED_LED, LOW);
    digitalWrite(PIN_GREEN_LED, LOW);
    digitalWrite(PIN_BLUE_LED, HIGH);
}

bool Car::Proximity() {
    Serial.println(F("[CARRO] Leitura de Sensor: Proximidade"));
    return(LOW == digitalRead(PIN_SENSOR));
}