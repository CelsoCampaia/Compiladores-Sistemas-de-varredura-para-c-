// Inclusão de bibliotecas padrão do C
#include <stdio.h>  // Para funções de entrada e saída (printf)
#include <stdlib.h> // Para funções padrão (NULL)
#include <string.h> // Para manipulação de strings (strcmp, memset, strcpy, strchr)
#include <ctype.h>  // Para verificação de tipos de caracteres (isspace, isalpha, isdigit, isalnum)

// --- Configurações Léxicas ---
#define MAX_LEXEME_LEN 100 // Tamanho máximo para a cadeia de caracteres (lexema)

// Estados do AFD (Autômato Finito Determinístico)
// Cada estado representa um ponto no processo de reconhecimento de um token
typedef enum {
    INICIO_S0,       // Estado inicial, ponto de partida
    IDENT_S1,        // Estado de reconhecimento de um Identificador (começou com letra ou _)
    NUMERO_S2,       // Estado de reconhecimento de um Número (começou com dígito)
    OP_REL_S3,       // Estado após encontrar <, >, = ou ! (pode ser um op. simples ou duplo)
    SIMPLES_S4,      // Estado (implícito) para símbolos de um caractere (ex: +, -, ;)
    DIVISAO_S5,      // Estado após encontrar uma / (pode ser divisão ou início de comentário)
    COMENTARIO_S6,   // Estado dentro de um comentário de bloco /* ... */
    OP_REL_DUPLO_S7, // Estado de aceitação para operadores de 2 caracteres (ex: ==, <=)
    ESTADO_ERRO      // Estado "ralo", para caracteres ou sequências inválidas
} Estado;

// Tipos de Tokens (Categorias de Marcas)
// Define a "etiqueta" que o analisador léxico atribui a cada lexema
typedef enum {
    TOKEN_ERRO,          // Lexema não reconhecido
    TOKEN_EOF,           // Fim de Arquivo (End of File)
    // Tokens de Aceitação
    TOKEN_ID, TOKEN_NUM, // Identificador, Número
    // Palavras Reservadas
    TOKEN_IF, TOKEN_ELSE, TOKEN_INT, TOKEN_RETURN, TOKEN_VOID, TOKEN_WHILE,
    // Operadores Aritméticos
    TOKEN_MAIS, TOKEN_MENOS, TOKEN_ASTERISCO, TOKEN_BARRA,
    // Operadores Relacionais e Atribuição
    TOKEN_MENOR, TOKEN_MENOR_IGUAL, TOKEN_MAIOR, TOKEN_MAIOR_IGUAL,
    TOKEN_IGUAL_DUPLO, TOKEN_DIFERENTE, TOKEN_ATRIBUICAO, // <<< CORREÇÃO 1
    // Símbolos de Pontuação
    TOKEN_PONTO_E_VIRGULA, TOKEN_ABRE_PARENTESES, TOKEN_FECHA_PARENTESES,
    TOKEN_ABRE_COLCHETES, TOKEN_FECHA_COLCHETES, TOKEN_ABRE_CHAVE, TOKEN_FECHA_CHAVE
} TipoToken;

// Mapa de strings para o tipo de token (usado para exibir a saída)
// Associa o enum TipoToken a uma string legível
const char* nomeTipoToken[] = {
    "ERRO", "FIM_DE_ARQUIVO", "IDENTIFICADOR", "NUMERO_INTEIRO",
    "PAL_RES_IF", "PAL_RES_ELSE", "PAL_RES_INT", "PAL_RES_RETURN", "PAL_RES_VOID", "PAL_RES_WHILE",
    "OP_MAIS", "OP_MENOS", "OP_MULTIPLICACAO", "OP_DIVISAO",
    "OP_MENOR", "OP_MENOR_IGUAL", "OP_MAIOR", "OP_MAIOR_IGUAL", "OP_IGUAL_DUPLO", "OP_DIFERENTE",
    "OP_ATRIBUICAO", // <<< CORREÇÃO 2
    "SIMB_PVIRGULA", "SIMB_ABRE_PAR", "SIMB_FECHA_PAR",
    "SIMB_ABRE_COL", "SIMB_FECHA_COL", "SIMB_ABRE_CHAVE", "SIMB_FECHA_CHAVE"
};

