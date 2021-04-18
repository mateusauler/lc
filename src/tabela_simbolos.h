#pragma once

#include "tabela_hash.h"
#include "main.h"

enum classe_t
{
    CL_VAR,
    CL_CONST,
    CL_NULL,
};

enum tipo_t
{
    TP_INT,
    TP_CHAR,
    TP_BOOL,
    TP_NULL,
};

struct registro_tabela_simbolos
{
    token_type_t tipo_token;
    std::string lexema;
    classe_t classe = CL_NULL;
    tipo_t tipo = TP_NULL;
    int tam = 0;

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
