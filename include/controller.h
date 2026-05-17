#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <Arduino.h>
#include <Wire.h>



class Controller{
  public:
    //static const int blocks_limit = 50;
    //int sequence[blocks_limit];
    //int blocks_read = 0;
    bool got_error = false;
    bool is_loop = false;

    Controller();     // Construtor
    void DebugMenu(); // Função de debug
      void writeEEPROM(int address, byte data); // Grava na EEPROM (a mando do debug menu)
    void Listener();  // Espera botão
    void Mapper(int sequence[], int& blocks_used);    // Mapeia todas as peças (forma a sequência, começa vazio)
    void Clock();     // Dá clock
    void Prepare();   // Prepara o shift register
    void transmitI2C(byte slaveAddress); // Comunica com o atuador
    void Stop();      // Só para o código naquele trecho para sempre
    void setupSegDisplay();         // Configura os pinos do display 7 segmentos como OUTPUT
    void ShowState(int stateIndex); // Exibe uma letra no display 7 segmentos
    void ShowError(int errorCode);  // Exibe 'E' + código (3 dígitos) e aguarda botão

  private:
    byte readEEPROM(int address); // Le a EEPROM (a mando do debug menu)
};

#endif