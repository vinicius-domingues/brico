#include "syntax.h"
#include "tokens.h"
#include "errors.h"

bool Syntax::Parser(int sequence[], int blocks_used){
    int token_da_vez;
    int count_conditions = 0;
    int count_blocks = 0;
    int count_functions = 0;
    bool error_flag = false;

    if(!error_flag){
        for(int i = 0 ; i < blocks_used ; i++){
            token_da_vez = sequence[i];

            switch (token_da_vez) {
                
                case _IF:
                case _WHILE:
                    count_conditions++;
                    count_blocks++;
                    break;

                case _DELAY:
                    count_functions++;
                    break;

                case _ENDBLOCK:
                    count_blocks--;
                    break;

                case _ENDFUNCTION:
                    count_functions--;
                    break;

                case _ENDCONDITION:
                    count_conditions--;
                    break;
            }
        }
        if (count_conditions != 0){
            if(count_conditions < 0) {
                result = ERR_SYN_COND_EXTRA;
                Serial.println(F("[PARSER] Erro 201: Ha mais fechamentos condicionais que condicoes"));
            }else{
                result = ERR_SYN_COND_FALTA;
                Serial.println(F("[PARSER] Erro 202: Faltam fechamentos condicionais"));
            }
            error_position = -1; // Erro de balanceamento global — sem posição única
            error_flag = true;
        }
    }

    if(!error_flag){
        if (count_blocks != 0){
            if(count_blocks < 0) {
                result = ERR_SYN_BLOCO_EXTRA;
                Serial.println(F("[PARSER] Erro 203: Ha mais fechamentos de bloco que condicoes"));
            }else{
                result = ERR_SYN_BLOCO_FALTA;
                Serial.println(F("[PARSER] Erro 204: Faltam fechamentos de bloco"));
            }
            error_position = -1;
            error_flag = true;
        }
    }

    if(!error_flag){
        if (count_functions != 0){
            if(count_functions < 0) {
                result = ERR_SYN_FUNC_EXTRA;
                Serial.println(F("[PARSER] Erro 205: Ha mais fechamentos de funcao que funcoes"));
            }else{
                result = ERR_SYN_FUNC_FALTA;
                Serial.println(F("[PARSER] Erro 206: Faltam fechamentos de funcao"));
            }
            error_position = -1;
            error_flag = true;
        }
    } 

    if(error_flag){
        Serial.print(F("[PARSER] Falha: "));
        Serial.println(result);
    }else{
        Serial.print(F("[PARSER] Sucesso: "));
        Serial.println(result);
    }

    return error_flag;
}

