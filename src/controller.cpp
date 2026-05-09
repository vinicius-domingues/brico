#include "Controller.h"
#include <tokens.h>


Controller::Controller() {
    Wire.begin(); 
    pinMode(PIN_SET, OUTPUT);
    pinMode(PIN_CLOCK, OUTPUT);
    pinMode(PIN_BUTTON, INPUT_PULLUP);
    
    digitalWrite(PIN_SET, LOW);
    digitalWrite(PIN_CLOCK, LOW);
}

void Controller::Prepare() {
    digitalWrite(PIN_SET, HIGH);
    delay(10); 
    Clock();
    digitalWrite(PIN_SET, LOW);
    delay(10);
    Serial.println(F("[HARDWARE] Prepare enviado."));
}

void Controller::Stop() {
    Serial.println(F("[STOP]"));
    delay(999999);
}

void Controller::Clock() {
    digitalWrite(PIN_CLOCK, HIGH);
    delay(10); 
    digitalWrite(PIN_CLOCK, LOW);
    delay(10);
    Serial.println(F("[HARDWARE] Pulso de CLOCK enviado."));
}

void Controller::Listener() {
    Serial.println(F("[CONTROLLER] Aguardando botao (Pino 5) para iniciar leitura..."));
    
    while (digitalRead(PIN_BUTTON) == HIGH) {
        delay(50); 
    }
    
    while (digitalRead(PIN_BUTTON) == LOW) {
        delay(50);
    }
    
    Serial.println(F("[CONTROLLER] Botao pressionado! Acionando Mapper."));
}

byte Controller::readEEPROM(int address) {
  byte deviceAddr;

  if (address < 256) {
    deviceAddr = EEPROM_ADDR_0;
  } else {
    deviceAddr = EEPROM_ADDR_1;
    address -= 256;
  }

  Wire.beginTransmission(deviceAddr);
  Wire.write(address); 
  Wire.endTransmission();

  Wire.requestFrom(deviceAddr, (byte)1);

  if (Wire.available()) {
    return Wire.read();
  }

  return 0xFF; 
}

void Controller::DebugMenu() {
    bool in_debug = true;
    bool estadoSet = false;

    Serial.println(F("\n====================================="));
    Serial.println(F("🛠️  CONSOLE DE DEBUG - FUZZY BLOCKS "));
    Serial.println(F("====================================="));
    Serial.println(F("'c' -> Gerar pulso de Clock"));
    Serial.println(F("'r' -> Ler EEPROM (Posição 1)"));
    Serial.println(F("'w' -> Escrever na EEPROM (Posição 1)"));
    Serial.println(F("'s' -> Alternar estado do pino SET"));
    Serial.println(F("'l' -> Sair do Debug e iniciar LISTENER"));
    Serial.println(F("'t' -> Transmitir para o outro Arduino"));

    Serial.println(F("====================================="));

    // Limpa qualquer lixo que tenha ficado no buffer da porta serial
    while(Serial.available() > 0) { Serial.read(); }

    // Trava o fluxo do Arduino neste menu até o usuário digitar 'L'
    while (in_debug) {
        if (Serial.available() > 0) {
            char comando = Serial.read();

            if (comando == 'c' || comando == 'C') {
                Clock();
            }
            else if (comando == 'r' || comando == 'R') {
                byte valor = readEEPROM(1); // Lê sempre da posição 1 (Shift Register)
                Serial.print(F("[DEBUG] Valor lido: "));
                Serial.println(valor);
            }
            else if (comando == 'w' || comando == 'W') {
                Serial.println(F("[DEBUG] Digite um valor numérico (0-255) para gravar:"));
                
                while (Serial.available() == 0); // Trava esperando o usuário digitar o valor
                int valor = Serial.parseInt();
                
                writeEEPROM(1, (byte)valor);
                delay(10);
                
                Serial.print(F("[DEBUG] Valor "));
                Serial.print(valor);
                Serial.println(F(" escrito com sucesso."));
            }
            else if (comando == 's' || comando == 'S') {
                estadoSet = !estadoSet;
                digitalWrite(PIN_SET, estadoSet ? HIGH : LOW);
                Serial.print(F("[DEBUG] Pino SET agora está: "));
                Serial.println(estadoSet ? F("LIGADO") : F("DESLIGADO"));
            }
            else if (comando == 'l' || comando == 'L') {
                Serial.println(F("\n[DEBUG] Saindo do console... Preparando Fluxo Principal."));
                in_debug = false; // Quebra o laço while, devolvendo o controle para o main.cpp
            }
            


        }
    }
}

