#include "Controller.h"
#include <tokens.h>
#include <evaluator.h>
#include <car_actuator.h>

Evaluator::Evaluator(int dados[], int tamanho, bool is_loop, Car* car){
   sequencia = dados;
   qtd_tokens = tamanho;
   pc = 0;
   sp = 0;
   this->is_loop = is_loop;
   this->carrinho = car; 
   this->start_time = millis();
}

unsigned long Evaluator::getSeconds() {
    return (millis() - this->start_time) / 1000;
}

int Evaluator::Casting(int token){
    int call_result = GARBAGE;

    switch (token){

    case _ZERO:
        call_result = 0;
        break;
    
    case _ONE:
        call_result = 1;
        break;

    case _FIVE:
        call_result = 5;
        break;

    case _FIFTY:
        call_result = 50;
        break;

    case _THOUSAND:
        call_result = 1000;
        break;

    case _TRUE:
        call_result = true;
        break;

    case _FALSE:
        call_result = false;
        break;

    case _SEGUNDOS:
        call_result = this->getSeconds();
        break;

    }

    return call_result;
}

int Evaluator::Calculator(int var_method, int oper, int val){
    bool result = false;

        if ( isMethod(var_method) ){
            var_method = this->CallMethod(var_method);
        }else{
            var_method = this->Casting(var_method); // Se for seconds, dentro do casting será esperado.
        }

        // Método não void já calculado
        if(oper == GARBAGE){
            result = var_method;
               
        // Expressão composta
        }else{
            // Convertendo valor
            if ( isMethod(val) ){
                val = this->CallMethod(val);
            }else{
                val = this->Casting(val);
            }

            // Comparação final
            switch (oper){
                case _SMALLER:
                    result = (var_method < val);
                    break;
                case _BIGGER:
                    result = (var_method > val);
                    break;
                case _EQUAL:
                    result = (var_method == val);
                    break;
            }     
        }

    return result;
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
            Serial.println(F("[EVALUATOR] Resultado do proximidade:"));
            Serial.println(result);
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
            delay(this->Casting(args[0]) * 1000);
            break;
    }

    return result;
}


