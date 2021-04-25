#pragma once

enum tipo_token_t
{
    TK_ID,          // Identificador
    TK_CONST,       // Constante

    TK_OP_ATRIB,    // :=

    TK_OP_LT,       // <
    TK_OP_GT,       // >
    TK_OP_LE,       // <=
    TK_OP_GE,       // >=
    TK_OP_EQ,       // =
    TK_OP_NE,       // <>

    TK_OP_MAIS,     // +
    TK_OP_MENOS,    // -
    TK_OP_MUL,      // *
    TK_OP_BARRA,    // /
    TK_OP_PORCENTO, // %
    TK_OP_VIRGULA,  // ,

    TK_GRU_A_PAR,   // (
    TK_GRU_F_PAR,   // )
    TK_GRU_A_COL,   // [
    TK_GRU_F_COL,   // ]
    TK_GRU_A_CHA,   // {
    TK_GRU_F_CHA,   // }

    TK_RES_FINAL,   // final
    TK_RES_INT,     // int
    TK_RES_CHAR,    // char
    TK_RES_BOOLEAN, // boolean

    TK_RES_IF,      // if
    TK_RES_ELSE,    // else
    TK_RES_THEN,    // then

    TK_RES_FOR,     // for

    TK_RES_AND,     // and
    TK_RES_OR,      // or
    TK_RES_NOT,     // not

    TK_RES_WRITE,   // write
    TK_RES_WRITELN, // writeln
    TK_RES_READLN,  // readln
    TK_RES_MAIN,    // main

    TK_FIM_DECL,    // ;
    TK_EOF          // EOF
};

enum estado_t
{
    ES_INICIO, // Inicio
    ES_FIM,    // Fim, deve retornar o token

    ES_ID_UNDERLINE,        // Somente leu `_`
    ES_ID_NOME,             // Esta lendo o nome do identificador (max 32 char)

    ES_CONST_HEX_INICIO,   // Leu `0`
    ES_CONST_HEX_ALPHA1,   // Leu `0(A-F)`
    ES_CONST_HEX_ALPHA2,   // Leu `0` seguido de dois digitos hexa (sendo, pelo menos um `A-F`)
    ES_CONST_HEX_NUM1,     // Leu `0` seguido de um digito
    ES_CONST_HEX_NUM2,     // Leu `0` seguido de dois digitos
    ES_CONST_NUM,          // Esta lendo constante numerica (nao hexa)

    ES_CONST_CHAR_INICIO,  // Leu `'`
    ES_CONST_CHAR_INTERNO, // Leu `'` e um caractere imprimivel

    ES_CONST_STR_INTERNO,  // Leu `"` e esta lendo caracteres de string

    ES_COMENTARIO,         // Leu `/*` e esta lendo comentario
    ES_COMENTARIO_FIM,     // Leu `*` e pode ler `/`, terminando o comentario

    ES_OP_BARRA,           // Leu `/`
    ES_OP_ATRIB_INICIO,    // Leu `:`
    ES_OP_LT,              // Leu `<`
    ES_OP_GT,              // Leu `>`
};

enum tipo_constante_t
{
    CONST_NULL,
    CONST_INT,
    CONST_CHAR,
    CONST_HEX,
    CONST_STR,
    CONST_BOOL
};

enum classe_t
{
    CL_NULL,
    CL_VAR,
    CL_CONST
};

enum tipo_dados_t
{
    TP_NULL,
    TP_INT,
    TP_CHAR,
    TP_BOOL,
    TP_STRING
};
