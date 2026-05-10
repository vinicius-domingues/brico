#include "Controller.h"
#include <tokens.h>
#include <evaluator.h>
#include <car_actuator.h>

Evaluator::Evaluator(int dados[], int tamanho, bool is_loop, Car* car){
   sequencia = dados;
   qtd_tokens = tamanho;
   pc = 0;
   this->is_loop = is_loop;
   this->carrinho = car; 
}

int Evaluator::CallMethod(int method_code, int args[], int size) {
    int result = GARBAGE;

    switch (method_code) {
        case _BRAKE:
            Serial.println(F("[EVALUATOR] Acao ativada: Frear"));
            this->carrinho->Brake();
            break;
            
        case _ACCELERATE:
            Serial.println(F("[EVALUATOR] Acao ativada: Acelerar"));
            this->carrinho->Accelerate();
            break;
            
        case _HONK:
            Serial.println(F("[EVALUATOR] Acao ativada: Buzinar"));
            this->carrinho->Honk();
            break;
            
        case _RED_LED:
            Serial.println(F("[EVALUATOR] Acao ativada: LED Vermelho"));
            this->carrinho->RedLed();
            break;
            
        case _GREEN_LED:
            Serial.println(F("[EVALUATOR] Acao ativada: LED Verde"));
            this->carrinho->GreenLed();
            break;
            
        case _BLUE_LED:
            Serial.println(F("[EVALUATOR] Acao ativada: LED Azul"));
            this->carrinho->BlueLed();
            break;

        case _PROXIMITY:
            Serial.println(F("[EVALUATOR] Acao ativada: Proximidade"));
            result = this->carrinho->Proximity();
            break;

        default:
            Serial.print(F("[EVALUATOR] Aviso: Token ignorado ou nao mapeado para acao fisica - "));
            Serial.println(method_code);
            break;
    }

    return result;
}


int Evaluator::CallFunction(int function_code, int args[], int size){
    int function_value = GARBAGE;
    int result = GARBAGE;

    switch(function_code){
        case _DELAY:
            Serial.println("[EXEC] Delay ativado.");

            switch (args[0]){
                case _ZERO:     function_value = 0;    break;
                case _ONE:      function_value = 1;    break;
                case _FIVE:     function_value = 5;    break;
                case _FIFTY:    function_value = 50;   break;
                case _THOUSAND: function_value = 1000; break;
            }
            delay(function_value * 1000);

            break;
    }

    return result;
}


void Evaluator::Eval(unsigned long segundos){
    static const int function_limit = 4;
    static const int condition_limit = 50;
    static int function_args[function_limit] = {GARBAGE};
    static int condition_args[condition_limit] = {GARBAGE};
    static int function_code = GARBAGE;
    static int condition_code = GARBAGE;
    static int result = GARBAGE;
    static int function_pointer = 0;
    static int condition_pointer = 0;
    static bool is_in_function = false;
    static bool is_in_condition = false;
    int token = sequencia[pc];
    bool error_flag = false;

    Serial.println("[EVAL] [ TOKEN INIT ] --------------------------------------------------------------- ");
    Serial.print("[EVAL] [ Token: "); Serial.print(token); Serial.print("  //  Segundos: "); Serial.print(segundos); Serial.println(" ]");

    // Validador de funções 
    // Valida o começo da função, pegando qual função é e ativando a flag 'is_in_function'
    if(isFunction(token)){
        Serial.println(" [EVAL] [ FUNCTION INIT ] --------------------------------------------------------------- ");
        is_in_function = true;
        function_code = token;

    // Se estiver em função, começa a armazenar os argumentos (máx 4) e valida se há estouro de argumentos (stack overflow)
    }else if(isEndFunction(token)){
        result = 0;
        Serial.println("[EVAL] viu que função acabou");
        is_in_function = false;

        // Executor de funções
        Serial.print("[EVAL] função acabou e a função ativa é a: "); Serial.println(function_code);
        result = CallFunction(function_code, function_args, function_pointer);

        // Limpeza para seguir o código
        Serial.print("[EVAL] limpeza");
        function_code = GARBAGE;
        for(int i = 0; i < function_pointer ; i++){
            function_args[i] = GARBAGE;
        }
        function_pointer = 0;
        Serial.println(" [EVAL] [ FUNCTION END ] --------------------------------------------------------------- ");
    }else if (is_in_function){
        Serial.println("[EVAL] viu que tá dentro da função, armazenou");
        function_args[function_pointer] = token;
        function_pointer++;
    // Ao final da função, tira da flag de 'is_in_function' e começa a executar pois já armazenou todos os argumentos
    }

    // Validador de condições
        
    // Executores de métodos void (Só passa o token e retorna GARBAGE)
    if(isVoidMethod(token)){
        result = this->CallMethod(token);
    }
    
    // Coordenador: Se o ponteiro estiver no último item lido e ainda não bateu em nenhum _END, é porque é LOOP, então volta para o começo (0)
    if(pc == (qtd_tokens - 1)){
        if(this->is_loop){
            pc = 0;
        }else{
            run = false;
        }
    // Se não está no fim, avança ponteiro
    }else{
        pc++;
    }

    Serial.println("[EVAL] [ TOKEN END ] --------------------------------------------------------------- ");
}
