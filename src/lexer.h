#pragma once

#include "tabela_simbolos.h"

enum state_t
{
    ST_START, // Inicio
    ST_END,   // Fim, deve retornar o token

    ST_ID_UNDERSCORE,       // Somente leu `_`
    ST_ID_NAME,             // Esta lendo o nome do identificador (max 32 char)

    ST_CONST_HEX_START,     // Leu `0`
    ST_CONST_HEX_ALPHA1,    // Leu `0(A-F)`
    ST_CONST_HEX_ALPHA2,    // Leu `0(A-F | 0-9)(A-F)`
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

struct token_t
{
    tipo_token_t tipo_token;
    std::string lex = "";

    registro_tabela_simbolos* simbolo = nullptr;

    tipo_constante_t tipo_constante = CONST_NULL;
    int tam_constante = 0;
    void *valor_const = nullptr;

    ~token_t();
};

class lexer
{
public:
    tabela_simbolos tbl_simbolos;

    lexer(FILE *f) : arq_fonte(f) {}

    // Retorna o numero da linha atual do arquivo fonte
    int get_linha() const;

    // Le o proximo token do arquivo fonte
    token_t proximo_token();

private:
    int num_linha = 1; // Numero da linha atual do arquivo fonte
    FILE *arq_fonte;
};
