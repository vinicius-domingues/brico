#ifndef SYNTAX_H
#define SYNTAX_H

#include <Arduino.h>


class Syntax {
    public:
        int result = 0; // success
        int* tokens;
        int total;
        int error_position;

        //Syntax();
        bool Parser(int sequence[], int blocks_used);
        bool LookAhead(int sequence[], int blocks_used);
        bool Semantic(int sequence[], int blocks_used);
        bool ExpressionValidator(int sequence_in_expression[], int positions);
        bool FunctionValidator(int function_code, int function_args[], int tamanho);
};

#endif