#ifndef ERRORS_H
  #define ERRORS_H

  // TABELA DE ERROS
  //  1xx  → Erros Léxicos       (token desconhecido, caractere inválido)
  //  2xx  → Erros Sintáticos    (estrutura mal formada, token inesperado)
  //  3xx  → Erros Semânticos    (tipo errado, operação inválida)
  //  4xx  → Erros de Runtime    (divisão por zero, stack overflow)
  //  5xx  → Erros de Hardware   (I2C falhou, sensor não respondeu)


  // ---------------------------------------------------------------------------
  // 1xx — ERROS LÉXICOS
  // ---------------------------------------------------------------------------
  #define ERR_LEX_TOKEN_DESCONHECIDO   100  // Token lido não existe no alfabeto
  #define ERR_LEX_CARACTERE_INVALIDO   101  // Byte inválido recebido do hardware

  // Parser — verificação de balanceamento global
  #define ERR_SYN_COND_EXTRA           201  // Fechamentos condicionais excedem aberturas
  #define ERR_SYN_COND_FALTA           202  // Faltam fechamentos condicionais
  #define ERR_SYN_BLOCO_EXTRA          203  // Fechamentos de bloco excedem aberturas
  #define ERR_SYN_BLOCO_FALTA          204  // Faltam fechamentos de bloco
  #define ERR_SYN_FUNC_EXTRA           205  // Fechamentos de função excedem aberturas
  #define ERR_SYN_FUNC_FALTA           206  // Faltam fechamentos de função

  // LookAhead — verificação de adjacência de tokens
  #define ERR_SYN_START_DUPLO          210  // _START seguido de _START
  #define ERR_SYN_START_INVALIDO       211  // Após _START, próximo token inválido
  #define ERR_SYN_COND_PROXIMO         212  // Após CONDIÇÃO, próximo token inválido
  #define ERR_SYN_OPER_PROXIMO         213  // Após OPERADOR, próximo token inválido
  #define ERR_SYN_MET_PROXIMO          214  // Após MÉTODO, próximo token inválido
  #define ERR_SYN_FUNC_PROXIMO         215  // Após FUNÇÃO, próximo token inválido
  #define ERR_SYN_VALOR_PROXIMO        216  // Após VALOR, próximo token inválido
  #define ERR_SYN_VAR_PROXIMO          217  // Após VARIÁVEL, próximo token inválido
  #define ERR_SYN_ENDCOND_PROXIMO      218  // Após FECHACOND, próximo token inválido
  #define ERR_SYN_ENDBLOCO_PROXIMO     219  // Após FECHABLOCO, próximo token inválido
  #define ERR_SYN_ENDFUNC_PROXIMO      220  // Após FECHAFUNC, próximo token inválido
  #define ERR_SYN_LOGICO_PROXIMO       221  // Após LÓGICO, próximo token inválido

  // Contexto de condição
  #define ERR_SEM_COND_OVERFLOW        301  // Condição longa demais (overflow do buffer)
  #define ERR_SEM_EXPR_VAZIA           302  // Expressão dentro da condição está vazia
  #define ERR_SEM_EXPR1_NAO_METODO     303  // Expressão de 1 elemento não é método
  #define ERR_SEM_EXPR1_NAO_BOOL       304  // Método da expressão única não é booleano
  #define ERR_SEM_EXPR3_ELEM3_NAO_VAL  305  // Elemento 3 da tripla não é um valor
  #define ERR_SEM_EXPR3_ELEM2_NAO_OPER 306  // Elemento 2 da tripla não é operador
  #define ERR_SEM_EXPR3_OPER_BOOL      307  // Operador deve ser IGUAL quando valor é booleano
  #define ERR_SEM_EXPR3_ELEM1_INVALIDO 308  // Elemento 1 não é variável nem método não-nulo
  #define ERR_SEM_TIPO_INCOMPATIVEL    309  // Tipos incompatíveis entre elem. 1 e elem. 3
  #define ERR_SEM_EXPR_INCOMPLETA      310  // Expressão com apenas 2 tokens (incompleta)
  #define ERR_SEM_EXPR_EXCESSO         311  // Fração da condição com 4+ tokens

  // Contexto de função
  #define ERR_SEM_FUNC_TIPO_INVALIDO   312  // Argumento de função não é valor numérico
  #define ERR_SEM_FUNC_OVERFLOW        313  // Quantidade de argumentos excede o limite geral
  #define ERR_SEM_FUNC_ARG_EXCESSO     314  // Quantidade de argumentos excede o limite da função específica
  #define ERR_SEM_FUNC_TIPO_ERRADO     315  // Argumento é do tipo errado para a função

  // Uso fora de contexto
  #define ERR_SEM_OPER_FORA_COND       316  // Operador de comparação usado fora de condição
  #define ERR_SEM_LOGICO_FORA_COND     317  // Operador lógico (E/OU) usado fora de condição
  #define ERR_SEM_MET_NONVOID_FORA     318  // Método não-nulo usado fora de condição

  // Fechamentos indevidos (guarda extra — nunca deve ocorrer se o Parser passar)
  #define ERR_SEM_BLOCO_SEM_COND       319  // Fechamento de bloco sem condição ativa
  #define ERR_SEM_COND_SEM_ABERTURA    320  // Fechamento de condição sem abertura
  #define ERR_SEM_FUNC_SEM_ABERTURA    321  // Fechamento de função sem abertura

  #define ERR_RUN_STACK_OVERFLOW       400  // Pilha de condições excedeu o limite em execução
  #define ERR_RUN_DIVISAO_ZERO         401  // Tentativa de divisão por zero (reservado)
  #define ERR_RUN_PC_INVALIDO          402  // Program counter apontou para posição inválida

  #define ERR_HW_I2C_FALHA             500  // Transmissão I2C não completou (Wire error)
  #define ERR_HW_SENSOR_TIMEOUT        501  // Sensor não respondeu dentro do tempo esperado
  #define ERR_HW_EEPROM_FALHA          502  // Leitura/escrita na EEPROM falhou
  #define ERR_HW_SHIFT_OVERFLOW        503  // Shift register excedeu o número máximo de peças

#endif