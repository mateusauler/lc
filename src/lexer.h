#pragma once

#include "tabela_simbolos.h"

struct token_t
{
    tipo_token_t tipo_token;
    std::string lex = "";

    registro_tabela_simbolos* simbolo = nullptr;

    tipo_constante_t tipo_constante = CONST_NULL;
    int tam_constante = 0;
};

class lexer
{
public:
    tabela_simbolos tbl_simbolos;

    lexer(FILE *f) : arq_fonte(f) {}
    ~lexer();

    // Retorna o numero da linha atual do arquivo fonte
    int get_linha() const;

    // Le o proximo token do arquivo fonte
    void proximo_token();

    // Ultimo token gerado pelo analisador lexico
    token_t *token_lido = nullptr;

protected:
    int num_linha = 1; // Numero da linha atual do arquivo fonte

private:
    FILE *arq_fonte;
};
