#include "hash_tbl.cpp"
#include "tipos.h"

using namespace std;

class tabela_simbolos: hash_tbl {
    public:
    hash_bkt **tabela;
    int tamanho_tbl;

    registro_tabela_simbolos* inserir(token_type_t tip_tok, string lexema) {
        registro_tabela_simbolos* obj = new registro_tabela_simbolos(tip_tok, lexema);
        hash_bkt* retorno = hash_tbl::inserir(lexema, obj);
        return (registro_tabela_simbolos*)retorno->elemento;
    }

    registro_tabela_simbolos* pesquisar(string lexema) {
        hash_bkt* retorno = hash_tbl::pesquisar(lexema);
        return (registro_tabela_simbolos*)retorno->elemento;
    }

    private:

};