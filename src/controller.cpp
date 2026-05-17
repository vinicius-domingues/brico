#include "Controller.h"
#include <tokens.h>
#include <errors.h>

// ===========================================================================
// TABELAS DO DISPLAY 7 SEGMENTOS
// Ordem dos segmentos: { A, B, C, D, E, F, G }
// 1 = Ligado, 0 = Desligado (Cátodo Comum)
// ===========================================================================

// --- Letras de estado (índices SEG_STATE_*) ---
static const byte letrasEstado[5][7] = {
  {0, 1, 1, 1, 1, 0, 1},  // 0: 'd' (Debug)      -> B, C, D, E, G
  {1, 0, 0, 1, 1, 1, 0},  // 1: 'C' (Compilando) -> A, D, E, F
  {0, 0, 0, 0, 1, 0, 1},  // 2: 'r' (Running)    -> E, G
  {1, 0, 0, 1, 1, 1, 1},  // 3: 'E' (Error)      -> A, D, E, F, G
  {0, 0, 0, 1, 1, 1, 0}   // 4: 'L' (Listening)  -> D, E, F
};

// --- Dígitos numéricos 0-9 ---
static const byte digitos[10][7] = {
//  A  B  C  D  E  F  G
  { 1, 1, 1, 1, 1, 1, 0 },  // 0
  { 0, 1, 1, 0, 0, 0, 0 },  // 1
  { 1, 1, 0, 1, 1, 0, 1 },  // 2
  { 1, 1, 1, 1, 0, 0, 1 },  // 3
  { 0, 1, 1, 0, 0, 1, 1 },  // 4
  { 1, 0, 1, 1, 0, 1, 1 },  // 5
  { 1, 0, 1, 1, 1, 1, 1 },  // 6
  { 1, 1, 1, 0, 0, 0, 0 },  // 7
  { 1, 1, 1, 1, 1, 1, 1 },  // 8
  { 1, 1, 1, 1, 0, 1, 1 },  // 9
};

// --- Mapa de pinos de segmento {A, B, C, D, E, F, G} ---
static const byte segPins[7] = {
  PIN_SEG_A, PIN_SEG_B, PIN_SEG_C, PIN_SEG_D,
  PIN_SEG_E, PIN_SEG_F, PIN_SEG_G
};

// --- Mapa de pinos de seleção de dígito (DIG1 = mais à esquerda) ---
static const byte digPins[SEG_DIGITS] = {
  PIN_DIG_1, PIN_DIG_2, PIN_DIG_3, PIN_DIG_4
};

// ---------------------------------------------------------------------------
// Helper interno: escreve um padrão de 7 segmentos e ativa o dígito pedido
// ---------------------------------------------------------------------------
static void _writeDigit(int digIndex, const byte pattern[7]) {
    // Apaga todos os dígitos antes de trocar (evita ghosting)
    for (int d = 0; d < SEG_DIGITS; d++) {
        digitalWrite(digPins[d], HIGH); // HIGH = dígito desligado (cátodo comum)
    }
    // Escreve o padrão de segmentos
    for (int s = 0; s < 7; s++) {
        digitalWrite(segPins[s], pattern[s] ? HIGH : LOW);
    }
    // Ativa apenas o dígito desejado
    digitalWrite(digPins[digIndex], LOW); // LOW = dígito ligado (cátodo comum)
}

