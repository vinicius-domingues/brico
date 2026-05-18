#include <Arduino.h>
#include "syntax.h"
#include "tokens.h"
#include "controller.h"
#include "car_actuator.h"
#include "evaluator.h"

Controller* arduino;
Syntax*     analisador;
Car*        carrinho;
Evaluator*  executor;

// Ponte global: repassa o callback do Syntax para o IlluminateBlock do Controller
static void _blockLedBridge(int blockIndex, byte color) {
    if (arduino != nullptr) arduino->IlluminateBlock(blockIndex, color);
}

static const int blocks_limit = 100; // Limite de blocos no sistema
int sequence[blocks_limit];          // Array de sequencia
int blocks_read = 0;                 // Posições do array que foram usadas
unsigned long brand_new_instant = 0;
unsigned long seconds_running = 0;
enum SystemState {STATE_DEBUG, STATE_COMPILE, STATE_RUNNING};
SystemState currentState = STATE_DEBUG; 

void setup() {
    Serial.begin(9600);

    Serial.println(" ");
        arduino = new Controller();
        carrinho = new Car();
        analisador = new Syntax();
        arduino->setupSegDisplay();
        arduino->ShowState(SEG_STATE_DEBUG);
    Serial.println(" ");
}

void loop() {
    switch (currentState) {
        case STATE_DEBUG:
            arduino->ShowState(SEG_STATE_DEBUG);
            arduino->DebugMenu();
            currentState = STATE_COMPILE;

        case STATE_COMPILE: {
            arduino->ShowState(SEG_STATE_COMPILE);
            arduino->ResetBlockLeds(); // Apaga todos os LEDs antes de comecar a varredura
            int error_stage = 0; 

            // arduino->Listener();
            
            // arduino->Mapper(sequence, blocks_read); // Tem que dar erro se houver mais que 100 blocos.
            
            // Testes apenas
            Serial.println(F("[MAIN] Rodando em modo de TESTE"));
            // int teste[] = {_START, _WHILE, _PROXIMITY, _EQUAL, _FALSE, _AND, _SMALLER, _FIVE, _ENDCONDITION, _GREEN_LED, _ENDBLOCK, _RED_LED, _END};
            // int teste[] = {_START, _WHILE, _SEGUNDOS, _SMALLER, _FIFTY, _ENDCONDITION, _WHILE, _SEGUNDOS, _SMALLER, _FIVE, _ENDCONDITION, _RED_LED, _ENDBLOCK, _GREEN_LED, _ENDBLOCK, _BLUE_LED, _START};
            // int teste[] = {_START, _WHILE, _SEGUNDOS, _SMALLER, _FIVE, _ENDCONDITION, _RED_LED, _ENDBLOCK, _GREEN_LED, _END};
            int teste[] = {_START, _RED_LED, _DELAY, _ONE, _ENDFUNCTION, _GREEN_LED, _DELAY, _ONE, _ENDFUNCTION, _BLUE_LED, _DELAY, _ONE, _ENDFUNCTION,_START}; // , _GREEN_LED, _DELAY, _ONE, _ENDFUNCTION, _GREEN_LED, _DELAY, _ONE, _ENDFUNCTION, _START};
            blocks_read = sizeof(teste) / sizeof(teste[0]);
            memcpy(sequence, teste, sizeof(teste));

            // Conecta callback de LEDs ao analisador
            analisador->onBlockLed = _blockLedBridge;

            if (analisador->Parser(sequence, blocks_read)) {
                error_stage++;
            } else if (analisador->LookAhead(sequence, blocks_read)) {
                error_stage++;
            } else if (analisador->Semantic(sequence, blocks_read)) {
                error_stage++;
            }       

            // Controle de transição com base no resultado
            if (error_stage == 0) {
                Serial.println(F("[MAIN] Sem erros. Passando codigo validado via UART."));

                // Passaria via UART. Não terá aqui o Evaluator nem o Executor
                    // Passamos: Sequencia uma a uma e se é loop ou não (o tamanho da sequencia n precisa pois isso pode ser feito no EvaL)

                // Mata objeto executor anterior
                if (executor != nullptr) { delete executor; }

                // Instância
                executor = new Evaluator(sequence, blocks_read, arduino->is_loop, carrinho);
                
                currentState = STATE_RUNNING; 
                arduino->ShowState(SEG_STATE_RUNNING);
                Serial.println(F("[MAIN] Avaliacao comecando."));
                
            } else {
                // LEDs ja foram atualizados pelo callback dentro do Syntax
                // Exibe 'E' + codigo no display e trava ate o botao ser pressionado
                arduino->ShowError(analisador->result);
                if (analisador != nullptr) { delete analisador; }
                if (executor != nullptr)  { delete executor; }
                currentState = STATE_DEBUG; 
            }
            break;
        }

        case STATE_RUNNING: {
            if (executor->run) {
                executor->Eval();
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