void Controller::writeEEPROM(int address, byte data) {
    byte deviceAddr;

    // Lógica de endereço do seu readEEPROM adaptada para escrita
    if (address < 256) {
        deviceAddr = EEPROM_ADDR_0; 
    } else {
        deviceAddr = EEPROM_ADDR_1;
        address -= 256;
    }

    Wire.beginTransmission(deviceAddr);
    Wire.write(address); 
    Wire.write(data);
    Wire.endTransmission();
}

void Controller::Mapper(int sequence[], int& blocks_used) {
    Serial.println(F("\n[MAPPER] Iniciando leitura do barramento (Shift Register)"));
    
    blocks_used = 0;
    got_error = false;
    is_loop = false;

    int qtd_conditions = 0;
    int qtd_functions = 0;

    // Pilha para registrar a profundidade física (quantidade de clocks) das bifurcações
    int branch_stack[100]; 
    int stack_pointer = 0;
    int physical_clocks = 0; // Conta os shifts mecânicos executados

    // 1 - Injeta o _START no começo (posição 0) da fita na RAM
    sequence[blocks_used] = _START;
    blocks_used++;
    Serial.println(F("[MAPPER] [INJECAO] _START (1) adicionado obrigatoriamente no inicio."));

    // Prepara o hardware (Limpa o Shift Register e seta o primeiro bloco)
    Prepare();

    // Loop de varredura (controlado pelo tamanho máximo da RAM)
    while (blocks_used < 100) {
        
        // Dá o clock para o Shift Register avançar o estado ativo para a próxima peça
        physical_clocks++; // Registra que o bastão andou um passo físico

        // Faz a leitura do pino de dados. 
        // Como é um Shift Register, lemos apenas a posição 1 da EEPROM do bloco ativo.
        byte token_lido = readEEPROM(1); 

        Serial.print(F("[MAPPER] Lido do hardware ativo: "));
        Serial.println(token_lido);

        // 2 - Avalia se é o fim de um braço de roteamento ou fim geral (255)
        if (token_lido == 255 || token_lido == _END) {
            Serial.println(F("Li um final"));

            if (qtd_functions > 0) {
                Serial.println(F("[MAPPER] [SUBSTITUICAO] 255 trocado por _ENDFUNCTION (6)."));
                sequence[blocks_used] = _ENDFUNCTION;
                blocks_used++;
                qtd_functions--;
                
                // Mergulho de Resgate (Fast-Forward no Shift Register)
                Serial.println(F("[MAPPER] [RESET FISICO] Acionando Prepare() (Mergulho de Funcao)."));
                Prepare(); 
                
                // Resgata de qual clock (profundidade) o desvio da função ocorreu
                stack_pointer--;
                int target_clocks = branch_stack[stack_pointer];
                
                Serial.print(F("[MAPPER] [FAST-FORWARD] Shiftando cegamente "));
                Serial.print(target_clocks);
                Serial.println(F(" vezes para alcançar a Funcao e rotear para BAIXO..."));
                
                for (int j = 1; j < target_clocks; j++) {
                    Clock();
                }
                // Sincroniza o ponteiro de passos físicos com a nova realidade da placa
                physical_clocks = target_clocks; 

            } else if (qtd_conditions > 0) {
                Serial.println(F("[MAPPER] [SUBSTITUICAO] 255 trocado por _ENDCONDITION (4)."));
                sequence[blocks_used] = _ENDCONDITION;
                blocks_used++;
                qtd_conditions--;
                
                // Mergulho de Resgate (Fast-Forward no Shift Register)
                Serial.println(F("[MAPPER] [RESET FISICO] Acionando Prepare() (Mergulho de Condicao)."));
                Prepare(); 
                
                // Resgata de qual clock (profundidade) o desvio da condição ocorreu
                stack_pointer--;
                int target_clocks = branch_stack[stack_pointer];
                
                Serial.print(F("[MAPPER] [FAST-FORWARD] Shiftando cegamente "));
                Serial.print(target_clocks);
                Serial.println(F(" vezes para alcançar a Condicao e rotear para BAIXO..."));
                
                for (int j = 1; j < target_clocks; j++) {

                    Clock();
                }
                // Sincroniza o ponteiro de passos físicos com a nova realidade da placa
                physical_clocks = target_clocks; 

            } else {
                // Não está dentro de nada e leu 255
                Serial.println(F("[MAPPER] 255 lido com saldos estruturais zerados. FIM REAL DA TRILHA."));
                
                // Injeção de Borda: O código final exige _START como fechador absoluto da fita virtual
                sequence[blocks_used] = _START;
                //blocks_used = blocks_limit;
                blocks_used++;
                
                Serial.println(F("[MAPPER] [INJECAO] _START (1) adicionado como FIM absoluto da fita na RAM."));
                break; // Sai do laço while, mapeamento concluído
            }

        } else {
            // Se não for 255, rastreamos o tipo estrutural usando tokens.h e adicionamos na RAM
            if (isCondition(token_lido)) {
                qtd_conditions++;
                // Grava a profundidade do Shift Register na pilha ANTES da peça rotear fisicamente para a direita
                branch_stack[stack_pointer] = physical_clocks;
                stack_pointer++;
                Serial.println(F("[MAPPER] [ESTADO] Entrou em uma CONDICAO. Endereco eletrico salvo na pilha."));
                
            } else if (isFunction(token_lido)) {
                qtd_functions++;
                // Grava a profundidade do Shift Register na pilha ANTES da peça rotear fisicamente para a direita
                branch_stack[stack_pointer] = physical_clocks;
                stack_pointer++;
                Serial.println(F("[MAPPER] [ESTADO] Entrou em uma FUNCAO. Endereco eletrico salvo na pilha."));
            }

            // Grava o token lido na fita virtual da RAM do Arduino
            sequence[blocks_used] = token_lido;
            blocks_used++;
        }
        
        Clock(); // O prepare ja da um clock inicial, entao só precisa ler aqui

        delay(1000); 
    
    }
    
// Debug final de como ficou a fita salva na RAM do Arduino, pronta para o Evaluator
    Serial.println(F("=================================================="));
    Serial.print(F("[MAPPER] FITA FINAL DE TOKENS NA RAM: "));
    for (int i = 0; i < blocks_used; i++) {
        Serial.print(sequence[i]);
        if (i < blocks_used - 1) Serial.print(F(" - "));
    }
    Serial.println();

    // ==========================================
    // TRANSMISSÃO I2C (Pacotes de 1 Byte)
    // ==========================================
    
    // 1. Avisa o Slave qual é o tamanho total da fita
    Wire.beginTransmission(8); 
    Wire.write(blocks_used); 
    
    
    delay(20); // Dá um respiro pro Slave zerar os contadores dele

    // 2. Envia a fita verdadeira, uma peça por vez!
    for (int i = 0; i < blocks_used; i++) {
        
        Wire.write(sequence[i]); // Cast para byte garante a conversão limpa
        
        
        delay(100); // Intervalo REAL no barramento I2C
    }
    Wire.endTransmission();
    Serial.println(F("[I2C] Transmissão para o Slave 8 concluída com sucesso!"));
    Serial.println(F("=================================================="));





}