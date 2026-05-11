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
      unsigned long start_time;
      Car* carrinho;
    
      Evaluator(int dados[], int tamanho, bool is_loop, Car* car);
      void Eval();
      int CallMethod(int method_code, int args[] = nullptr, int size = GARBAGE);
      int CallFunction(int function_code, int args[] = nullptr, int size = GARBAGE);
      int Calculator(int var, int oper = GARBAGE, int val = GARBAGE);
      int Casting(int token);
      unsigned long getSeconds();
  };

#endif