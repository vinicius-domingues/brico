#include <Arduino.h>
#include "syntax.h"
#include "tokens.h"
#include "controller.h"
#include "car_actuator.h"
#include "evaluator.h"

Controller* arduino;
Syntax* analisador;
Car* carrinho;
Evaluator* executor;

static const int blocks_limit = 100; // Limite de blocos no sistema
int sequence[blocks_limit];          // Array de sequencia
int blocks_read = 0;                 // Posições do array que foram usadas

unsigned long brand_new_instant = 0;
unsigned long seconds_running = 0;
enum SystemState {
    STATE_DEBUG,
    STATE_COMPILE,
    STATE_RUNNING
};

SystemState currentState = STATE_DEBUG; 

void setup() {
    Serial.begin(9600);

    Serial.println(" ");

    arduino = new Controller();
    carrinho = new Car();
    analisador = new Syntax();

    Serial.println(" ");
}

void loop() {
    switch (currentState) {
        case STATE_DEBUG:
            arduino->DebugMenu();
            currentState = STATE_COMPILE;

        case STATE_COMPILE: {
            int error_stage = 0; 

            // arduino->Listener();
            
            // arduino->Mapper(sequence, blocks_read); // Tem que dar erro se houver mais que 100 blocos.
            
            // Testes apenas
            Serial.println(F("[MAIN] Rodando em modo de TESTE"));
            int teste[] = {_START, _RED_LED, _DELAY, _FIVE, _ENDFUNCTION, _GREEN_LED, _END};
            blocks_read = sizeof(teste) / sizeof(teste[0]);
            memcpy(sequence, teste, sizeof(teste));

            if (analisador->Parser(sequence, blocks_read)) {
                error_stage = 1;
            } else if (analisador->LookAhead(sequence, blocks_read)) {
                error_stage = 2;
            } else if (analisador->Semantic(sequence, blocks_read)) {
                error_stage = 3;
            }       

            // Controle de transição com base no resultado
            if (error_stage == 0) {
                Serial.println(F("[MAIN] Sem erros. Passando código validado via UART."));

                // Passaria via UART. Não terá aqui o Evaluator nem o Executor
                    // Passamos: Sequencia uma a uma e se é loop ou não (o tamanho da sequencia n precisa pois isso pode ser feito no EvaL)

                // Mata objeto executor anterior
                if (executor != nullptr) { delete executor; }

                // Instância
                executor = new Evaluator(sequence, blocks_read, arduino->is_loop, carrinho);
                
                currentState = STATE_RUNNING; 

                Serial.println(F("[MAIN] Avaliação começando."));
                
            } else {
                Serial.println(F("[MAIN] Voltando ao Debug devido a erros."));
                if (analisador != nullptr) { delete analisador; }
                if (executor != nullptr) { delete executor; }
                currentState = STATE_DEBUG; 
            }
            break;
        }

        case STATE_RUNNING: {
            unsigned long actual_instant = millis();

            // Incrementa nossa variável 'Segundos', com no mínimo de taxa de atualização de 1 segundo
            if (actual_instant - brand_new_instant >= 1000) {
                Serial.println(F("[EVAL] +1s."));
                seconds_running += (actual_instant - brand_new_instant) / 1000;           
                brand_new_instant = actual_instant; 
            }

            // Executa até voltar para esperar outro eventual código
            if (executor->run) {
                Serial.println(F("[EVAL] Rodando"));
                executor->Eval(seconds_running);
            } else {
                Serial.println(F("\n[MAIN] Fim do script alcançado! Execução do carrinho concluída.    //    Retornando ao console de depuração..."));
                delete analisador; 
                currentState = STATE_DEBUG; 
            }

            break;
        }
    }

    Serial.println(" ");
}