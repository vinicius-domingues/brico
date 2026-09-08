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
            var_method = this->Casting(var_method);
        }

        if(oper == GARBAGE){
            result = var_method;
               
        }else{
            if ( isMethod(val) ){
                val = this->CallMethod(val);
            }else{
                val = this->Casting(val);
            }

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
                case _NOT:
                    result = (var_method != val);
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
    // Trava de seguranca: Nao ler alem da quantidade de tokens recebidos
    if (this->pc >= this->qtd_tokens || this->qtd_tokens <= 0) {
        Serial.print(F("[EVAL] Fim da sequencia atingido (PC: "));
        Serial.print(this->pc);
        Serial.print(F(" / Total: "));
        Serial.print(this->qtd_tokens);
        Serial.println(F("). Finalizando execucao."));
        if (this->carrinho != nullptr) {
            this->carrinho->Brake();
        }
        this->run = false;
        return;
    }

    // Trava de seguranca: Se passar de 2 minutos (120 segundos), para o carrinho e desliga a execucao
    static const unsigned long MAX_EXECUTION_SECONDS = 120;
    if (this->getSeconds() >= MAX_EXECUTION_SECONDS) {
        Serial.println(F("\n[EVAL] [TIMEOUT] Limite maximo de 2 minutos alcancado! Parando carrinho."));
        if (this->carrinho != nullptr) {
            this->carrinho->Brake();
        }
        this->run = false;
        return;
    }

    static const int function_limit = 4;
    static const int condition_limit = 40;

    static int structure_args[function_limit] = {GARBAGE};
    static int condition_args[condition_limit] = {GARBAGE};
    static int expression_args[condition_limit] = {GARBAGE};
    static int solved_args[condition_limit] = {GARBAGE};

    // Pilha de retorno: guarda PC e tipo de condiÃ§Ã£o de cada nÃ­vel aninhado
    static int condition_comeback_pc[condition_limit]   = {GARBAGE};
    static int condition_comeback_code[condition_limit] = {GARBAGE};
    static int repetition_counter = 0;

    static int function_code = GARBAGE;
    static int result = GARBAGE;
    
    static int structure_pointer = 0;
    static int condition_pointer = 0;
    static int expression_pointer = 0;
    static int solved_counter = 0;
    // move_forward_depth: 0 = executando normalmente
    // >0 = pulando N nÃ­veis de bloco (cada WHILE/IF falso empilha +1, cada ENDBLOCK desempilha -1)
    static int move_forward_depth = 0;
    
    static bool is_in_function = false;
    static bool is_in_condition = false;
    bool exists_and = false;
    int sum_args = 0;
    int value_args = 0;
    int token = sequencia[pc];

    if(move_forward_depth == 0){
        Serial.println(F("[EVAL] [ TOKEN INIT ] --------------------------------------------------------------- "));
        Serial.print(F("[EVAL] [ Token: ")); Serial.print(token); Serial.print(F("  //  Segundos: ")); Serial.print(this->getSeconds()); Serial.println(F(" ]"));

        // Validador de funÃ§Ãµes 
        if(isFunction(token)){
            Serial.println(F("[EVAL] [ FUNCTION INIT ] ----------------------------------------------------------- "));
            is_in_function = true;
            function_code = token;

        }else if(isEndFunction(token)){
            result = 0;
            is_in_function = false;

            Serial.print(F("[EVAL] FunÃ§Ã£o acabou // FunÃ§Ã£o ativa Ã© a: ")); Serial.println(function_code);
            result = CallFunction(function_code, structure_args, structure_pointer);

            function_code = GARBAGE;
            for(int i = 0; i < structure_pointer ; i++){
                structure_args[i] = GARBAGE;
            }
            structure_pointer = 0;
            Serial.println(F("[EVAL] [ FUNCTION END ] ------------------------------------------------------------- "));
            
        } else if (is_in_function){
            Serial.println(F("[EVAL] Armazenando valor de funÃ§Ã£o"));
            structure_args[structure_pointer] = token;
            structure_pointer++;
        }

        // Validador de condiÃ§Ãµes
        if (isCondition(token)){
            Serial.println(F("[EVAL] [ CONDITION INIT ] ----------------------------------------------------------- "));
            is_in_condition = true;

            // Salva PC e tipo da condiÃ§Ã£o na pilha
            condition_comeback_pc[repetition_counter]   = pc;
            condition_comeback_code[repetition_counter] = token;
            repetition_counter++;
            Serial.print(F("[EVAL] Salvando PC de retorno: "));
            Serial.println(pc);
            
        }else if(isEndCondition(token)){
            result = 0;
            Serial.println(F("[EVAL] CondiÃ§Ã£o acabou"));
            is_in_condition = false;

            for(int j = 0; j < condition_pointer ; j++){
                
                if(isLogical(condition_args[j]) || (j == condition_pointer - 1)){
                    Serial.println(F("Acabou a expressÃ£o"));

                    if (j == condition_pointer - 1 && !isLogical(condition_args[j])) {
                        expression_args[expression_pointer] = condition_args[j];
                        expression_pointer++;
                    }

                    solved_args[solved_counter] = (expression_pointer == 1) 
                        ? this->Calculator(expression_args[0]) 
                        : this->Calculator(expression_args[0], expression_args[1], expression_args[2]);

                    solved_counter++;
                    
                    if(isLogical(condition_args[j])){ 
                        Serial.println(F("Acrescenta valor lÃ³gico no solved_args"));
                        solved_args[solved_counter] = condition_args[j];
                        solved_counter++;
                    }

                    for(int i = 0 ; i < expression_pointer ; i++) {
                        expression_args[i] = GARBAGE;
                    }
                    expression_pointer = 0; 

                }else{
                    Serial.println(F("Armazena enquanto nÃ£o acabou a expressÃ£o"));
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

            condition_pointer = 0;
            solved_counter = 0; 

            // Se condiÃ§Ã£o FALSA: incrementa depth para comeÃ§ar a pular este bloco
            bool condition_false = false;
            if(exists_and && sum_args != value_args){ condition_false = true; }
            else if(!exists_and && sum_args < 1){     condition_false = true; }

            if(condition_false){
                move_forward_depth++;
                repetition_counter--;
                condition_comeback_pc[repetition_counter]   = GARBAGE;
                condition_comeback_code[repetition_counter] = GARBAGE;
                Serial.print(F("[EVAL] CondiÃ§Ã£o falsa â€” pulando bloco, depth: "));
                Serial.println(move_forward_depth);
            }

            Serial.println(F("[EVAL] [ CONDITION END ] ------------------------------------------------------------ "));
            Serial.print(F("[EVAL] move_forward_depth: ")); Serial.println(move_forward_depth);
            
        }else if (is_in_condition){
            Serial.println(F("[EVAL] Armazenando valor de condiÃ§Ã£o"));
            condition_args[condition_pointer] = token;
            condition_pointer++;
        }

        // Executores de mÃ©todos void
        if(isVoidMethod(token)){
            result = this->CallMethod(token);
        }
    }

    // Controle do ponteiro de execuÃ§Ã£o
    if((token == _START && pc > 0) || (token == _END)){
        if(token == _START){
            Serial.println(F("[EVAL] Loop: reiniciando execucao do inicio."));

            // Reseta PC
            pc = 0;

            // Reseta pilha de condiÃ§Ãµes/WHILE
            for(int i = 0; i < condition_limit; i++){
                condition_comeback_pc[i]   = GARBAGE;
                condition_comeback_code[i] = GARBAGE;
                condition_args[i]          = GARBAGE;
                expression_args[i]         = GARBAGE;
                solved_args[i]             = GARBAGE;
            }
            repetition_counter = 0;

            // Reseta contexto de funÃ§Ã£o
            for(int i = 0; i < function_limit; i++){
                structure_args[i] = GARBAGE;
            }
            function_code     = GARBAGE;
            structure_pointer = 0;

            // Reseta ponteiros e contadores de condiÃ§Ã£o
            condition_pointer  = 0;
            expression_pointer = 0;
            solved_counter     = 0;

            // Reseta flags e depth
            move_forward_depth = 0;
            is_in_function     = false;
            is_in_condition    = false;

        }else{
            run = false;
        }
    }else{
        Serial.print(F("[EVAL] Incrementando PC: ")); Serial.println(pc);
        pc++;

        // Enquanto pulando (depth > 0): rastreia blocos aninhados para nÃ£o sair cedo
        if(move_forward_depth > 0){
            if(isCondition(token)){
                move_forward_depth++; // bloco aninhado dentro do bloco que estamos pulando
                Serial.print(F("[EVAL] Bloco aninhado encontrado ao pular, depth: "));
                Serial.println(move_forward_depth);
            }
            if(isEndBlock(token)){
                move_forward_depth--;
                Serial.print(F("[EVAL] ENDBLOCK ao pular, depth agora: "));
                Serial.println(move_forward_depth);
                // Se chegou a 0, saÃ­mos do bloco que estava sendo pulado â€” continua normalmente
            }
        } else {
            // Executando normalmente â€” trata ENDBLOCK
            if(isEndBlock(token)){
                // Consulta o topo da pilha para saber o tipo da condiÃ§Ã£o deste bloco
                if(condition_comeback_code[repetition_counter - 1] == _WHILE){
                    Serial.print(F("[EVAL] WHILE verdadeiro â€” voltando para PC: "));
                    Serial.println(condition_comeback_pc[repetition_counter - 1]);
                    pc = condition_comeback_pc[repetition_counter - 1];
                    // Desempilha â€” o WHILE vai se reempilhar quando o PC chegar nele novamente
                    repetition_counter--;
                    condition_comeback_pc[repetition_counter]   = GARBAGE;
                    condition_comeback_code[repetition_counter] = GARBAGE;
                } else {
                    // IF ou outra condiÃ§Ã£o sem repetiÃ§Ã£o â€” descarta o topo da pilha
                    repetition_counter--;
                    condition_comeback_pc[repetition_counter]   = GARBAGE;
                    condition_comeback_code[repetition_counter] = GARBAGE;
                }
            }
        }
    }   
}