bool Syntax::LookAhead(int sequence[], int blocks_used){
    bool error_flag = false;
    int token_da_vez;
    int proximo = 0;

    for (int i = 0; i < blocks_used; i++) {
        
        token_da_vez = sequence[i];

        if (i + 1 < blocks_used){
            proximo = sequence[i + 1];
        }else{
            proximo = _END;
        }

        // Token _START (1/11)
        if(token_da_vez == _START){
            if(proximo == _START){
                result = ERR_SYN_START_DUPLO;
                Serial.println(F("[LOOKAHEAD] Erro 210: INICIO seguido de INICIO vazio"));
                error_flag = true;
                error_position = i;
            }else if(!isCondition(proximo) && !isMethod(proximo) && !isFunction(proximo) && proximo != _END){
                result = ERR_SYN_START_INVALIDO;
                Serial.println(F("[LOOKAHEAD] Erro 211: INICIO deve ser seguido por Condicao, Metodo ou Funcao"));
                error_flag = true;
                error_position = i;
            }
        }

        // Token CONDIÇÃO (2/11)
        else if(isCondition(token_da_vez)){
            if(!isMethod(proximo) && !isVariable(proximo) && proximo != _START){
                result = ERR_SYN_COND_PROXIMO;
                Serial.println(F("[LOOKAHEAD] Erro 212: Condicao deve ser seguida por Metodo, Variavel ou INICIO"));
                error_flag = true;
                error_position = i;
            }
        }
        
        // Token OPERADOR (3/11)
        else if(isOperation(token_da_vez)){
            if(!isMethod(proximo) && !isValue(proximo)){
                result = ERR_SYN_OPER_PROXIMO;
                Serial.println(F("[LOOKAHEAD] Erro 213: Operador deve ser seguido por Metodo ou Valor"));
                error_flag = true;
                error_position = i;
            }
        }

        // Token MÉTODO (4/11)
        else if(isMethod(token_da_vez)){
            if(isVariable(proximo) || isValue(proximo) /*|| isCondition(proximo)*/){ 
                result = ERR_SYN_MET_PROXIMO;
                Serial.println(F("[LOOKAHEAD] Erro 214: Metodo nao pode ser seguido por METODO, VARIAVEL ou VALOR"));
                error_flag = true;
                error_position = i;
            }
        }

        // Token FUNÇÃO (5/11)
        else if(isFunction(token_da_vez)){
            if(proximo != _START && !isValue(proximo)){
                result = ERR_SYN_FUNC_PROXIMO;
                Serial.println(F("[LOOKAHEAD] Erro 215: Funcao deve ser seguida por INICIO ou Valor"));
                error_flag = true;
                error_position = i;
            }
        }

        // Token VALOR (6/11)
        else if(isValue(token_da_vez)){
            if(!isLogical(proximo) && !isEndCondition(proximo) && !isEndFunction(proximo) && !isValue(proximo) ){
                result = ERR_SYN_VALOR_PROXIMO;
                Serial.println(F("[LOOKAHEAD] Erro 216: Valor deve ser seguido por Logico, valor ou Fechamento de funcao/condicao"));
                error_flag = true;
                error_position = i;
            }
        }

        // Token VARIAVEL (7/11)
        else if(isVariable(token_da_vez)){
            if(proximo != _START && !isOperation(proximo)){
                result = ERR_SYN_VAR_PROXIMO;
                Serial.println(F("[LOOKAHEAD] Erro 217: Variavel deve ser seguida por Operacao ou INICIO"));
                error_flag = true;
                error_position = i;
            }
        }

        // Token ENDCOND (8/11)
        else if(isEndCondition(token_da_vez)){
            if(proximo != _START && !isCondition(proximo) && !isMethod(proximo) && !isFunction(proximo) && !isEndBlock(proximo) ){
                result = ERR_SYN_ENDCOND_PROXIMO;
                Serial.println(F("[LOOKAHEAD] Erro 218: Proximo token invalido apos Fechar Condicao"));
                error_flag = true;
                error_position = i;
            }
        }

        // Token ENDBLOCK (9/11)
        else if(isEndBlock(token_da_vez)){
            if(proximo != _START && !isCondition(proximo) && !isMethod(proximo) && !isFunction(proximo) && !isEndBlock(proximo) && proximo != _END){
                result = ERR_SYN_ENDBLOCO_PROXIMO;
                Serial.println(F("[LOOKAHEAD] Erro 219: Proximo token invalido apos Fechar Bloco"));
                error_flag = true;
                error_position = i;
            }
        }

        // Token ENDFUNCTION (10/11)
        else if(isEndFunction(token_da_vez)){
            if(proximo != _START && !isCondition(proximo) && !isMethod(proximo) && !isFunction(proximo) && !isEndBlock(proximo) && proximo != _END){
                result = ERR_SYN_ENDFUNC_PROXIMO;
                Serial.println(F("[LOOKAHEAD] Erro 220: Proximo token invalido apos Fechar Funcao"));
                error_flag = true;
                error_position = i;
            }
        }

       // Token LÓGICO (11/11)
        else if(isLogical(token_da_vez)){
            if(!isVariable(proximo) && !isMethod(proximo) ){
                result = ERR_SYN_LOGICO_PROXIMO;
                Serial.println(F("[LOOKAHEAD] Erro 221: Operador Logico deve ser seguido por Variavel ou Metodo"));
                error_flag = true;
                error_position = i;
            }
        }

        if(error_flag){
            Serial.print(F("[LOOKAHEAD] Atual/Prox: "));
            Serial.print(token_da_vez);
            Serial.print(F(" / "));
            Serial.println(proximo);
            Serial.print(F("[LOOKAHEAD] Posicao do erro: "));
            Serial.println(error_position);
            break;
        }
    }

    if(!error_flag){
        Serial.print(F("[LOOKAHEAD] Sucesso: "));
        Serial.println(result);
    }

    return error_flag;
}

