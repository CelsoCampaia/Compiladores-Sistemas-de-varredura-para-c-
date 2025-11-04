#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// --- Configurações Léxicas ---
#define MAX_LEXEME_LEN 100 // Tamanho máximo para a cadeia de caracteres (lexema)

// Estados do AFD (baseados na Tabela de Estados)
typedef enum {
    INICIO_S0,
    IDENT_S1,
    NUMERO_S2,
    OP_REL_S3,
    SIMPLES_S4,
    DIVISAO_S5,
    COMENTARIO_S6,
    OP_REL_DUPLO_S7,
    ESTADO_ERRO
} Estado;

// Tipos de Tokens (Categorias de Marcas)
typedef enum {
    TOKEN_ERRO,
    TOKEN_EOF,
    // Aceitação
    TOKEN_ID, TOKEN_NUM,
    TOKEN_IF, TOKEN_ELSE, TOKEN_INT, TOKEN_RETURN, TOKEN_VOID, TOKEN_WHILE,
    TOKEN_MAIS, TOKEN_MENOS, TOKEN_ASTERISCO, TOKEN_BARRA,
    TOKEN_MENOR, TOKEN_MENOR_IGUAL, TOKEN_MAIOR, TOKEN_MAIOR_IGUAL, TOKEN_IGUAL_DUPLO, TOKEN_DIFERENTE,
    TOKEN_PONTO_E_VIRGULA, TOKEN_ABRE_PARENTESES, TOKEN_FECHA_PARENTESES,
    TOKEN_ABRE_COLCHETES, TOKEN_FECHA_COLCHETES, TOKEN_ABRE_CHAVE, TOKEN_FECHA_CHAVE
} TipoToken;

// Mapa de strings para o tipo de token (para exibição)
const char* nomeTipoToken[] = {
    "ERRO", "FIM_DE_ARQUIVO", "IDENTIFICADOR", "NUMERO_INTEIRO",
    "PAL_RES_IF", "PAL_RES_ELSE", "PAL_RES_INT", "PAL_RES_RETURN", "PAL_RES_VOID", "PAL_RES_WHILE",
    "OP_MAIS", "OP_MENOS", "OP_MULTIPLICACAO", "OP_DIVISAO",
    "OP_MENOR", "OP_MENOR_IGUAL", "OP_MAIOR", "OP_MAIOR_IGUAL", "OP_IGUAL_DUPLO", "OP_DIFERENTE",
    "SIMB_PVIRGULA", "SIMB_ABRE_PAR", "SIMB_FECHA_PAR",
    "SIMB_ABRE_COL", "SIMB_FECHA_COL", "SIMB_ABRE_CHAVE", "SIMB_FECHA_CHAVE"
};

// Estrutura para o Token (Marca)
typedef struct {
    TipoToken tipo;
    char lexema[MAX_LEXEME_LEN + 1];
} Token;

// Tabela de Palavras Reservadas [cite: 7]
typedef struct {
    const char* lexema;
    TipoToken tipo;
} PalavraReservada;

PalavraReservada palavrasReservadas[] = {
    {"if", TOKEN_IF}, {"else", TOKEN_ELSE}, {"int", TOKEN_INT},
    {"return", TOKEN_RETURN}, {"void", TOKEN_VOID}, {"while", TOKEN_WHILE},
    {NULL, TOKEN_ID} // Sentinela
};

// --- Controle de Leitura do Código Fonte ---
char* codigoFonte = NULL;
int indiceAtual = 0;

// Função para avançar o ponteiro de leitura e retornar o caractere
char proximoCaractere() {
    return codigoFonte[indiceAtual++];
}

// Função para recuar o ponteiro (usado após a detecção do fim de um lexema)
void recuarCaractere() {
    indiceAtual--;
}

// Verifica e classifica um Identificador como Palavra Reservada ou ID [cite: 20]
TipoToken verificaPalavraReservada(const char* lexema) {
    for (int i = 0; palavrasReservadas[i].lexema != NULL; i++) {
        if (strcmp(lexema, palavrasReservadas[i].lexema) == 0) {
            return palavrasReservadas[i].tipo;
        }
    }
    return TOKEN_ID;
}