void Evaluator::Eval(){
    static const int function_limit = 4;
    static const int condition_limit = 50;

    static int structure_args[function_limit] = {GARBAGE};
    static int condition_args[condition_limit] = {GARBAGE};
    static int expression_args[condition_limit] = {GARBAGE};
    static int solved_args[condition_limit] = {GARBAGE};
    static int function_code = GARBAGE;
    static int condition_code = GARBAGE;
    static int result = GARBAGE;
    
    static int structure_pointer = 0;
    static int condition_pointer = 0;
    static int expression_pointer = 0;
    static int solved_counter = 0;
    static int jumps = 0;
    
    static bool is_in_function = false;
    static bool is_in_condition = false;
    static bool move_forward = false;
    bool exists_and = false;
    int sum_args = 0;
    int value_args = 0;
    int token = sequencia[pc];

    if(!move_forward){
        Serial.println(F("[EVAL] [ TOKEN INIT ] --------------------------------------------------------------- "));
        Serial.print(F("[EVAL] [ Token: ")); Serial.print(token); Serial.print(F("  //  Segundos: ")); Serial.print(this->getSeconds()); Serial.println(F(" ]"));

        // Validador de funções 
        if(isFunction(token)){
            Serial.println(F(" [EVAL] [ FUNCTION INIT ] ----------------------------------------------------------- "));
            is_in_function = true;
            function_code = token;

        }else if(isEndFunction(token)){
            result = 0;
            is_in_function = false;

            Serial.print(F("[EVAL] Função acabou // Função ativa é a: ")); Serial.println(function_code);
            result = CallFunction(function_code, structure_args, structure_pointer);

            Serial.print(F("[EVAL] limpeza"));
            function_code = GARBAGE;
            for(int i = 0; i < structure_pointer ; i++){
                structure_args[i] = GARBAGE;
            }
            structure_pointer = 0;
            Serial.println(F("[EVAL] [ FUNCTION END ] ------------------------------------------------------------- "));
            
        } else if (is_in_function){
            Serial.println(F("[EVAL] Armazenando valor de função"));
            structure_args[structure_pointer] = token;
            structure_pointer++;
        }

        // Validador de condições
        if (isCondition(token)){
            Serial.println(F("[EVAL] [ CONDITION INIT ] ----------------------------------------------------------- "));
            is_in_condition = true;
            condition_code = token;
            sp = pc;
            
        }else if(isEndCondition(token)){
            result = 0;
            Serial.println(F("[EVAL] Condição acabou"));
            is_in_condition = false;

            // For de resolver expressão
            for(int j = 0; j < condition_pointer ; j++){
                
                // Se for bloco lógico ou fim da expressão
                if(isLogical(condition_args[j]) || (j == condition_pointer - 1)){
                    Serial.println(F("Acabou a expressão"));

                    // Se for último de todos da condição
                    if (j == condition_pointer - 1 && !isLogical(condition_args[j])) {
                        expression_args[expression_pointer] = condition_args[j];
                        expression_pointer++;
                    }

                    solved_args[solved_counter] = (expression_pointer == 1) 
                        ? this->Calculator(expression_args[0]) 
                        : this->Calculator(expression_args[0], expression_args[1], expression_args[2]);

                    solved_counter++;
                    
                    if(isLogical(condition_args[j])){ 
                        Serial.println(F("Acrescenta valor lógico no solved_args"));
                        solved_args[solved_counter] = condition_args[j];
                        solved_counter++;
                    }

                    for(int i = 0 ; i < expression_pointer ; i++) {
                        expression_args[i] = GARBAGE;
                    }
                    expression_pointer = 0; 

                }else{
                    Serial.println(F("Armazena enquanto não acabou a expressão"));
                    expression_args[expression_pointer] = condition_args[j];
                    expression_pointer++;
                }
            }

            Serial.println(F("solved-args: "));
            for(int m = 0; m < solved_counter ; m++){
                Serial.println(solved_args[m]);

                if(solved_args[m] == _AND){
                    exists_and = true;
                }

                if(solved_args[m] == 0 || solved_args[m] == 1){
                    sum_args += solved_args[m];
                    value_args++;
                }
            }

            // Limpeza Total da Condição
            // condition_code = GARBAGE;
            condition_pointer = 0;
            solved_counter = 0; 

            // Se existir AND, só de ter um falso já invalida tudo.
            if(exists_and && sum_args != value_args){
                move_forward = true;
            }else if(!exists_and && sum_args < 1){
                move_forward = true;
            }
            
            Serial.println(F("[EVAL] [ CONDITION END ] ------------------------------------------------------------ "));

            move_forward ? Serial.println(F("\n[EVAL] PASSA BATIDO NA CONDIÇÃO!")) : Serial.println(F("[EVAL] ENTRA NA CONDIÇÃO !"));;

        }else if (is_in_condition){
            Serial.println(F("[EVAL] Armazenando valor de condição"));
            condition_args[condition_pointer] = token;
            condition_pointer++;
        }

        // Executores de métodos void
        if(isVoidMethod(token)){
            result = this->CallMethod(token);
        }
    }

    // Controle do ponteiro de execução
    if(pc == (qtd_tokens - 1)){
        if(this->is_loop){
            pc = 0;
        }else{
            run = false;
        }
    }else{
        pc++;

        // Se for fim de condição e for realmente o fechamento da condição raiz
        if (isEndBlock(token) && jumps == 0){

            // Se tava pulando (condição falsa), não volte mais
            if(move_forward){
                move_forward = false;
            } else{
                // Se for WHILE não tava pulando, por enquanto é verdade, volte.
                if(condition_code == _WHILE){
                    pc = sp;
                }
            }
        }

        // Se achar alguma condição aninhada, e estiver pulando
        if(move_forward){
            if (isCondition(token)){
                jumps++;
            }

            if (isEndBlock(token)){
                jumps--;
            }
        }
    }   
}