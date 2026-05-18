#ifndef TOKENS_H
    #define TOKENS_H

    #define _END 255
    #define GARBAGE -2
    #define ACTIVATED -3

    // Fluxo inicial
    #define _START 1  

    // Condição
    #define _WHILE 2
    #define _IF 3

    // Fechadores solitários
    #define _ENDCONDITION 4
    #define _ENDBLOCK 5
    #define _ENDFUNCTION 6

    // Operação
    #define _EQUAL 10
    #define _BIGGER 11
    #define _SMALLER 12
    #define _NOT 13

    // Lógico
    #define _AND 20
    #define _OR 21



    // Função
    #define _DELAY 50

    // Valor (ATENÇÃO: Numéricos vêm antes dos Booleanos para a regra isValue abranger todos)
    // Intervalo Numérico: 60 a 64
    #define _ZERO 60
    #define _ONE 61
    #define _FIVE 62
    #define _FIFTY 63
    #define _THOUSAND 64
    
    // Intervalo Booleano: 70 a 71
    #define _TRUE 70
    #define _FALSE 71

    // Variáveis
    #define _SEGUNDOS 80

    // Estados
    #define _LISTENING 0
    #define _MAPPING 1
    #define _INTERPRETING 2
    #define _RUNNING 3

    // Tipos
    #define _BOOLEAN -100
    #define _NUMERIC -200
    #define _VOID -300    

    
    // Método: Não void (Intervalo isolado)
    #define _PROXIMITY 40   // Comando para ler sensor

    // Método: Void (Intervalo: 30 a 35)
    #define _BRAKE 30       // Comando para freiar                  (VOID)
    #define _ACCELERATE 31  // Comando para acelerar                (VOID)
    #define _HONK 32        // Comando para buzinar                 (VOID)
    #define _RED_LED 33     // Comando para acender led vermelho    (VOID)
    #define _GREEN_LED 34   // Comando para acender led verde       (VOID)
    #define _BLUE_LED 35    // Comando para acender led azul        (VOID)

    // Hardware — Controller
    #define PIN_BUTTON 5 
    #define PIN_CLOCK 6
    #define PIN_SET 7
    #define I2C 8
    #define EEPROM_ADDR_0 0x50 
    #define EEPROM_ADDR_1 0x51

    // Hardware — LEDs dos blocos físicos
    // TODO: definir pinos reais quando o hardware estiver pronto
    // Cada bloco tem um LED bicolor (verde = normal, vermelho = erro)
    // Sugestão: controle via shift register nos pinos abaixo
    #define PIN_BLOCK_LED_DATA   -1  // Pino de dados serial para os LEDs dos blocos (INDEFINIDO)
    #define PIN_BLOCK_LED_CLOCK  -1  // Pino de clock serial para os LEDs dos blocos (INDEFINIDO)
    #define PIN_BLOCK_LED_LATCH  -1  // Pino de latch para os LEDs dos blocos          (INDEFINIDO)
    #define BLOCK_LED_MAX        100  // Número máximo de blocos suportados

    // Cores dos LEDs dos blocos físicos
    #define BLOCK_COLOR_OFF    0  // Apagado
    #define BLOCK_COLOR_GREEN  1  // Verde  (bloco avaliado sem erro)
    #define BLOCK_COLOR_YELLOW 2  // Amarelo (bloco sendo avaliado agora)
    #define BLOCK_COLOR_RED    3  // Vermelho (bloco com erro / erro global)

    // Hardware — Display 7 segmentos (pinos A0-A6 usados como digitais 14-20)
    // Ordem dos segmentos: { A,  B,  C,  D,  E,  F,  G }
    #define PIN_SEG_A  14  // A0
    #define PIN_SEG_B  15  // A1
    #define PIN_SEG_C  16  // A2
    #define PIN_SEG_D  17  // A3
    #define PIN_SEG_E  18  // A4
    #define PIN_SEG_F  19  // A5
    #define PIN_SEG_G  20  // A6 (somente leitura no Uno — usar Mega ou substituir por digital livre)

    // Seleção de dígito — cátodo comum: LOW ativa o dígito
    // Ajuste os pinos conforme seu hardware
    #define PIN_DIG_1  2   // Dígito 1 (mais à esquerda) — exibe 'E'
    #define PIN_DIG_2  3   // Dígito 2 — centenas do código de erro
    #define PIN_DIG_3  4   // Dígito 3 — dezenas  do código de erro
    #define PIN_DIG_4  13  // Dígito 4 — unidades  do código de erro
    #define SEG_DIGITS 4   // Total de dígitos no display

    // Tempo de cada dígito ativo durante o multiplexing (ms)
    #define SEG_MUX_DELAY_MS 3

    // Índices de estado do display 7 segmentos (tabela letrasEstado)
    #define SEG_STATE_DEBUG     0  // 'd'
    #define SEG_STATE_COMPILE   1  // 'C'
    #define SEG_STATE_RUNNING   2  // 'r'
    #define SEG_STATE_ERROR     3  // 'E'
    #define SEG_STATE_LISTENING 4  // 'L'


// 1 - Ações e Sensores (Separados para corrigir erros semânticos)
inline bool isVoidMethod(int t)    { return (t >= _BRAKE && t <= _BLUE_LED); } 
inline bool isNonVoidMethod(int t) { return (t >= _PROXIMITY && t <= _PROXIMITY); }                      
inline bool isMethod(int t)        { return (isVoidMethod(t) || isNonVoidMethod(t)); } 
inline bool isBoolMethod(int t)    { return (t == _PROXIMITY); } 

// 2 - Valores e Variáveis
inline bool isVariable(int t)      { return (t == _SEGUNDOS); }   
inline bool isValue(int t)         { return (t >= _ZERO && t <= _FALSE); }          
inline bool isBooleanValue(int t)  { return (t >= _TRUE && t <= _FALSE); }        
inline bool isNumberValue(int t)   { return (t >= _ZERO && t <= _THOUSAND); }

// 3 - Operadores
inline bool isOperation(int t)     { return (t >= _EQUAL && t <= _NOT); } 
inline bool isLogical(int t)       { return (t >= _AND && t <= _OR); }        

// 4 - Estruturas e Funções
inline bool isCondition(int t)     { return (t == _IF || t == _WHILE); }      
inline bool isFunction(int t)      { return (t == _DELAY); }                 

// 5 - Fechadores
inline bool isEndCondition(int t)  { return (t == _ENDCONDITION); }                
inline bool isEndBlock(int t)      { return (t == _ENDBLOCK); }               
inline bool isEndFunction(int t)   { return (t == _ENDFUNCTION); }            

// 6 - Verificadores de tipagem
inline int getType(int t) {
    
    // 1 = Tipo booleano
    if (isBooleanValue(t) || isBoolMethod(t)) {
        return _BOOLEAN; 
    }
    
    // 0 = Tipo numérico
    if (isNumberValue(t) || isVariable(t)) {
        return _NUMERIC; 
    }
    
    // -1 = Sem tipo (void)
    return _VOID; 
} 

#endif // TOKENS_H