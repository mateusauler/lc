#include "tabela_simbolos.h"

registro_tabela_simbolos* tabela_simbolos::inserir(token_type_t tipo_token, std::string lexema)
{
    registro_tabela_simbolos* obj = new registro_tabela_simbolos(tipo_token, lexema);

    hash_bkt* retorno = tabela_hash::inserir(lexema, obj);

    return (registro_tabela_simbolos*)retorno->elemento;
}

registro_tabela_simbolos* tabela_simbolos::pesquisar(std::string lexema)
{
    hash_bkt* retorno = tabela_hash::pesquisar(lexema);

    if (retorno == NULL) return NULL;

    return (registro_tabela_simbolos*)retorno->elemento;
}