bool Syntax::Semantic(int sequence[], int blocks_used){
    static const int condition_spaces = 50; // Limite de blocos para a condição
    static const int function_spaces = 4; // Limite de blocos para a função
    static int function_code = GARBAGE;
    static int function_pointer = 0;
    int function_args[function_spaces] = {GARBAGE};
    int tokens_in_condition_space[condition_spaces] = {GARBAGE}; 
    int token_da_vez;
    int qtd_conditions = 0;
    int qtd_functions = 0;
    int saving_token = 0;
    int qtd_elements_in_condition = 0;
    int missing_endblocks = 0; 
    bool error_flag = false;
    bool is_in_condition = false;
    bool is_in_function = false;

    for (int i = 0; i < blocks_used ; i++) {
        token_da_vez = sequence[i];

        // Se abertura de condições maior que zero, está em uma condição
        if(qtd_conditions > 0){
            if(!isEndCondition(token_da_vez)){
                // Bloqueia overflow (uma condição pode ter apenas 4 OU ou E)
                if(saving_token < condition_spaces) { 
                        tokens_in_condition_space[saving_token] = token_da_vez;
                        saving_token++;
                } else {
                        result = ERR_SEM_COND_OVERFLOW;
                        Serial.println(F("[SEMANTIC] Erro 301: Condicao complexa demais (Overflow)"));
                        error_position = i;
                        error_flag = true;
                        break;
                }
            }

            is_in_condition = true;
        }else{
            /* Serial.println(F("Começo/fim da condição: ")); Serial.println((token_da_vez));}*/     
            if(saving_token > 0 || qtd_elements_in_condition != 0){
                //Limpezas

                // Antes de limpar, joga para analisar lexicamente
                // error_position já é setado dentro de ExpressionValidator via this->
                error_flag = Syntax::ExpressionValidator(tokens_in_condition_space, saving_token);
                
                // Se encontrou erro de expressão, já para tudo
                if(error_flag){
                    error_position = i; // posição do fechamento da condição que revelou o problema
                    break;
                }else{ // Limpa para continuar para a próxima condição
                    for(int k = 0 ; k < condition_spaces ; k++) {
                        tokens_in_condition_space[k] = GARBAGE;        
                    }
                    
                    saving_token = 0;
                    qtd_elements_in_condition = 0; 
                }
             }         
             is_in_condition = false;      
        }
        
    
        // Se abertura de funções maior que zero, está em uma função
        if(qtd_functions > 0){
            is_in_function = true;
        }else{
            is_in_function = false;  
        }

        if(isCondition(token_da_vez)){
            qtd_conditions++;
            missing_endblocks++;
        }else if(isEndCondition(token_da_vez)){
            qtd_conditions--;
        }

        if(isEndBlock(token_da_vez)){
            missing_endblocks--;
        }

        if(isFunction(token_da_vez)){
            qtd_functions++;
            function_code = token_da_vez;
        }else if(isEndFunction(token_da_vez)){
            qtd_functions--;

            // Valida se os tipos e a quantidade de argumentos está certa
            error_flag = this->FunctionValidator(function_code, function_args, function_pointer);
            if(error_flag){ error_position = i; }
            
            // Limpeza
            function_code = GARBAGE;

            for(int l = 0 ; l < function_pointer ; l++){
                function_args[l] = GARBAGE;
            }
                
            function_pointer = 0;

        }else if(is_in_function){
            // Não pode nada além de valor numérico dentro de função
            if(!isNumberValue(token_da_vez) && !isFunction(token_da_vez) && !isEndFunction(token_da_vez)){
                result = ERR_SEM_FUNC_TIPO_INVALIDO;
                Serial.println(F("[SEMANTIC] Erro 312: Funcao aceita apenas valores numericos"));
                error_position = i;
                error_flag = true; 
            }

            // Armazena
            function_args[function_pointer] = token_da_vez;

            // Serial.print(F("Somando um no ponteiro: ")); Serial.println(function_pointer);
            function_pointer++;

            // Valida estouro de funções gerais
            if(function_pointer > function_spaces){
                result = ERR_SEM_FUNC_OVERFLOW;
                Serial.println(F("[SEMANTIC] Erro 313: Estouro de valores."));
                error_position = i;
                error_flag = true;
            }
        }

        if(is_in_condition){
            qtd_elements_in_condition++;
        }else{
            // Não pode comparar ou usar métodos não nulos fora de condição
            if(isOperation(token_da_vez)){
                result = ERR_SEM_OPER_FORA_COND;
                Serial.println(F("[SEMANTIC] Erro 316: Operacoes devem estar dentro de condicoes"));
                error_position = i;
                error_flag = true;
            }else if(isLogical(token_da_vez)){
                result = ERR_SEM_LOGICO_FORA_COND;
                Serial.println(F("[SEMANTIC] Erro 317: Logicos (E/OU) devem estar dentro de condicoes"));
                error_position = i;
                error_flag = true;   
            }else if(isNonVoidMethod(token_da_vez)){
                result = ERR_SEM_MET_NONVOID_FORA;
                Serial.println(F("[SEMANTIC] Erro 318: Acoes (metodos) nao nulos devem estar dentro de condicoes"));
                error_position = i;
                error_flag = true;   
            }
        }
        
        // Não pode fechar bloco sem estar em condicional
        if(missing_endblocks < 0){
            result = ERR_SEM_BLOCO_SEM_COND;
            Serial.println(F("[SEMANTIC] Erro 319: Fechou blocos sem poder"));
            error_position = i;
            error_flag = true;
        } else if(qtd_conditions < 0){ // Isso nunca aconteceria, é só precaução
            result = ERR_SEM_COND_SEM_ABERTURA;
            Serial.println(F("[SEMANTIC] Erro 320: Fechou condicoes sem poder"));
            error_position = i;
            error_flag = true; 
        }else if(qtd_functions < 0){ // Isso nunca aconteceria, é só precaução
            result = ERR_SEM_FUNC_SEM_ABERTURA;
            Serial.println(F("[SEMANTIC] Erro 321: Fechou funcoes sem poder"));
            error_position = i;
            error_flag = true; 
        }

        // Para o loop se encontrar erro
        if(error_flag){
            Serial.print(F("[SEMANTIC] Falha: "));
            Serial.println(result);
            Serial.print(F("[SEMANTIC] Posicao do erro: "));
            Serial.println(error_position);
            break;
        }
    }

    if(!error_flag){
        Serial.print(F("[SEMANTIC] Sucesso: "));
        Serial.println(result);
    }
    
    return error_flag;
}