// ---------------------------------------------------------------------------
// setupSegDisplay
// ---------------------------------------------------------------------------
void Controller::setupSegDisplay() {
    // Configura pinos de segmento
    for (int i = 0; i < 7; i++) {
        pinMode(segPins[i], OUTPUT);
        digitalWrite(segPins[i], LOW);
    }
    // Configura pinos de seleção de dígito (HIGH = desligado no cátodo comum)
    for (int d = 0; d < SEG_DIGITS; d++) {
        pinMode(digPins[d], OUTPUT);
        digitalWrite(digPins[d], HIGH);
    }
    Serial.println(F("[DISPLAY] Pinos do 7 segmentos configurados."));
}


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

    Serial.println(F("\n|===================================="));
    Serial.println(F("|  CONSOLE DE DEBUG - FUZZY BLOCKS  |"));
    Serial.println(F("|===================================="));
    Serial.println(F("| 'c' -> Gerar pulso de Clock"));
    Serial.println(F("| 'r' -> Ler EEPROM (Posição 1)"));
    Serial.println(F("| 'w' -> Escrever na EEPROM (Posição 1)"));
    Serial.println(F("| 's' -> Alternar estado do pino SET"));
    Serial.println(F("| 'l' -> Sair / iniciar LISTENER"));
    Serial.println(F("| 't' -> Transmitir para o outro Arduino"));
    Serial.println(F("|____________________________________"));

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
    while (blocks_used < 100) {  // Overflow → ERR_HW_SHIFT_OVERFLOW (503)
        
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
    if (Wire.endTransmission() != 0) {
        Serial.println(F("[I2C] Erro 500: Falha na transmissao I2C (ERR_HW_I2C_FALHA)"));
    } else {
        Serial.println(F("[I2C] Transmissao para o Slave 8 concluida com sucesso!"));
    }
    Serial.println(F("=================================================="));
}

void Controller::ShowState(int stateIndex) {
    // Guarda de índice inválido
    if (stateIndex < 0 || stateIndex > 4) {
        Serial.print(F("[DISPLAY] Erro: indice invalido para ShowState: "));
        Serial.println(stateIndex);
        return;
    }

    // Exibe o padrão no dígito 1 (único dígito para estados simples)
    _writeDigit(0, letrasEstado[stateIndex]);

    Serial.print(F("[DISPLAY] ShowState -> indice "));
    Serial.println(stateIndex);
}

void Controller::ShowError(int errorCode) {
    Serial.print(F("[DISPLAY] ShowError -> codigo "));
    Serial.println(errorCode);

    // Decompoe o codigo em 3 digitos (maximo 999 para um codigo de 3 digitos)
    int cod = (errorCode >= 0 && errorCode <= 999) ? errorCode : 999;
    int centenas = cod / 100;
    int dezenas  = (cod % 100) / 10;
    int unidades = cod % 10;

    // Monta o array de padroes para os 4 digitos: [E, centenas, dezenas, unidades]
    const byte* padroes[SEG_DIGITS] = {
        letrasEstado[SEG_STATE_ERROR], // Digito 1: 'E'
        digitos[centenas],             // Digito 2: centenas
        digitos[dezenas],              // Digito 3: dezenas
        digitos[unidades]              // Digito 4: unidades
    };

    Serial.println(F("[DISPLAY] Aguardando botao para continuar..."));

    // Garante que o botao nao esteja ja pressionado antes de comecar a esperar
    while (digitalRead(PIN_BUTTON) == LOW) {
        // Continua multiplexando enquanto o botao permanece pressionado
        for (int d = 0; d < SEG_DIGITS; d++) {
            _writeDigit(d, padroes[d]);
            delay(SEG_MUX_DELAY_MS);
        }
    }

    // Espera o botao ser pressionado (LOW) — multiplexando o display enquanto isso
    while (digitalRead(PIN_BUTTON) == HIGH) {
        for (int d = 0; d < SEG_DIGITS; d++) {
            _writeDigit(d, padroes[d]);
            delay(SEG_MUX_DELAY_MS);
        }
    }

    // Espera o botao ser solto (debounce)
    while (digitalRead(PIN_BUTTON) == LOW) {
        for (int d = 0; d < SEG_DIGITS; d++) {
            _writeDigit(d, padroes[d]);
            delay(SEG_MUX_DELAY_MS);
        }
    }

    // Apaga todos os digitos ao sair
    for (int d = 0; d < SEG_DIGITS; d++) {
        digitalWrite(digPins[d], HIGH);
    }

    Serial.println(F("[DISPLAY] Botao pressionado. Retornando ao Debug."));
}