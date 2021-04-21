#pragma once

#include <string>

#define exc_simples(N, M) \
class N : public erro_fonte \
{ \
public: \
    N() : erro_fonte(M) {} \
}

#define exc_lexema(N, M) \
class N : public erro_fonte \
{ \
public: \
    N(std::string lex) : erro_fonte(M) {} \
}

class erro_fonte : public std::exception
{
protected:
    std::string msg;

public:
    erro_fonte(std::string m) : msg(m) {}

    const char * what() const throw() { return msg.c_str(); }
};

exc_simples(char_invalido,         "caractere invalido.");
exc_lexema(lex_nao_identificado,   "lexema nao identificado [" + lex + "].");
exc_lexema(token_invalido,         "token nao esperado [" + lex + "].");
exc_simples(eof_inesperado,        "fim de arquivo nao esperado.");
exc_lexema(id_nao_declarado,       "identificador nao declarado [" + lex + "].");
exc_lexema(id_ja_declarado,        "identificador ja declarado [" + lex + "].");
exc_lexema(classe_id_incompativel, "classe de identificador incompativel [" + lex + "].");
exc_simples(tipo_incompativel,     "tipos incompativeis.");
exc_simples(tam_vet_excede_max,    "tamanho do vetor excede o maximo permitido.");
