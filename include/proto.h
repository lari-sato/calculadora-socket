#ifndef PROTO_H
#define PROTO_H

// Configurações do servidor
#define BUF_SIZE 1024
#define MAX_CLIENTS 10
#define DEFAULT_PORT 8080

// Comandos do protocolo
#define CMD_QUIT "QUIT"
#define CMD_HELP "HELP"
#define CMD_VERSION "VERSION"

// Operadores matemáticos (formato prefixo)
#define OP_ADD "ADD"
#define OP_SUB "SUB"
#define OP_MUL "MUL"
#define OP_DIV "DIV"

// Operadores matemáticos (formato infixo)
#define OP_ADD_INFIX "+"
#define OP_SUB_INFIX "-"
#define OP_MUL_INFIX "*"
#define OP_DIV_INFIX "/"

// Formatos de operação
#define FORMATO_PREFIXO 0
#define FORMATO_INFIXO 1

// Respostas do servidor
#define RESP_OK "OK"
#define RESP_ERR "ERR"

// Códigos de retorno das funções
#define CALC_SUCCESS 0
#define CALC_ERROR -1
#define CALC_DIVZERO -2
#define CMD_QUIT_CODE 2
#define CMD_HELP_CODE 3
#define CMD_VERSION_CODE 4

// Códigos de erro
#define ERR_BUSY "EBUSY"
#define ERR_INVALID "INVALID"
#define ERR_DIVZERO "DIVZERO"

// Informações da aplicação
#define APP_VERSION "1.0.0"
#define APP_HELP "Comandos disponíveis:\n- HELP: mostra esta ajuda\n- VERSION: versão do servidor\n- QUIT: sair\n- Calculadora (prefixo): ADD 5 3, SUB 10 2, MUL 4 6, DIV 8 2\n- Calculadora (infixo): 5 + 3, 10 - 2, 4 * 6, 8 / 2"

// Protocolo de comunicação
#define PROTO_HELP \
"Protocolo:\n" \
"Prefixo: ADD A B | SUB A B | MUL A B | DIV A B\n" \
"Infixo : A + B | A - B | A * B | A / B\n" \
"QUIT   : encerra a conexão\n" \
"Resposta:\n" \
"  OK <R>\\n  ou  ERR <COD> <mensagem>\\n"

#endif // PROTO_H