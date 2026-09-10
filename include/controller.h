#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <Arduino.h>
#include <Wire.h>
#include <tokens.h>


// Estrutura de pacote do protocolo bit-bang — usada pelo Mapper() e seus auxiliares
struct ProtocolPacket {
    uint8_t count;
    uint8_t ids[BITBANG_MAX_IDS];
};

class Controller{
  public:
    //static const int blocks_limit = 50;
    //int sequence[blocks_limit];
    //int blocks_read = 0;
    bool got_error = false;
    bool is_loop = false;

    Controller();     // Construtor
    void writeEEPROM(int address, byte data); // Grava na EEPROM
    void Listener();  // Espera botão
    void Mapper(int sequence[], int& blocks_used);    // Mapeia todas as peças (forma a sequência, começa vazio)
    void transmitI2C(byte slaveAddress); // Comunica com o atuador
    void Stop();      // Só para o código naquele trecho para sempre
    void ShowError(int errorCode);  // Exibe 'E' + código (3 dígitos) e aguarda botão
    void IlluminateBlock(int blockIndex, byte color); // Atualiza cor de um bloco e envia ao hardware
    void ResetBlockLeds();           // Apaga todos os LEDs dos blocos

  private:
    byte readEEPROM(int address);
    void FlushBlockLeds();           // Envia blockLedState[] ao hardware dos LEDs dos blocos
    byte blockLedState[BLOCK_LED_MAX]; // Estado atual de cor de cada bloco

    // --- Auxiliares do Mapper() — protocolo bit-bang ---
    // Aguarda o pino PIN_DATA_IN entrar em estado HIGH (idle) antes de receber
    bool _waitIdleHigh(uint32_t timeoutMs);
    // Decodifica um byte via protocolo bit-bang (start bit + 8 bits LSB-first + stop bit)
    bool _receiveByte(uint8_t& data, uint32_t timeoutMs);
    // Calcula o checksum XOR do pacote (count ^ ids[0] ^ ... ^ ids[n-1])
    uint8_t _checksum(const ProtocolPacket& packet);
    // Recebe um frame completo: aguarda SOF, lê count, ids[] e valida checksum
    bool _receivePacket(ProtocolPacket& packet);
};

#endif