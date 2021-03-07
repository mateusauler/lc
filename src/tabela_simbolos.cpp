#include "hash_tbl.cpp"
#include "main.h"
#include <list>

using namespace std;

class tabela_simbolos : public hash_tbl
{
public:
    using hash_tbl::hash_tbl;

    registro_tabela_simbolos* inserir(token_type_t tipo_token, string lexema)
    {
        registro_tabela_simbolos* obj = new registro_tabela_simbolos(tipo_token, lexema);

        hash_bkt* retorno = hash_tbl::inserir(lexema, obj);

        return (registro_tabela_simbolos*)retorno->elemento;
    }

    registro_tabela_simbolos* pesquisar(string lexema)
    {
        hash_bkt* retorno = hash_tbl::pesquisar(lexema);

        if (retorno == NULL) return NULL;

        return (registro_tabela_simbolos*)retorno->elemento;
    }

    list<registro_tabela_simbolos> listar_simbolos()
    {
        list<hash_bkt> lb = hash_tbl::listar_elementos();
        list<registro_tabela_simbolos> *l = new list<registro_tabela_simbolos>;

        for (hash_bkt const &i: lb)
        {
            registro_tabela_simbolos r = *(registro_tabela_simbolos*)i.elemento;
            l->push_back(r);
        }

        return *l;
    }
};