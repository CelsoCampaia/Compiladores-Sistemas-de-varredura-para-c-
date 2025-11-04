====================================
Scanner (Analisador Léxico) para C-
====================================

Este programa é um Analisador Léxico (Scanner) para a linguagem C-, 
implementado em C. 

Ele lê um código-fonte (armazenado em uma string) e o divide em "tokens" 
(palavras-chave, identificadores, números, operadores, etc.).

O scanner é implementado como um Autômato Finito Determinístico (AFD) 
usando 'switch-case' para gerenciar as transições de estado.

------------------------------------
COMO RODAR NO CODE::BLOCKS
------------------------------------

1. Abra o Code::Blocks.
2. Vá em 'File' > 'New' > 'Project...'.
3. Escolha 'Console application' e clique 'Go'.
4. Selecione 'C' como linguagem.
5. Dê um nome ao projeto.
6. O Code::Blocks criará um arquivo 'main.c'.
7. Apague todo o conteúdo do 'main.c' e cole este código.
8. Pressione F9 (ou clique em 'Build and run').

O programa irá compilar e executar, exibindo a lista de tokens 
identificados no código-fonte de exemplo (a variável 'source' 
na função 'main').

----------------------------------------------------

No final quando rodar o programa deverá aparecer isso:

--- Tokens Identificados ---
TIPO DA MARCA             | CADEIA DE CARACTERES (LEXEMA)
--------------------------|----------------------------------
PAL_RES_INT               | int
IDENTIFICADOR             | main
SIMB_ABRE_PAR             | (
PAL_RES_VOID              | void
SIMB_FECHA_PAR            | )
SIMB_ABRE_CHAVE           | {
PAL_RES_INT               | int
IDENTIFICADOR             | contador
OP_ATRIBUICAO             | =
NUMERO_INTEIRO            | 10
SIMB_PVIRGULA             | ;
PAL_RES_WHILE             | while
SIMB_ABRE_PAR             | (
IDENTIFICADOR             | contador
OP_MAIOR_IGUAL            | >=
NUMERO_INTEIRO            | 0
SIMB_FECHA_PAR            | )
SIMB_ABRE_CHAVE           | {
PAL_RES_IF                | if
SIMB_ABRE_PAR             | (
IDENTIFICADOR             | contador
OP_IGUAL_DUPLO            | ==
NUMERO_INTEIRO            | 5
SIMB_FECHA_PAR            | )
SIMB_ABRE_CHAVE           | {
IDENTIFICADOR             | contador
OP_ATRIBUICAO             | =
IDENTIFICADOR             | contador
OP_MENOS                  | -
NUMERO_INTEIRO            | 1
SIMB_PVIRGULA             | ;
SIMB_FECHA_CHAVE          | }
PAL_RES_ELSE              | else
SIMB_ABRE_CHAVE           | {
IDENTIFICADOR             | contador
OP_ATRIBUICAO             | =
IDENTIFICADOR             | contador
OP_MAIS                   | +
NUMERO_INTEIRO            | 1
SIMB_PVIRGULA             | ;
SIMB_FECHA_CHAVE          | }
SIMB_FECHA_CHAVE          | }
PAL_RES_RETURN            | return
NUMERO_INTEIRO            | 0
SIMB_PVIRGULA             | ;
SIMB_FECHA_CHAVE          | }
FIM_DE_ARQUIVO            | EOF
