#ifndef LEXER_H
#define LEXER_H

#include "main.h"
#include "excessoes.h"
#include "tabela_simbolos.h"

class lexer
{
public:
    int num_linha = 1;
    tabela_simbolos *tbl_simbolos;
    list<token_t> *registro_lexico;

    lexer(FILE *_f);

    token_t proximo_token();

private:
    FILE *f;
};

#endif
