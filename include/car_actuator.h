#ifndef CAR_ACTUATOR_H
#define CAR_ACTUATOR_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

// ---------------------------------------------------------------------------
// Mapeamento de pinos — ESP32 (conforme configuracao_carrinho_brico.md)
// ---------------------------------------------------------------------------
constexpr uint8_t MOTOR_ESQ_IN1    = 13;
constexpr uint8_t MOTOR_ESQ_IN2    = 14;
constexpr uint8_t MOTOR_DIR_IN1    = 27;
constexpr uint8_t MOTOR_DIR_IN2    = 26;

constexpr uint8_t  WS2812_DIN       = 32;
constexpr uint16_t QUANTIDADE_LEDS  = 8;   // Ajustar para a fita utilizada

constexpr uint8_t BOTAO             = 4;
constexpr uint8_t BUZZER            = 15;
constexpr uint8_t ULTRASSONICO_TRIG = 18;
constexpr uint8_t ULTRASSONICO_ECHO = 19;

// ---------------------------------------------------------------------------
// Limiar de proximidade para Proximity() retornar true  (em centímetros)
// ---------------------------------------------------------------------------
constexpr float PROXIMIDADE_LIMIAR_CM = 20.0f;

class Car {
  public:
    Car();

    void Brake();
    void Accelerate();
    void Honk();

    bool Proximity();

    void RedLed();
    void GreenLed();
    void BlueLed();

  private:
    Adafruit_NeoPixel strip;
};

#endif


/*
    Tokens lógicos associados (definidos em tokens.h):
    #define _BRAKE      30   // Comando para frear              (VOID)
    #define _ACCELERATE 31   // Comando para acelerar           (VOID)
    #define _HONK       32   // Comando para buzinar            (VOID)
    #define _RED_LED    33   // Comando para acender led vermelho (VOID)
    #define _GREEN_LED  34   // Comando para acender led verde  (VOID)
    #define _BLUE_LED   35   // Comando para acender led azul   (VOID)
    #define _PROXIMITY  40   // Sensor ultrassônico             (BOOL)
*/