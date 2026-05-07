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

bool Evaluator::CallFunction(int function_code, int function_args[], int tamanho){
    int function_value = GARBAGE;
    bool error_flag = false;

    if(function_code == _DELAY){
        Serial.print("[EVAL] function_code == _DELAY");
        // Valida tipos
        if(isNumberValue(function_args[0])){
            if(function_args[0] == _ZERO){
                function_value = 0;
            }else if(function_args[0] == _ONE){
                function_value = 1;
            }
            else if(function_args[0] == _FIVE){
                function_value = 5;
            }
            else if(function_args[0] == _FIFTY){
                function_value = 50;
            }
            else if(function_args[0] == _THOUSAND){
                function_value = 1000;
            }

            // Se não der erro, executa
            if(!error_flag){
                // delay espera ms, então multiplicamos por 1000
                function_value *= 1000;
                Serial.print("[EVAL] Parando por (ms): "); Serial.println(function_value);
                delay(function_value);
            }
        }
        else{
                Serial.print("[EVAL] Erro, valor não é numérico");
                error_flag = true;
            }



    }

    return error_flag;
}
 


void Evaluator::CallMethod(int token) {
    switch (token) {
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
            
        default:
            Serial.print(F("[EVALUATOR] Aviso: Token ignorado ou nao mapeado para acao fisica - "));
            Serial.println(token);
            break;
    }
}

void Evaluator::Eval(unsigned long segundos){
    Serial.println(" li ---------------------------------------------------------------------------------------------------------------------------- ");
    int token = sequencia[pc];
    bool error_flag = false;

    static int function_args[] = {GARBAGE, GARBAGE, GARBAGE, GARBAGE};
    static int function_code = GARBAGE;
    static int function_pointer = 0;
    static int function_value = GARBAGE;
    static bool is_in_function = false;
    //static bool is_in_condition = false;

    Serial.println(" ");
    Serial.print("[EVAL] Token: "); Serial.println(token);
    Serial.print("[EVAL] Segundos: "); Serial.println(segundos);

    // Validador de funções --------------------------------------------------------------------------------------------------------
    // Valida o começo da função, pegando qual função é e ativando a flag 'is_in_function'
    Serial.println(" fi ------------------------------------------");
    if(isFunction(token)){
        Serial.println("[EVAL] viu que é função");
        is_in_function = true;
        function_code = token;

    // Se estiver em função, começa a armazenar os argumentos (máx 4) e valida se há estouro de argumentos (stack overflow)
    }else if(isEndFunction(token)){
        Serial.println("[EVAL] viu que função acabou");
        is_in_function = false;

        Serial.print("[EVAL] função acabou e a função ativa é a: "); Serial.println(function_code);

        // Executor de funções (Valida tipos + Executa)
        error_flag = this->CallFunction(function_code, function_args, function_pointer);


        // Limpeza para seguir o código
        Serial.print("[EVAL] limpeza");
        function_value = GARBAGE;
        function_code = GARBAGE;
        for(int i = 0; i < function_pointer ; i++){
            function_args[i] = GARBAGE;
        }
        function_pointer = 0;
    }else if (is_in_function){
        Serial.println("[EVAL] viu que tá dentro da função, armazenou");
        function_args[function_pointer] = token;

        // Valida estouro de argumentos
        if(function_pointer + 1 == 5){
            Serial.print("[EVAL] Erro, estouro de argumentos (maior que 4)");
            error_flag = true;
        }else{
            function_pointer++;
        }
        
    // Ao final da função, tira da flag de 'is_in_function' e começa a executar pois já armazenou todos os argumentos
    }
    Serial.println(" ff ------------------------------------------");


    // Validador de condições
    



    // Executores de métodos void
    if (isVoidMethod(token)){
        this->CallMethod(token);
    }

    // Coordenador: Se o ponteiro estiver no último item lido e ainda não bateu em nenhum _END, é porque é LOOP, então volta para o começo
    if(pc == (qtd_tokens - 1)){
        if(this->is_loop){
            pc = 0;
        }else{
            run = false;
        }
        
    }else{
        pc++;
    }

    // Apenas checa o for de argumentos (debug)
    for(int i = 0; i < 4 ; i++){
        Serial.print("[EVAL] contador: "); Serial.println(i);
        Serial.print("[EVAL] valor: "); Serial.println(function_args[i]);
    }

    // O erro aqui apenas faz parar de rodar
    if(error_flag){
        this->run = false;
    }
    Serial.println(" lf ---------------------------------------------------------------------------------------------------------------------------- ");
}

