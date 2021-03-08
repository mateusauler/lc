#ifndef TABELA_SIMBOLOS_H
#define TABELA_SIMBOLOS_H

#include "hash_tbl.h"
#include "main.h"
#include <list>

using namespace std;

class tabela_simbolos : hash_tbl
{

public:
    using hash_tbl::hash_tbl;
    registro_tabela_simbolos* inserir(token_type_t tipo_token, string lexema);
    registro_tabela_simbolos* pesquisar(string lexema);
    list<registro_tabela_simbolos> listar_simbolos();

};

#endif