#ifndef TABELA_SIMBOLOS_H
#define TABELA_SIMBOLOS_H

#include "tabela_hash.h"
#include "main.h"

using namespace std;

class tabela_simbolos : tabela_hash
{

public:
    using tabela_hash::tabela_hash;
    registro_tabela_simbolos* inserir(token_type_t tipo_token, string lexema);
    registro_tabela_simbolos* pesquisar(string lexema);
    list<registro_tabela_simbolos> listar_simbolos();

};

#endif