// --- Função Principal do Scanner ---
Token getToken() {
    Estado estadoAtual = INICIO_S0;
    Token token;
    char c = '\0';
    int posLexema = 0;

    // Zera o lexema
    memset(token.lexema, 0, sizeof(token.lexema));
    token.tipo = TOKEN_ERRO;

    while (estadoAtual != ESTADO_ERRO) {
        c = proximoCaractere();

        switch (estadoAtual) {
            case INICIO_S0: // Estado Inicial [cite: 11]
                // Ignora espaços em branco, tabulação ou quebra de linha [cite: 7, 14]
                while (isspace(c)) {
                    c = proximoCaractere();
                }

                if (c == '\0') {
                    token.tipo = TOKEN_EOF;
                    strcpy(token.lexema, "EOF");
                    return token;
                }

                // Reconhece letra ou '_' -> S1 (Identificador/Palavra-chave) [cite: 12]
                if (isalpha(c) || c == '_') {
                    estadoAtual = IDENT_S1;
                    token.lexema[posLexema++] = c;
                }
                // Reconhece dígito -> S2 (Número) [cite: 12]
                else if (isdigit(c)) {
                    estadoAtual = NUMERO_S2;
                    token.lexema[posLexema++] = c;
                }
                // Reconhece <, >, =, ! -> S3 (Operadores Relacionais) [cite: 12]
                else if (c == '<' || c == '>' || c == '=' || c == '!') {
                    estadoAtual = OP_REL_S3;
                    token.lexema[posLexema++] = c;
                }
                // Reconhece / -> S5 (Divisão ou Comentário) [cite: 12]
                else if (c == '/') {
                    estadoAtual = DIVISAO_S5;
                    token.lexema[posLexema++] = c;
                }
                // Reconhece Símbolos Simples (+, -, *, ;, (, ), [, ], {, }) -> S4 [cite: 12, 7]
                else if (strchr("+-*;,()[]{}", c) != NULL) {
                    token.lexema[posLexema++] = c;
                    token.tipo = TOKEN_ERRO; // Será definido pelo S4 implícito
                    goto S4_SIMPLES; // Vai para a lógica S4 e retorna o token
                }
                else {
                    estadoAtual = ESTADO_ERRO;
                    token.lexema[posLexema++] = c;
                }
                break;

            case IDENT_S1: // Estado Identificador/Palavra-chave
                // Transição: letra/dígito/_ -> S1
                if (isalnum(c) || c == '_') {
                    if (posLexema < MAX_LEXEME_LEN) {
                        token.lexema[posLexema++] = c;
                    }
                } else {
                    recuarCaractere(); // Volta o caractere que não pertence ao lexema
                    token.lexema[posLexema] = '\0';
                    token.tipo = verificaPalavraReservada(token.lexema); // Palavra-chave ou ID
                    return token;
                }
                break;

            case NUMERO_S2: // Estado Número Inteiro
                // Transição: dígito -> S2
                if (isdigit(c)) {
                    if (posLexema < MAX_LEXEME_LEN) {
                        token.lexema[posLexema++] = c;
                    }
                } else {
                    recuarCaractere(); // Volta o caractere que não pertence ao lexema
                    token.lexema[posLexema] = '\0';
                    token.tipo = TOKEN_NUM;
                    return token;
                }
                break;

            case OP_REL_S3: // Estado Operador Relacional Simples (<, >, =, !)
                // Transição: '=' -> S7 (Operador Relacional Duplo)
                if (c == '=') {
                    estadoAtual = OP_REL_DUPLO_S7;
                    token.lexema[posLexema++] = c;
                } else {
                    recuarCaractere(); // Volta o caractere que não pertence
                    token.lexema[posLexema] = '\0';

                    // Se não for seguido por '=', é um operador simples (somente '<' ou '>')
                    if (strcmp(token.lexema, "<") == 0) token.tipo = TOKEN_MENOR;
                    else if (strcmp(token.lexema, ">") == 0) token.tipo = TOKEN_MAIOR;
                    // O símbolo '=' sozinho não é válido (é tratado com o símbolo simples)
                    else {
                        estadoAtual = ESTADO_ERRO;
                        return token;
                    }
                    return token;
                }
                break;

            case OP_REL_DUPLO_S7: // Estado Operador Relacional Duplo (<=, >=, ==, !=) [cite: 16]
                // O S7 é um estado de aceitação imediata (já consumiu 2 caracteres)
                token.lexema[posLexema] = '\0';
                if (strcmp(token.lexema, "<=") == 0) token.tipo = TOKEN_MENOR_IGUAL;
                else if (strcmp(token.lexema, ">=") == 0) token.tipo = TOKEN_MAIOR_IGUAL;
                else if (strcmp(token.lexema, "==") == 0) token.tipo = TOKEN_IGUAL_DUPLO;
                else if (strcmp(token.lexema, "!=") == 0) token.tipo = TOKEN_DIFERENTE;
                else estadoAtual = ESTADO_ERRO; // Nunca deve acontecer
                recuarCaractere(); // O caractere 'c' lido já está fora do lexema (recua 1 e sai)
                return token;

            case DIVISAO_S5: // Estado / encontrado
                // Transição: '*' -> S6 (Comentário)
                if (c == '*') {
                    estadoAtual = COMENTARIO_S6;
                } else {
                    recuarCaractere(); // Volta o caractere lido (ex: se for um espaço)
                    token.lexema[posLexema] = '\0';
                    token.tipo = TOKEN_BARRA; // É o token de Divisão '/'
                    return token;
                }
                break;

            case COMENTARIO_S6: // Estado Comentário /*...*/
                // Ignora tudo até encontrar o padrão '*/'
                if (c == '\0') {
                    // Erro: Fim de arquivo antes do fim do comentário
                    estadoAtual = ESTADO_ERRO;
                    token.tipo = TOKEN_ERRO;
                    strcpy(token.lexema, "Comentário não fechado");
                    return token;
                }
                if (c == '*') {
                    // Próximo pode ser o final
                    if (codigoFonte[indiceAtual] == '/') {
                        proximoCaractere(); // Consome o '/'
                        estadoAtual = INICIO_S0; // Volta ao estado inicial (Comentários são ignorados) [cite: 21, 25]
                        posLexema = 0; // Prepara para o próximo token
                        // S6 é estado Não-Aceitação, retorna ao S0
                    }
                }
                // Permanece em S6
                break;

            case ESTADO_ERRO:
                // Tratado fora do switch
                break;
        }
    }

    // S4_SIMPLES: Lógica para Símbolos Simples (Estado de Aceitação Implícito)
    S4_SIMPLES:
    if (token.lexema[0] == '+') token.tipo = TOKEN_MAIS;
    else if (token.lexema[0] == '-') token.tipo = TOKEN_MENOS;
    else if (token.lexema[0] == '*') token.tipo = TOKEN_ASTERISCO;
    else if (token.lexema[0] == ';') token.tipo = TOKEN_PONTO_E_VIRGULA;
    else if (token.lexema[0] == '(') token.tipo = TOKEN_ABRE_PARENTESES;
    else if (token.lexema[0] == ')') token.tipo = TOKEN_FECHA_PARENTESES;
    else if (token.lexema[0] == '[') token.tipo = TOKEN_ABRE_COLCHETES;
    else if (token.lexema[0] == ']') token.tipo = TOKEN_FECHA_COLCHETES;
    else if (token.lexema[0] == '{') token.tipo = TOKEN_ABRE_CHAVE;
    else if (token.lexema[0] == '}') token.tipo = TOKEN_FECHA_CHAVE;
    else if (token.lexema[0] == '=') token.tipo = TOKEN_IGUAL_DUPLO; // Assumindo '==' para '=' em S3, o que sobra aqui é o '=' de atribuição
    else token.tipo = TOKEN_ERRO;

    // Se o estado final é ERRO
    if (estadoAtual == ESTADO_ERRO) {
        token.tipo = TOKEN_ERRO;
        token.lexema[posLexema] = '\0';
    }

    return token;
}

// --- Função Principal de Teste (main) ---
int main() {
    // Exemplo de código-fonte C- com todos os elementos definidos [cite: 7]
    char source[] = "/* Arquivo de teste C- */\n"
                    "int main (void) {\n"
                    "  int contador = 10; \n"
                    "  while (contador >= 0) {\n"
                    "    if (contador == 5) {\n"
                    "      contador = contador - 1;\n"
                    "    } else {\n"
                    "      contador = contador + 1;\n"
                    "    }\n"
                    "  }\n"
                    "  return 0;\n"
                    "}";

    codigoFonte = source;
    Token token;

    printf("--- Scanner para Linguagem C- ---\n\n");
    printf("Código Fonte de Exemplo:\n%s\n", codigoFonte);
    printf("\n--- Tokens Identificados ---\n");
    printf("%-25s | %s\n", "TIPO DA MARCA", "CADEIA DE CARACTERES (LEXEMA)");
    printf("--------------------------|----------------------------------\n");

    do {
        token = getToken();
        printf("%-25s | %s\n", nomeTipoToken[token.tipo], token.lexema);
    } while (token.tipo != TOKEN_EOF);

    return 0;
}
