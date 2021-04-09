#pragma once

enum token_type_t
{
    TK_ID,           // Identificador
    TK_CONST,        // Constante

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

enum const_type_t
{
    CONST_NULL,
    CONST_INT,
    CONST_CHAR,
    CONST_HEX,
    CONST_STR,
    CONST_BOOL
};
