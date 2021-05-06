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
    int endereco = 0;

    registro_tabela_simbolos() {}

    registro_tabela_simbolos(tipo_token_t t, std::string l)
        : tipo_token(t), lexema(l) {}
};

class tabela_simbolos : public tabela_hash<registro_tabela_simbolos>
{

public:

    tabela_simbolos();

    /*
     * Insere um registro na tabela.
     * Retorna um ponteiro para o registro inserido.
     */
    registro_tabela_simbolos* inserir(tipo_token_t tipo_token, std::string lexema);

    /*
     * Busca um elemento com a chave "lexema" na tabela.
     * Retorna um ponteiro para o registro encontrado ou NULL, caso nao tenha sido encontrado.
     */
    registro_tabela_simbolos* buscar(std::string lexema);

};
