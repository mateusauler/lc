#pragma once

enum tipo_token_t
{
    TK_ID,            // Identificador
    TK_CONST,         // Constante

    TK_OP_ATTRIB,     // :=

    TK_OP_LT,         // <
    TK_OP_GT,         // >
    TK_OP_LE,         // <=
    TK_OP_GE,         // >=
    TK_OP_EQ,         // =
    TK_OP_NE,         // <>

    TK_OP_PLUS,       // +
    TK_OP_MINUS,      // -
    TK_OP_MUL,        // *
    TK_OP_SLASH,      // /
    TK_OP_PERCENT,    // %
    TK_OP_COMMA,      // ,

    TK_BRA_O_PAR,     // (
    TK_BRA_C_PAR,     // )
    TK_BRA_O_SQR,     // [
    TK_BRA_C_SQR,     // ]
    TK_BRA_O_CUR,     // {
    TK_BRA_C_CUR,     // }

    TK_RES_FINAL,     // final
    TK_RES_INT,       // int
    TK_RES_CHAR,      // char
    TK_RES_BOOLEAN,   // boolean

    TK_RES_IF,        // if
    TK_RES_ELSE,      // else
    TK_RES_THEN,      // then

    TK_RES_FOR,       // for

    TK_RES_AND,       // and
    TK_RES_OR,        // or
    TK_RES_NOT,       // not

    TK_RES_WRITE,     // write
    TK_RES_WRITELN,   // writeln
    TK_RES_READLN,    // readln
    TK_RES_MAIN,      // main

    TK_END_STATEMENT, // ;
    TK_EOF            // EOF
};

enum state_t
{
    ST_START, // Inicio
    ST_END,   // Fim, deve retornar o token

    ST_ID_UNDERSCORE,       // Somente leu `_`
    ST_ID_NAME,             // Esta lendo o nome do identificador (max 32 char)

    ST_CONST_HEX_START,     // Leu `0`
    ST_CONST_HEX_ALPHA1,    // Leu `0(A-F)`
    ST_CONST_HEX_ALPHA2,    // Leu `0` seguido de dois digitos hexa (sendo, pelo menos um `A-F`)
    ST_CONST_HEX_NUM1,      // Leu `0` seguido de um digito
    ST_CONST_HEX_NUM2,      // Leu `0` seguido de dois digitos
    ST_CONST_NUM,           // Esta lendo constante numerica (nao hexa)

    ST_CONST_CHAR_START,    // Leu `'`
    ST_CONST_CHAR_INTERNAL, // Leu `'` e um caractere imprimivel

    ST_CONST_STR_INTERNAL,  // Leu `"` e esta lendo caracteres de string

    ST_COMMENT,             // Leu `/*` e esta lendo comentario
    ST_COMMENT_END,         // Leu `*` e pode ler `/`, terminando o comentario

    ST_OP_SLASH,            // Leu `/`
    ST_OP_ATTRIB_START,     // Leu `:`
    ST_OP_LT,               // Leu `<`
    ST_OP_GT,               // Leu `>`
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
    TP_BOOL
};
