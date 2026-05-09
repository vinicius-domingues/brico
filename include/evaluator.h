#ifndef EVALUATOR_H

  #define EVALUATOR_H

  #include <Arduino.h>
  #include "car_actuator.h"

  class Evaluator{
    public:
      int* sequencia;
      int pc; // Ponteiro de execução
      int qtd_tokens;
      bool is_loop;
      bool run = true;
      Car* carrinho;
    
      Evaluator(int dados[], int tamanho, bool is_loop, Car* car);
      void Eval(unsigned long seconds);
      void CallMethod(int token);
      bool CallFunction(int function_code, int function_args[], int tamanho);
  };

#endif