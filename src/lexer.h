#pragma once

#include <string>
#include <list>

#include "tabela_simbolos.h"

enum state_t
{
    ST_START, // Inicio
    ST_END,   // Fim, deve retornar o token

    ST_ID_UNDERSCORE,       // Somente leu _
    ST_ID_NAME,             // Esta lendo o nome do identificador (max 32 char)

    ST_CONST_HEX_START,     // Leu 0
    ST_CONST_HEX_ALPHA1,    // Leu um [A-F]
    ST_CONST_HEX_ALPHA2,    // Leu dois [A-F]
    ST_CONST_HEX_NUM1,      // Leu 0 seguido de um digito
    ST_CONST_HEX_NUM2,      // Leu 0 seguido de dois digitos
    ST_CONST_NUM,           // Esta lendo constante numerica (nao hexa)

    ST_CONST_CHAR_START,    // Leu '
    ST_CONST_CHAR_INTERNAL, // Leu ' e um caractere imprimivel

    ST_CONST_STR_INTERNAL,  // Leu " e esta lendo caracteres de string

    ST_COMMENT,             // Leu /* e esta lendo comentario
    ST_COMMENT_END,         // Leu * e pode ler /, terminando o comentario

    ST_OP_SLASH,            // Leu /
    ST_OP_ATTRIB_START,     // Leu :
    ST_OP_LT,               // Leu <
    ST_OP_GT,               // Leu >
};

struct token_t
{
    token_type_t tipo;
    std::string lex;
    registro_tabela_simbolos* simbolo;
    const_type_t tipo_constante;
    int tam_constante;
};

class lexer
{
public:
    int num_linha = 1;
    tabela_simbolos *tbl_simbolos;

    lexer(FILE *_f);
    ~lexer();

    token_t proximo_token();

private:
    FILE *f;
};
