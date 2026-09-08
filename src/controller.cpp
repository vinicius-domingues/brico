#include "Controller.h"
#include <tokens.h>
#include <errors.h>






Controller::Controller() {
    Wire.begin(); 
    pinMode(PIN_SET, OUTPUT);
    pinMode(PIN_CLOCK, OUTPUT);
    pinMode(PIN_BUTTON, INPUT_PULLUP);
    pinMode(PIN_DATA_IN, INPUT_PULLDOWN); // Pino de leitura do protocolo bit-bang
    
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

// ---------------------------------------------------------------------------
// _waitIdleHigh — Auxiliar do Mapper()
// Aguarda o pino PIN_DATA_IN entrar em estado HIGH (idle) antes de receber.
// Retorna false se o timeout expirar sem que a linha estabilize em HIGH.
// ---------------------------------------------------------------------------
bool Controller::_waitIdleHigh(uint32_t timeoutMs) {
    const uint32_t start = millis();

    while (digitalRead(PIN_DATA_IN) == LOW) {
        if ((millis() - start) >= timeoutMs) {
            return false;
        }
    }

    // Confirma que a linha permaneceu em repouso HIGH.
    delay(50);
    return digitalRead(PIN_DATA_IN) == HIGH;
}

// ---------------------------------------------------------------------------
// _receiveByte — Auxiliar do Mapper()
// Decodifica um byte do protocolo bit-bang:
//   - Aguarda start bit (HIGH→LOW)
//   - Amostra o meio de cada bit (8 bits, LSB-first)
//   - Verifica stop bit (deve ser HIGH)
// ---------------------------------------------------------------------------
bool Controller::_receiveByte(uint8_t& data, uint32_t timeoutMs) {
    const uint32_t start = millis();
    uint8_t received = 0;

    // Aguarda o start bit em LOW.
    while (digitalRead(PIN_DATA_IN) == HIGH) {
        if ((millis() - start) >= timeoutMs) {
            return false;
        }
    }

    // Vai para o centro do start bit.
    delayMicroseconds(BITBANG_BIT_TIME_US / 2);

    if (digitalRead(PIN_DATA_IN) != LOW) {
        return false;
    }

    // Vai para o centro do primeiro bit de dados.
    delayMicroseconds(BITBANG_BIT_TIME_US);

    // Recebe 8 bits, do menos significativo para o mais significativo.
    for (uint8_t bit = 0; bit < 8; bit++) {
        if (digitalRead(PIN_DATA_IN) == HIGH) {
            received |= static_cast<uint8_t>(1U << bit);
        }

        delayMicroseconds(BITBANG_BIT_TIME_US);
    }

    // Confere o stop bit.
    if (digitalRead(PIN_DATA_IN) != HIGH) {
        return false;
    }

    data = received;
    return true;
}

// ---------------------------------------------------------------------------
// _checksum — Auxiliar do Mapper()
// Calcula o checksum XOR do pacote: count ^ ids[0] ^ ids[1] ^ ... ^ ids[n-1]
// ---------------------------------------------------------------------------
uint8_t Controller::_checksum(const ProtocolPacket& packet) {
    uint8_t value = packet.count;

    for (uint8_t i = 0; i < packet.count; i++) {
        value ^= packet.ids[i];
    }

    return value;
}

// ---------------------------------------------------------------------------
// _receivePacket — Auxiliar do Mapper()
// Recebe um frame completo do protocolo:
//   1. Varre bytes até encontrar o SOF (0xA5)
//   2. Lê o count (quantidade de blocos)
//   3. Lê cada id[] do pacote
//   4. Lê e valida o checksum XOR
// ---------------------------------------------------------------------------
bool Controller::_receivePacket(ProtocolPacket& packet) {
    const uint32_t frameStart = millis();
    uint8_t value = 0;

    // Procura o byte inicial 0xA5.
    do {
        const uint32_t elapsed = millis() - frameStart;

        if (elapsed >= BITBANG_FRAME_TIMEOUT_MS) {
            return false;
        }

        if (!_receiveByte(value, BITBANG_FRAME_TIMEOUT_MS - elapsed)) {
            return false;
        }
    } while (value != BITBANG_SOF);

    if (!_receiveByte(packet.count, BITBANG_BYTE_TIMEOUT_MS)) {
        return false;
    }

    if ((packet.count == 0) || (packet.count > BITBANG_MAX_IDS)) {
        return false;
    }

    for (uint8_t i = 0; i < packet.count; i++) {
        if (!_receiveByte(packet.ids[i], BITBANG_BYTE_TIMEOUT_MS)) {
            return false;
        }
    }

    if (!_receiveByte(value, BITBANG_BYTE_TIMEOUT_MS)) {
        return false;
    }

    return value == _checksum(packet);
}

// ---------------------------------------------------------------------------
// Mapper
// Recebe a sequência de blocos via protocolo bit-bang no pino PIN_DATA_IN.
// Popula sequence[] com os ids recebidos e atualiza blocks_used.
// A transmissão para o carrinho é feita separadamente via STATE_UART (main.cpp).
// ---------------------------------------------------------------------------
void Controller::Mapper(int sequence[], int& blocks_used) {
    Serial.println(F("\n[MAPPER] Iniciando recepcao via protocolo bit-bang..."));

    blocks_used = 0;
    got_error   = false;
    is_loop     = false;

    // 1. Aguarda a linha estabilizar em HIGH antes de receber
    Serial.println(F("[MAPPER] Aguardando linha em idle HIGH..."));
    if (!_waitIdleHigh(BITBANG_LINE_TIMEOUT_MS)) {
        Serial.println(F("[MAPPER] ERRO: timeout aguardando idle HIGH na linha de dados."));
        got_error = true;
        return;
    }

    // 2. Recebe o pacote completo (SOF + count + ids[] + checksum)
    ProtocolPacket packet{};
    if (!_receivePacket(packet)) {
        Serial.println(F("[MAPPER] ERRO: pacote ausente ou invalido."));
        got_error = true;
        return;
    }

    // 3. Popula sequence[] com os ids recebidos (uint8_t → int).
    //    Comportamento idêntico ao Mapper legado:
    //      - Sempre injeta _START obrigatório no início.
    //      - Se o último id do pacote for _END (255) → a fita termina com _END
    //        (programa com fim definido, sem loop).
    //      - Se o último id NÃO for _END → injeta _START no final
    //        (programa em loop, igual ao "fechador da fita virtual" do Mapper antigo).
    sequence[0] = _START;
    Serial.println(F("[MAPPER] [INJECAO] _START (1) adicionado obrigatoriamente no inicio."));

    for (uint8_t i = 0; i < packet.count; i++) {
        sequence[i + 1] = static_cast<int>(packet.ids[i]);
    }

    bool ultimo_e_end = (packet.ids[packet.count - 1] == _END);

    if (ultimo_e_end) {
        // Transmissor sinalizou fim — mantém _END como fechador da fita
        blocks_used = static_cast<int>(packet.count) + 1; // +1 pelo _START inicial
        Serial.println(F("[MAPPER] Ultimo token e _END (255). Fita com FIM DEFINIDO (sem loop)."));
    } else {
        // Sem _END → injeta _START no final como fechador (programa em loop)
        sequence[packet.count + 1] = _START;
        blocks_used = static_cast<int>(packet.count) + 2; // +2 pelos dois _START
        Serial.println(F("[MAPPER] [INJECAO] _START (1) adicionado como FIM da fita (programa em LOOP)."));
    }



    // 4. Debug: imprime a fita de tokens recebida na RAM
    Serial.println(F("=================================================="));
    Serial.print(F("[MAPPER] FITA FINAL DE TOKENS NA RAM: "));
    for (int i = 0; i < blocks_used; i++) {
        Serial.print(sequence[i]);
        if (i < blocks_used - 1) Serial.print(F(" - "));
    }
    Serial.println();
    Serial.println(F("=================================================="));
}



void Controller::ShowState(int stateIndex) {
    Serial.print(F("[STATE] "));
    switch (stateIndex) {
        case SEG_STATE_DEBUG:     Serial.println(F("DEBUG"));      break;
        case SEG_STATE_COMPILE:   Serial.println(F("COMPILANDO")); break;
        case SEG_STATE_RUNNING:   Serial.println(F("RUNNING"));    break;
        case SEG_STATE_ERROR:     Serial.println(F("ERROR"));      break;
        case SEG_STATE_LISTENING: Serial.println(F("LISTENING"));  break;
        default:                  Serial.println(stateIndex);      break;
    }
}

void Controller::ShowError(int errorCode) {
    Serial.print(F("[ERROR] Codigo: "));
    Serial.println(errorCode);
    Serial.println(F("[ERROR] Aperte o botao para retomar."));

    // Garante que o botao nao esteja ja pressionado
    while (digitalRead(PIN_BUTTON) == LOW) { delay(10); }

    // Espera o botao ser pressionado
    while (digitalRead(PIN_BUTTON) == HIGH) { delay(10); }

    // Espera o botao ser solto (debounce)
    while (digitalRead(PIN_BUTTON) == LOW)  { delay(10); }

    Serial.println(F("[ERROR] Botao pressionado. Retomando."));
}

void Controller::ResetBlockLeds() {
    for (int i = 0; i < BLOCK_LED_MAX; i++) blockLedState[i] = BLOCK_COLOR_OFF;
    FlushBlockLeds();
    Serial.println(F("[BLOCK_LED] LEDs resetados."));
}

void Controller::IlluminateBlock(int blockIndex, byte color) {
    if (blockIndex == -1) {
        // Erro global: tudo vermelho
        for (int i = 0; i < BLOCK_LED_MAX; i++) blockLedState[i] = BLOCK_COLOR_RED;
    } else if (blockIndex >= 0 && blockIndex < BLOCK_LED_MAX) {
        blockLedState[blockIndex] = color;
    }
    FlushBlockLeds();
}

void Controller::FlushBlockLeds() {
    // Cada bloco ocupa 2 bits: [RED_EN | GREEN_EN]
    // BLOCK_COLOR_OFF    = 00, GREEN = 01, YELLOW = 11, RED = 10
    // Total de bits: BLOCK_LED_MAX * 2  (enviados via shift register)

#if (PIN_BLOCK_LED_DATA != -1) && (PIN_BLOCK_LED_CLOCK != -1) && (PIN_BLOCK_LED_LATCH != -1)
    digitalWrite(PIN_BLOCK_LED_LATCH, LOW);
    // Envia do bloco mais distante para o mais proximo (shift register empilha)
    for (int b = BLOCK_LED_MAX - 1; b >= 0; b--) {
        byte green_bit = 0, red_bit = 0;
        switch (blockLedState[b]) {
            case BLOCK_COLOR_GREEN:  green_bit = 1; red_bit = 0; break;
            case BLOCK_COLOR_YELLOW: green_bit = 1; red_bit = 1; break;
            case BLOCK_COLOR_RED:    green_bit = 0; red_bit = 1; break;
            default:                 green_bit = 0; red_bit = 0; break;
        }
        // Envia red_bit, depois green_bit
        for (int bit = 1; bit >= 0; bit--) {
            byte out = (bit == 1) ? red_bit : green_bit;
            digitalWrite(PIN_BLOCK_LED_DATA, out ? HIGH : LOW);
            digitalWrite(PIN_BLOCK_LED_CLOCK, HIGH); delayMicroseconds(5);
            digitalWrite(PIN_BLOCK_LED_CLOCK, LOW);  delayMicroseconds(5);
        }
    }
    digitalWrite(PIN_BLOCK_LED_LATCH, HIGH);
#else
    // STUB: pinos nao definidos ainda em tokens.h
    // (sem saida Serial para nao poluir o log de compilacao token a token)
#endif
}