bool Syntax::ExpressionValidator(int tokens_in_condition[], int size_tokens_in_condition){
    int* position_in_array = tokens_in_condition; 
    int total_qtd = size_tokens_in_condition;
    int qtd_dividers = 0;
    int qtd_tokens_per_expression = 0;
    int last_condition_token = 0;
    bool error_flag = false;

    //Serial.println(F("--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------"));
   // Serial.print(F("[SEMANTIC] Quantia total: ")); Serial.println(total_qtd);
    for(int l = 0; l < total_qtd ; l++){
        //Serial.print(F("[SEMANTIC] [l]: ")); Serial.println(l);
        //Serial.print(F("[SEMANTIC] [Pos absoluta]: ")); Serial.println(position_in_array[l]);

        // Se não é lógico, soma o token lido
        if(!isLogical(position_in_array[l])){
            qtd_tokens_per_expression++;    
            //Serial.print(F("[SEMANTIC] qtd_tokens_per_expression SOMADO: ")); Serial.println(qtd_tokens_per_expression);
        }else{
            //Serial.print(F("[SEMANTIC] qtd_tokens_per_expression NÃO SOMADO: ")); Serial.println(qtd_tokens_per_expression);
        }

        // Se for fim da expressão (caraceter condicional) ou fim da condição, valida o progresso até ali
        if(isLogical(position_in_array[l]) || (total_qtd == (l + 1))){
            qtd_dividers++;
            //Serial.print(F("[SEMANTIC] [Entrada]: ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ")); Serial.print(qtd_dividers); Serial.println(F(" ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ "));

            // Reposicionamento de último token considerado lido (não inclui os lógicos)
            last_condition_token = l;
            if(isLogical(position_in_array[l])){
                Serial.println(F("[SEMANTIC] Motivo de entrada 1: isLogical(position_in_array[l]"));
                last_condition_token--;
            }
            else {
                //Serial.println(F("[SEMANTIC] Motivo de entrada 2: total_qtd == (qtd_tokens_per_expression == l+1)"));
            }            

            //Serial.print(F("[SEMANTIC] [Último token relativo]: ")); Serial.println(position_in_array[last_condition_token]);            
            //Serial.print(F("[SEMANTIC] [Expressão]: ")); Serial.println(qtd_dividers);            

            /* Validação de quantidade nas expressões
                1 - Tem quer ter 1 ou 3 valores (qtd_tokens_per_expression)
            */
            
            switch (qtd_tokens_per_expression){
                case 0:
                    // Serial.println(F("[SEMANTIC] Case 0 (qtd_tokens_per_expression)"));
                    result = ERR_SEM_EXPR_VAZIA;
                    Serial.println(F("[SEMANTIC] Erro 302: Expressao vazia"));
                    error_flag = true;
                    break;
                case 1:
                    // Serial.println(F("[SEMANTIC] Case 1 (qtd_tokens_per_expression)"));
                    if(!isMethod(position_in_array[last_condition_token])){
                        result = ERR_SEM_EXPR1_NAO_METODO;
                        Serial.println(F("[SEMANTIC] Erro 303: Expressao unica nao e METODO do tipo BOOLEANO"));
                        error_flag = true;
                    }else{
                        if(!(getType(position_in_array[last_condition_token]) == _BOOLEAN)){
                            result = ERR_SEM_EXPR1_NAO_BOOL;
                            Serial.println(F("[SEMANTIC] Erro 304: Metodo da expressao unica deveria ser booleano"));
                            error_flag = true;
                        }
                    }
                    break;
                case 2:
                    // Serial.println(F("[SEMANTIC] Case 2 (qtd_tokens_per_expression)"));
                    result = ERR_SEM_EXPR_INCOMPLETA;
                    Serial.println(F("[SEMANTIC] Erro 310: Expressao tem valores insuficientes (2)"));
                    error_flag = true;
                    break;
                case 3:
                    // Serial.println(F("[SEMANTIC] Case 3 (qtd_tokens_per_expression)"));
                {
                    int pointer = last_condition_token;      // x
                    int slots = last_condition_token - 3;    // x - 3
                    int stage = 0;
                    int value_type;                          // Tipo da estrutura (evitar comparação entre tipos diferentes)
                    
                    // Verificação da estrutura passada
                    for( ; pointer > slots ; pointer--){
                        
                        // Serial.print(F("[SEMANTIC] Stage: ")); Serial.println(position_in_array[pointer]);
                        /* Validação da lógica nas expressões compostas
                            1 - Comparações devem ser realizadas entre o mesmo tipo
                            2 - Se o valor for booleano, a comparação deve ser com igual
                        */
                        
                        switch(stage){
                            case 0: // Verifica se é valor (METODO/VARIAVEL  IGUAL  <<TRUE>>)
                                if(!isValue(position_in_array[pointer])){
                                    result = ERR_SEM_EXPR3_ELEM3_NAO_VAL;
                                    Serial.println(F("[SEMANTIC] Erro 305: Elemento 3 da expressao nao e um valor"));
                                    error_flag = true;
                                }else{
                                    value_type = getType(position_in_array[pointer]);
                                }
                                break;
                            case 1: // Verifica se é operador (METODO/VARIAVEL  <<IGUAL>>  TRUE)
                                if(!isOperation(position_in_array[pointer])){
                                    result = ERR_SEM_EXPR3_ELEM2_NAO_OPER;
                                    Serial.println(F("[SEMANTIC] Erro 306: Elemento 2 da expressao nao e um operador"));
                                    error_flag = true;
                                }else{
                                    // Se value_type for booleano e a operação for diferente de IGUAL
                                    if(value_type == _BOOLEAN && !(position_in_array[pointer] == _EQUAL)){
                                        result = ERR_SEM_EXPR3_OPER_BOOL;
                                        Serial.println(F("[SEMANTIC] Erro 307: Operador (elem. 2) deve ser IGUAL quando valor (elem. 3) e BOOLEANO"));
                                        error_flag = true;
                                    }
                                }
                                break;
                            case 2: // Verifica se é método ou variável (<<METODO/VARIAVEL>>  IGUAL  TRUE)
                                if(!isVariable(position_in_array[pointer]) && !isNonVoidMethod(position_in_array[pointer])){
                                    result = ERR_SEM_EXPR3_ELEM1_INVALIDO;
                                    Serial.println(F("[SEMANTIC] Erro 308: Elemento 1 nao e variavel ou nao e metodo nao-nulo"));
                                    error_flag = true;
                                }else{
                                    if(getType(position_in_array[pointer]) != value_type){
                                        result = ERR_SEM_TIPO_INCOMPATIVEL;
                                        Serial.println(F("[SEMANTIC] Erro 309: Metodo/variavel (elem. 1) deve ser do mesmo tipo do valor (elem. 3)"));
                                        error_flag = true;    
                                    }
                                }
                                break;
                        }
                        stage++;

                        if(error_flag){
                            break;
                        }
                    }
                    break;
                }
                default:
                    result = ERR_SEM_EXPR_EXCESSO;
                    Serial.println(F("[SEMANTIC] Erro 311: Fracao da condicao tem valores demais (4+)"));
                    error_flag = true;
                    break;
            }
            
            // Limpeza (pois daqui iniciará uma nova expressão na mesma condição)
            qtd_tokens_per_expression = 0; 
            // Serial.print(F("[SEMANTIC] [Saída]: ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ")); Serial.print(qtd_dividers); Serial.println(F(" ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ "));
        }
    }

    if(!error_flag){
        Serial.print(F("[EXPRESSION] Sucesso: ")); Serial.println(result);
    }
    
    return error_flag;
}

bool Syntax::FunctionValidator(int function_code, int function_args[], int tamanho){
    bool error_flag = false;

    switch(function_code){
        case _DELAY:
            Serial.println(F("[SEMANTIC] function_code == _DELAY"));

            // Valida quantidade
            if (tamanho > 1){
                result = ERR_SEM_FUNC_ARG_EXCESSO;
                Serial.println(F("[SEMANTIC] Erro 314: Estouro de valores para essa funcao especifica."));
                error_flag = true;    
            }

            // Valida tipos
            else if(!isNumberValue(function_args[0])){
                result = ERR_SEM_FUNC_TIPO_ERRADO;
                Serial.println(F("[SEMANTIC] Erro 315: Valor de tipo errado"));
                error_flag = true;
            }
        break;
    }
    return error_flag;
}