// Estrutura para o Token (Marca)
// Armazena a informação de um token reconhecido
typedef struct {
    TipoToken tipo;                     // A categoria do token (ex: TOKEN_ID)
    char lexema[MAX_LEXEME_LEN + 1]; // O texto original do código (ex: "contador")
} Token;

// Tabela de Palavras Reservadas
// Estrutura para facilitar a busca de palavras-chave
typedef struct {
    const char* lexema;
    TipoToken tipo;
} PalavraReservada;

// Lista de todas as palavras reservadas da linguagem C-
PalavraReservada palavrasReservadas[] = {
    {"if", TOKEN_IF}, {"else", TOKEN_ELSE}, {"int", TOKEN_INT},
    {"return", TOKEN_RETURN}, {"void", TOKEN_VOID}, {"while", TOKEN_WHILE},
    {NULL, TOKEN_ID} // Sentinela: marca o fim da lista
};

// --- Controle de Leitura do Código Fonte ---
char* codigoFonte = NULL; // Ponteiro global para o início do código-fonte na memória
int indiceAtual = 0;      // "Cursor" que indica a posição atual de leitura no código-fonte

// Função para avançar o ponteiro de leitura e retornar o caractere
// Simula a leitura de um arquivo, caractere por caractere
char proximoCaractere() {
    // Retorna o caractere na posição atual e DEPOIS incrementa o índice
    return codigoFonte[indiceAtual++];
}

// Função para recuar o ponteiro (usado após a detecção do fim de um lexema)
// Isso é essencial quando o AFD "lê um caractere a mais" para decidir o fim
void recuarCaractere() {
    // Decrementa o índice para que o próximo 'proximoCaractere()' leia o mesmo char novamente
    indiceAtual--;
}

// Verifica e classifica um Identificador como Palavra Reservada ou ID
// É chamada após o AFD reconhecer algo que *parece* um ID (ex: "if", "contador")
TipoToken verificaPalavraReservada(const char* lexema) {
    // Itera pela tabela de palavras reservadas
    for (int i = 0; palavrasReservadas[i].lexema != NULL; i++) {
        // Compara o lexema encontrado com a palavra reservada
        if (strcmp(lexema, palavrasReservadas[i].lexema) == 0) {
            // Se for igual, retorna o token da palavra reservada (ex: TOKEN_IF)
            return palavrasReservadas[i].tipo;
        }
    }
    // Se não encontrou em nenhuma, é um Identificador comum (ex: "contador")
    return TOKEN_ID;
}

