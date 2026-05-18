#ifndef SYNTAX_H
#define SYNTAX_H

#include <Arduino.h>

// Callback chamado durante a análise para animar os LEDs dos blocos físicos
typedef void (*LedCallback)(int blockIndex, byte color);

class Syntax {
    public:
        int result = 0;
        int* tokens;
        int total;
        int error_position = -1;
        LedCallback onBlockLed = nullptr; // Definir em main.cpp

        //Syntax();
        bool Parser(int sequence[], int blocks_used);
        bool LookAhead(int sequence[], int blocks_used);
        bool Semantic(int sequence[], int blocks_used);
        bool ExpressionValidator(int sequence_in_expression[], int positions);
        bool FunctionValidator(int function_code, int function_args[], int tamanho);
};

#endif