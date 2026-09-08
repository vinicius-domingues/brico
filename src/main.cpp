#include <Arduino.h>
#include "tokens.h"
#include "car_actuator.h"
#include "evaluator.h"

// GPIO 6 a 11 sao da Flash SPI interna no ESP32. GPIO 33 e livre e seguro conforme o MD.
#define PIN_DOCK_SENSOR 33  
#define PIN_CAR_BUTTON  4   // BOTAO — GPIO4 conforme configuracao_carrinho_brico.md

Car*       carrinho = nullptr;
Evaluator* executor = nullptr;

static const int MAX_BLOCKS = 100;
int sequence[MAX_BLOCKS];
int blocks_read = 0;
bool is_loop_mode = false;

enum CarState {
    STATE_CHECK_CAIXA, // Verifica se o carrinho esta na caixa
    STATE_WAIT_BOX,     // Aguarda receber os dados via UART da caixa
    STATE_STORAGE,     // Armazena e confirma a sequencia recebida
    STATE_LISTENER,    // Fora da caixa: aguarda clique no botao do carrinho
    STATE_EXECUTE      // Executa o script interpretado pelo Evaluator
};

CarState currentState = STATE_CHECK_CAIXA;

bool isNaCaixa() {
    return digitalRead(PIN_DOCK_SENSOR) == LOW; 
}

void setup() {
    Serial.begin(115200);
    delay(1000); // Aguarda serial estabilizar
    Serial.println(F("\n=========================================="));
    Serial.println(F("[SETUP] ESP32 ACORDOU COM SUCESSO!"));
    Serial.println(F("=========================================="));

    Serial.println(F("[SETUP] Configurando pinos basicos (DOCK e BOTAO)..."));
    pinMode(PIN_DOCK_SENSOR, INPUT_PULLUP);
    pinMode(PIN_CAR_BUTTON, INPUT_PULLDOWN);

    Serial.println(F("[SETUP] Instanciando Car..."));
    carrinho = new Car();
    Serial.println(F("[SETUP] Setup finalizado com sucesso!"));
}

void loop() {
    switch (currentState) {

        case STATE_CHECK_CAIXA: {
            Serial.println(F("Estou vendo se estou na caixa ou nao"));
            if (isNaCaixa()) {
                Serial.println(F("Estou na caixa esperando UART"));
                currentState = STATE_WAIT_BOX;
            } else {
                Serial.println(F("Estou fora da caixa, aguardando meu botao pra comecar"));
                currentState = STATE_LISTENER;
            }
            break;
        }

        case STATE_WAIT_BOX: {
            Serial.println(F("Estou esperando vir codigos da caixa"));
            if (Serial.available() > 0) {
                String line = Serial.readStringUntil('\n');
                line.trim();

                if (line.startsWith("START:")) {
                    blocks_read = line.substring(6).toInt();
                    int idx = 0;
                    unsigned long timeout = millis() + 3000;

                    while (millis() < timeout && idx < blocks_read) {
                        if (Serial.available() > 0) {
                            String blockLine = Serial.readStringUntil('\n');
                            blockLine.trim();

                            if (blockLine.startsWith("BLOCK[")) {
                                int colonIdx = blockLine.indexOf(':');
                                if (colonIdx != -1) {
                                    sequence[idx++] = blockLine.substring(colonIdx + 1).toInt();
                                }
                            } else if (blockLine == "END") {
                                break;
                            }
                        }
                    }
                    currentState = STATE_STORAGE;
                }
            }
            break;
        }

        case STATE_STORAGE: {
            Serial.println(F("Blocos armazenados na memoria"));
            Serial.println(F("ACK"));
            carrinho->GreenLed();
            currentState = STATE_CHECK_CAIXA;
            break;
        }

        case STATE_LISTENER: {
            // Se foi recolocado na caixa, volta a checar
            if (isNaCaixa()) {
                currentState = STATE_CHECK_CAIXA;
                break;
            }

            if (digitalRead(PIN_CAR_BUTTON) == HIGH) {
                delay(50); // Debounce simples
                if (digitalRead(PIN_CAR_BUTTON) == HIGH) {
                    if (blocks_read == 0) {
                        Serial.println(F("[LISTENER] Botao pressionado, mas NENHUM bloco foi carregado pela caixa ainda!"));
                        delay(500);
                        break;
                    }

                    Serial.print(F("[LISTENER] Iniciando execucao com "));
                    Serial.print(blocks_read);
                    Serial.println(F(" blocos..."));

                    if (executor != nullptr) { delete executor; }
                    executor = new Evaluator(sequence, blocks_read, is_loop_mode, carrinho);
                    currentState = STATE_EXECUTE;
                }
            }
            break;
        }

        case STATE_EXECUTE: {
            if (executor != nullptr && executor->run) {
                executor->Eval();
            } else {
                Serial.println(F("Executou tudo"));

                if (executor != nullptr) {
                    delete executor;
                    executor = nullptr;
                }

                carrinho->Brake();
                
                // Apos executar, volta para o LISTENER aguardando novo start
                currentState = STATE_LISTENER;
            }
            break;
        }
    }
}
