#include "tabela_simbolos.h"

using namespace std;

registro_tabela_simbolos* tabela_simbolos::inserir(token_type_t tipo_token, string lexema)
{
    registro_tabela_simbolos* obj = new registro_tabela_simbolos(tipo_token, lexema);

    hash_bkt* retorno = tabela_hash::inserir(lexema, obj);

    return (registro_tabela_simbolos*)retorno->elemento;
}

registro_tabela_simbolos* tabela_simbolos::pesquisar(string lexema)
{
    hash_bkt* retorno = tabela_hash::pesquisar(lexema);

    if (retorno == NULL) return NULL;

    return (registro_tabela_simbolos*)retorno->elemento;
}

list<registro_tabela_simbolos> tabela_simbolos::listar_simbolos()
{
    list<hash_bkt> lb = tabela_hash::listar_elementos();
    list<registro_tabela_simbolos> *l = new list<registro_tabela_simbolos>;

    for (hash_bkt const &i: lb)
    {
        registro_tabela_simbolos r = *(registro_tabela_simbolos*)i.elemento;
        l->push_back(r);
    }

    return *l;
}
