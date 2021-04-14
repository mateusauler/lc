#pragma once

#include "tabela_hash.h"
#include "main.h"

struct registro_tabela_simbolos
{
    token_type_t tipo_token;

    std::string lexema;

    registro_tabela_simbolos() { }

    registro_tabela_simbolos(token_type_t t, std::string l)
        : tipo_token(t), lexema(l) { }
};

class tabela_simbolos : public tabela_hash<registro_tabela_simbolos>
{

public:
    using tabela_hash<registro_tabela_simbolos>::tabela_hash;
    registro_tabela_simbolos* inserir(token_type_t tipo_token, std::string lexema);
    registro_tabela_simbolos* buscar(std::string lexema);

};
