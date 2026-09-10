#include <Arduino.h>
#include "syntax.h"
#include "tokens.h"
#include "controller.h"

Controller* arduino;
Syntax*     analisador;

// [BLOCK_LED DESATIVADO] Ponte global: repassa o callback do Syntax para IlluminateBlock
// static void _blockLedBridge(int blockIndex, byte color) {
//     if (arduino != nullptr) arduino->IlluminateBlock(blockIndex, color);
// }

static const int blocks_limit = 100; // Limite de blocos no sistema
int sequence[blocks_limit];          // Array de sequencia
int blocks_read = 0;                 // Posições do array que foram usadas
unsigned long brand_new_instant = 0;
unsigned long seconds_running = 0;
enum SystemState {STATE_LISTENER, STATE_ERROR, STATE_COMPILE, STATE_UART};
SystemState currentState = STATE_LISTENER; 

void setup() {
    Serial.begin(115200);
    Serial2.begin(115200, SERIAL_8N1, PIN_UART_RX, PIN_UART_TX);

    arduino = new Controller();
    analisador = new Syntax();

}

void loop() {
    switch (currentState) {
        case STATE_LISTENER: {
            // Aguarda botão para ler novamente
            arduino->Listener();
            currentState = STATE_COMPILE;
            break;
        }

        case STATE_COMPILE: {
            arduino->ShowState(SEG_STATE_COMPILE);
            // arduino->ResetBlockLeds(); // [BLOCK_LED DESATIVADO] Apaga todos os LEDs antes de comecar a varredura
            int error_stage = 0; 
            
            arduino->Mapper(sequence, blocks_read); // Tem que dar erro se houver mais que 100 blocos.

            // --- Print da sequência lida pelo Mapper ---
            Serial.print(F("[MAIN] Sequencia lida ("));
            Serial.print(blocks_read);
            Serial.print(F(" blocos): ["));
            for (int i = 0; i < blocks_read; i++) {
                if (i > 0) Serial.print(F(", "));
                Serial.print(sequence[i]);
            }
            Serial.println(F("]"));

            // analisador->onBlockLed = _blockLedBridge; // [BLOCK_LED DESATIVADO]

            if (analisador->Parser(sequence, blocks_read)) {
                error_stage++;
            } else if (analisador->LookAhead(sequence, blocks_read)) {
                error_stage++;
            } else if (analisador->Semantic(sequence, blocks_read)) {
                error_stage++;
            }       

            // Controle de transição com base no resultado
            if (error_stage == 0) {
                // Transmite a sequência para o carrinho via UART
                currentState = STATE_UART; 

                arduino->ShowState(SEG_STATE_RUNNING);
                Serial.println(F("[MAIN] Compilacao concluida. Pronto para transmitir."));
                
            } else {
                // LEDs ja foram atualizados pelo callback dentro do Syntax
                // Exibe 'E' + codigo no display e trava ate o botao ser pressionado
                arduino->ShowError(analisador->result);
                if (analisador != nullptr) { delete analisador; }
                currentState = STATE_ERROR; 
            }
            break;
        }

        case STATE_ERROR: {
            currentState = STATE_COMPILE;
            break;
        }

        case STATE_UART: {
            Serial.println(F("iniciando passagem"));

            for (int i = 0; i < blocks_read; i++) {
                Serial2.println(sequence[i]);
                delay(10);
            }

            currentState = STATE_LISTENER;
            break;
        }
    }

    Serial.println(F(" "));
}