#pragma once

#include "tabela_hash.h"
#include "main.h"

struct registro_tabela_simbolos
{
    token_type_t tipo_token;

    std::string lexema;

    registro_tabela_simbolos() { }

    registro_tabela_simbolos(token_type_t t, std::string l)
    {
        this->tipo_token = t;
        this->lexema = l;
    }
};

class tabela_simbolos : tabela_hash
{

public:
    using tabela_hash::tabela_hash;
    ~tabela_simbolos();
    registro_tabela_simbolos* inserir(token_type_t tipo_token, std::string lexema);
    registro_tabela_simbolos* buscar(std::string lexema);

};
