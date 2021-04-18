#pragma once

#include "tabela_hash.h"
#include "enums.h"

struct registro_tabela_simbolos
{
    tipo_token_t tipo_token;
    std::string lexema;
    classe_t classe = CL_NULL;
    tipo_dados_t tipo = TP_NULL;
    int tam = 0;

    registro_tabela_simbolos() { }

    registro_tabela_simbolos(tipo_token_t t, std::string l)
        : tipo_token(t), lexema(l) { }
};

class tabela_simbolos : public tabela_hash<registro_tabela_simbolos>
{

public:
    tabela_simbolos();
    registro_tabela_simbolos* inserir(tipo_token_t tipo_token, std::string lexema);
    registro_tabela_simbolos* buscar(std::string lexema);

};
