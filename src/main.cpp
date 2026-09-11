#include <Arduino.h>
#include "tokens.h"
#include "car_actuator.h"
#include "evaluator.h"

#define PIN_CAR_BUTTON  4   // BOTAO — GPIO4 conforme configuracao_carrinho_brico.md

// Pinos da Serial2 (Comunicação com a Caixa)
#define RXD2 16
#define TXD2 17

Car*       carrinho = nullptr;
Evaluator* executor = nullptr;

static const int MAX_BLOCKS = 100;
int sequence[MAX_BLOCKS];
int blocks_read = 0;
bool is_loop_mode = false;

// Variáveis para controle de tempo sem travar o código (millis)
unsigned long ultimoPing = 0;
unsigned long tempoUltimoDado = 0;

enum CarState {
    STATE_LISTENER,    // Fica em loop constante escutando a Caixa (UART) e o Botão
    STATE_WAIT_BOX,    // Lendo os dados enviados via UART
    STATE_STORAGE,     // Armazena e confirma a sequencia recebida
    STATE_EXECUTE      // Executa o script interpretado pelo Evaluator
};

// O carrinho já começa no modo ouvinte
CarState currentState = STATE_LISTENER; 

void setup() {
    // 1. Inicializa a porta Serial padrão (USB) para visualizar no Monitor Serial
    Serial.begin(115200);

    // 2. Inicializa a porta Serial2 (UART2)
    Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);

    delay(1000); // Aguarda serial estabilizar

    pinMode(PIN_CAR_BUTTON, INPUT_PULLDOWN);
    carrinho = new Car();
    Serial.println(F("[SETUP] Setup finalizado. Iniciando fluxo..."));
}

void loop() {
    switch (currentState) {

        case STATE_LISTENER: {
            // 2. PRIORIDADE 1: A CAIXA. Escuta a Serial2 (Pinos 16 e 17) o tempo todo
            if (Serial2.available() > 0) {
                String dadosRecebidos = Serial2.readStringUntil('\n');
                
                // Remove espaços em branco ou quebras de linha (\r) no final da string
                dadosRecebidos.trim();

                if (dadosRecebidos == "a") { 
                    Serial.println(F("[LISTENER] Recebida flag 'a'. Preparando para ler blocos..."));
                    blocks_read = 0; // Zera a contagem para a nova leitura
                    tempoUltimoDado = millis(); // Inicializa o temporizador
                    currentState = STATE_WAIT_BOX;
                }
                break;
            }

            // 3. PRIORIDADE 2: O BOTÃO.
            if (digitalRead(PIN_CAR_BUTTON) == HIGH) {
                delay(50); // Debounce simples

                if (digitalRead(PIN_CAR_BUTTON) == HIGH) {

                    // Se a memória estiver vazia, ele NÃO ANDA e foca de volta na UART
                    if (blocks_read == 0) {
                        Serial.println(F("[AVISO] Botao apertado, mas NÃO HÁ CÓDIGO! Voltando a esperar a UART2..."));

                        // Trava aqui até você soltar o botão, para não flodar o terminal
                        while(digitalRead(PIN_CAR_BUTTON) == HIGH) { delay(10); }

                        // Zera o timer do ping para ele exibir a mensagem de "Aguardando" imediatamente
                        ultimoPing = 0; 
                        break; 
                    }

                    // Se tiver código, parte para a execução
                    Serial.print(F("\n[EXECUCAO] Iniciando carrinho com "));
                    Serial.print(blocks_read);
                    Serial.println(F(" blocos..."));

                    if (executor != nullptr) { delete executor; }
                    executor = new Evaluator(sequence, blocks_read, is_loop_mode, carrinho);

                    // Aguarda soltar o botão antes de andar
                    while(digitalRead(PIN_CAR_BUTTON) == HIGH) { delay(10); } 

                    currentState = STATE_EXECUTE;
                    break;
                }
            }
            break;
        }

        case STATE_WAIT_BOX: {
            // Se houver dados chegando, lemos o número
            if (Serial2.available() > 0) {
                String blockLine = Serial2.readStringUntil('\n');
                blockLine.trim();

                if (blockLine != "z") {
                    Serial.print(F("[DEBUG UART2] Numero Recebido: "));
                    Serial.println(blockLine);

                    if (blocks_read < MAX_BLOCKS) {
                        // Converte diretamente a string com o número para inteiro e armazena
                        sequence[blocks_read++] = blockLine.toInt();
                    } else {
                        Serial.println(F("[ERRO] Limite maximo de blocos atingido!"));
                    }
                }else{
                  Serial.print(F("Saindo pq recebi o Z"));
                  currentState = STATE_STORAGE;
                }
                
                // Atualiza o tempo do último dado recebido com sucesso
                tempoUltimoDado = millis(); 
                
            } 
            break;
        }

        case STATE_STORAGE: {
            Serial.println(F("\n[SUCESSO] Blocos armazenados na memoria!"));

            // Avisa a caixa que recebeu tudo com sucesso
            //carrinho->GreenLed();

            // Limpa o buffer da Serial2 pra garantir que não sobrou lixo (evita loop infinito)!
            delay(10); 
            while(Serial2.available()) { Serial2.read(); } 

            // Tudo salvo, volta para o LISTENER pronto para rodar
            ultimoPing = 0; 
            currentState = STATE_LISTENER;
            break;
        }

        case STATE_EXECUTE: {
            if (executor != nullptr && executor->run) {
                executor->Eval();
            } else {
                Serial.println(F("[FIM] Percurso finalizado!"));

                if (executor != nullptr) {
                    delete executor;
                    executor = nullptr;
                }

                carrinho->Brake();

                // Apos executar, volta a escutar a UART e o Botao
                ultimoPing = 0;
                currentState = STATE_LISTENER;
            }
            break;
        }
    }
}