// --- Função Principal do Scanner ---
// Esta é a implementação do AFD. A cada chamada, ela retorna o *próximo* token
Token getToken() {
    Estado estadoAtual = INICIO_S0; // Sempre começa no estado inicial
    Token token;                    // O token que será construído e retornado
    char c = '\0';                  // Armazena o caractere lido atualmente
    int posLexema = 0;              // Posição atual de escrita no 'token.lexema'

    // Zera o lexema para evitar "lixo" da chamada anterior
    memset(token.lexema, 0, sizeof(token.lexema));
    token.tipo = TOKEN_ERRO; // Define como ERRO por padrão

    // O "motor" do AFD: continua em loop enquanto não chegar a um estado final
    while (estadoAtual != ESTADO_ERRO) {
        c = proximoCaractere(); // Lê o próximo caractere

        // O 'switch' implementa a Tabela de Transição de Estados
        switch (estadoAtual) {
            // --- ESTADO INICIAL (S0) ---
            case INICIO_S0:
                // Ignora espaços em branco (espaço, tab, quebra de linha)
                // Fica em loop no S0
                while (isspace(c)) {
                    c = proximoCaractere();
                }

                // Transição S0 -> Fim de Arquivo (EOF)
                if (c == '\0') {
                    token.tipo = TOKEN_EOF;
                    strcpy(token.lexema, "EOF");
                    return token; // Retorna o token de Fim de Arquivo
                }

                // Transição S0 -> S1 (Identificador)
                if (isalpha(c) || c == '_') {
                    estadoAtual = IDENT_S1;
                    token.lexema[posLexema++] = c;
                }
                // Transição S0 -> S2 (Número)
                else if (isdigit(c)) {
                    estadoAtual = NUMERO_S2;
                    token.lexema[posLexema++] = c;
                }
                // Transição S0 -> S3 (Operadores Relacionais ou Atribuição)
                else if (c == '<' || c == '>' || c == '=' || c == '!') {
                    estadoAtual = OP_REL_S3;
                    token.lexema[posLexema++] = c;
                }
                // Transição S0 -> S5 (Divisão ou Comentário)
                else if (c == '/') {
                    estadoAtual = DIVISAO_S5;
                    token.lexema[posLexema++] = c;
                }
                // Transição S0 -> S4 (Símbolos Simples)
                else if (strchr("+-*;,()[]{}", c) != NULL) {
                    token.lexema[posLexema++] = c;
                    token.tipo = TOKEN_ERRO; // Será definido pelo S4 implícito
                    goto S4_SIMPLES; // Pula para o bloco de aceitação S4
                }
                // Se não for nada acima, é um caractere inválido
                else {
                    estadoAtual = ESTADO_ERRO;
                    token.lexema[posLexema++] = c;
                }
                break; // Fim do case INICIO_S0

            // --- ESTADO IDENTIFICADOR (S1) ---
            case IDENT_S1:
                // Loop em S1: Se for letra, dígito ou _
                if (isalnum(c) || c == '_') {
                    if (posLexema < MAX_LEXEME_LEN) { // Evita overflow do buffer
                        token.lexema[posLexema++] = c;
                    }
                }
                // Fim do ID: Se for qualquer outro caractere
                else {
                    recuarCaractere(); // Devolve o caractere lido (que não faz parte do ID)
                    token.lexema[posLexema] = '\0'; // Finaliza a string
                    // S1 é um estado de aceitação. Verifica se é palavra-chave ou ID
                    token.tipo = verificaPalavraReservada(token.lexema);
                    return token; // Retorna o token
                }
                break; // Fim do case IDENT_S1

            // --- ESTADO NÚMERO (S2) ---
            case NUMERO_S2:
                // Loop em S2: Se for dígito
                if (isdigit(c)) {
                    if (posLexema < MAX_LEXEME_LEN) {
                        token.lexema[posLexema++] = c;
                    }
                }
                // Fim do Número: Se for qualquer outro caractere (esta linguagem só aceita int)
                else {
                    recuarCaractere(); // Devolve o caractere lido (que não faz parte do NÚMERO)
                    token.lexema[posLexema] = '\0'; // Finaliza a string
                    // S2 é um estado de aceitação
                    token.tipo = TOKEN_NUM;
                    return token; // Retorna o token
                }
                break; // Fim do case NUMERO_S2

            // --- ESTADO OP_REL/ATRIB (S3) ---
            // Chegou aqui após ler <, >, = ou !
            case OP_REL_S3:
                // Transição S3 -> S7: Se o próximo for '=' (formando <=, >=, ==, !=)
                if (c == '=') {
                    estadoAtual = OP_REL_DUPLO_S7;
                    token.lexema[posLexema++] = c;
                }
                // Se o próximo NÃO for '='
                else {
                    recuarCaractere(); // Devolve o caractere lido
                    token.lexema[posLexema] = '\0'; // Finaliza o lexema (com 1 caractere)

                    // S3 é um estado de aceitação para <, > ou =
                    // <<< CORREÇÃO 3: Lógica para tratar <, >, =
                    if (strcmp(token.lexema, "<") == 0) token.tipo = TOKEN_MENOR;
                    else if (strcmp(token.lexema, ">") == 0) token.tipo = TOKEN_MAIOR;
                    else if (strcmp(token.lexema, "=") == 0) token.tipo = TOKEN_ATRIBUICAO;
                    // Se foi '!', mas não '!=', é um erro nesta linguagem
                    else {
                        estadoAtual = ESTADO_ERRO;
                        return token; // Retorna token de ERRO com lexema "!"
                    }
                    return token; // Retorna o token <, > ou =
                }
                break; // Fim do case OP_REL_S3

            // --- ESTADO OP_REL_DUPLO (S7) ---
            // Chegou aqui após ler <=, >=, == ou !=
            case OP_REL_DUPLO_S7:
                // S7 é um estado de aceitação imediata (já consumiu os 2 caracteres)
                token.lexema[posLexema] = '\0'; // Finaliza a string

                // Classifica qual operador duplo foi lido
                if (strcmp(token.lexema, "<=") == 0) token.tipo = TOKEN_MENOR_IGUAL;
                else if (strcmp(token.lexema, ">=") == 0) token.tipo = TOKEN_MAIOR_IGUAL;
                else if (strcmp(token.lexema, "==") == 0) token.tipo = TOKEN_IGUAL_DUPLO;
                else if (strcmp(token.lexema, "!=") == 0) token.tipo = TOKEN_DIFERENTE;
                else estadoAtual = ESTADO_ERRO; // Nunca deve acontecer

                // O 'proximoCaractere()' no topo do 'while' já leu o char *depois* do operador
                // (ex: o espaço em "== 10"). Precisamos devolvê-lo.
                recuarCaractere();
                return token; // Retorna o token

            // --- ESTADO DIVISÃO (S5) ---
            // Chegou aqui após ler /
            case DIVISAO_S5:
                // Transição S5 -> S6: Se o próximo for '*' (início de comentário /*)
                if (c == '*') {
                    estadoAtual = COMENTARIO_S6;
                    // Nota: Não salvamos o lexema do comentário
                }
                // Se não for '*', é apenas o operador de divisão
                else {
                    recuarCaractere(); // Devolve o caractere lido (ex: o '2' em "10 / 2")
                    token.lexema[posLexema] = '\0'; // Finaliza o lexema "/"
                    // S5 é um estado de aceitação para /
                    token.tipo = TOKEN_BARRA;
                    return token; // Retorna o token de divisão
                }
                break; // Fim do case DIVISAO_S5

            // --- ESTADO COMENTÁRIO (S6) ---
            // Estado "ralo" que consome caracteres até encontrar */
            case COMENTARIO_S6:
                // Se chegar ao fim do arquivo antes de fechar o comentário
                if (c == '\0') {
                    estadoAtual = ESTADO_ERRO;
                    token.tipo = TOKEN_ERRO;
                    strcpy(token.lexema, "Comentário não fechado");
                    return token;
                }
                // Se encontrar um *, pode ser o fim do comentário
                if (c == '*') {
                    // "Espia" o próximo caractere sem consumi-lo
                    if (codigoFonte[indiceAtual] == '/') {
                        proximoCaractere(); // Agora sim, consome o '/'
                        estadoAtual = INICIO_S0; // Volta ao estado inicial
                        posLexema = 0; // Prepara para o próximo token (o comentário é ignorado)
                        // O 'while' vai reiniciar, chamando getToken() internamente
                    }
                    // Se não for /, continua em S6 (ex: /***... )
                }
                // Se não for * (ou * seguido de /), apenas consome o caractere
                // e permanece em S6
                break; // Fim do case COMENTARIO_S6

            // --- ESTADO DE ERRO ---
            case ESTADO_ERRO:
                // O loop 'while' vai parar
                break;
        } // Fim do switch
    } // Fim do while

    // --- LÓGICA PARA SÍMBOLOS SIMPLES (S4) ---
    // Usamos 'goto' para pular para cá e evitar duplicar código
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
    else token.tipo = TOKEN_ERRO; // Segurança, não deve acontecer

    // Se o estado final é ERRO (ex: caractere '@')
    if (estadoAtual == ESTADO_ERRO) {
        token.tipo = TOKEN_ERRO;
        token.lexema[posLexema] = '\0'; // Finaliza o lexema do erro
    }

    return token; // Retorna o token de Símbolo Simples ou de Erro
}

// --- Função Principal de Teste (main) ---
// Configura o scanner e o executa sobre um código de exemplo
int main() {
    // Exemplo de código-fonte C- com todos os elementos definidos
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

    // Aponta o ponteiro global para o nosso código de exemplo
    codigoFonte = source;
    Token token; // Variável para armazenar o token retornado

    printf("--- Scanner para Linguagem C- ---\n\n");
    printf("Código Fonte de Exemplo:\n%s\n", codigoFonte);
    printf("\n--- Tokens Identificados ---\n");
    printf("%-25s | %s\n", "TIPO DA MARCA", "CADEIA DE CARACTERES (LEXEMA)");
    printf("--------------------------|----------------------------------\n");

    // Loop principal: pede tokens até encontrar o Fim de Arquivo (EOF)
    do {
        token = getToken(); // Pega o próximo token
        // Imprime o nome do tipo do token e seu lexema
        printf("%-25s | %s\n", nomeTipoToken[token.tipo], token.lexema);
    } while (token.tipo != TOKEN_EOF); // Para quando o token for EOF

    return 0; // Fim do programa
}
