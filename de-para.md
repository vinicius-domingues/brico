# De-Para — Tokens Fuzzy Blocks

Referência completa de todos os IDs de token usados no protocolo bit-bang e no compilador.

## Controle de Fluxo

| ID | Peça | Significado |
|----|------|-------------|
| 1 | `_START` | Início da fita — delimitador obrigatório de abertura do programa |
| 255 | `_END` | Fim da fita — encerra o programa sem loop |

## Estruturas Condicionais

| ID | Peça | Significado |
|----|------|-------------|
| 2 | `_WHILE` | Laço de repetição — executa o bloco enquanto a condição for verdadeira |
| 3 | `_IF` | Condicional — executa o bloco se a condição for verdadeira |

## Fechadores

| ID | Peça | Significado |
|----|------|-------------|
| 4 | `_ENDCONDITION` | Fecha uma estrutura condicional (`_IF` / `_WHILE`) |
| 5 | `_ENDBLOCK` | Fecha um bloco de comandos |
| 6 | `_ENDFUNCTION` | Fecha uma chamada de função |

## Operadores de Comparação

| ID | Peça | Significado |
|----|------|-------------|
| 10 | `_EQUAL` | Operador `==` — verifica igualdade |
| 11 | `_BIGGER` | Operador `>` — verifica se maior |
| 12 | `_SMALLER` | Operador `<` — verifica se menor |
| 13 | `_NOT` | Operador `!` — negação lógica |

## Operadores Lógicos

| ID | Peça | Significado |
|----|------|-------------|
| 20 | `_AND` | Operador `&&` — E lógico entre duas condições |
| 21 | `_OR` | Operador `||` — OU lógico entre duas condições |

## Função

| ID | Peça | Significado |
|----|------|-------------|
| 50 | `_DELAY` | Aguarda um tempo definido pelo valor seguinte |

## Métodos (retornam valor)

| ID | Peça | Significado |
|----|------|-------------|
| 40 | `_PROXIMITY` | Lê o sensor de proximidade — retorna booleano |

## Métodos Void (comandos de ação)

| ID | Peça | Significado |
|----|------|-------------|
| 30 | `_BRAKE` | Freia o carrinho |
| 31 | `_ACCELERATE` | Acelera o carrinho |
| 32 | `_HONK` | Aciona a buzina |
| 33 | `_RED_LED` | Acende o LED vermelho |
| 34 | `_GREEN_LED` | Acende o LED verde |
| 35 | `_BLUE_LED` | Acende o LED azul |

## Valores Numéricos

| ID | Peça | Significado |
|----|------|-------------|
| 60 | `_ZERO` | Valor numérico `0` |
| 61 | `_ONE` | Valor numérico `1` |
| 62 | `_FIVE` | Valor numérico `5` |
| 63 | `_FIFTY` | Valor numérico `50` |
| 64 | `_THOUSAND` | Valor numérico `1000` |

## Valores Booleanos

| ID | Peça | Significado |
|----|------|-------------|
| 70 | `_TRUE` | Valor booleano `true` |
| 71 | `_FALSE` | Valor booleano `false` |

## Variáveis

| ID | Peça | Significado |
|----|------|-------------|
| 80 | `_SEGUNDOS` | Variável de tempo — representa segundos decorridos |

## Internos (não mapeados em peças físicas)

| ID | Peça | Significado |
|----|------|-------------|
| -2 | `GARBAGE` | Token inválido / lixo de leitura |
| -3 | `ACTIVATED` | Marca que o bloco já foi processado |
| -100 | `_BOOLEAN` | Tipo booleano (uso interno do analisador semântico) |
| -200 | `_NUMERIC` | Tipo numérico (uso interno do analisador semântico) |
| -300 | `_VOID` | Tipo void (uso interno do analisador semântico) |
| 0 | `_LISTENING` | Estado: aguardando leitura |
| 1 | `_MAPPING` | Estado: mapeando blocos |
| 2 | `_INTERPRETING` | Estado: interpretando programa |
| 3 | `_RUNNING` | Estado: executando programa |
