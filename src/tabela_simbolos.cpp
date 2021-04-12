#include "tabela_simbolos.h"

tabela_simbolos::~tabela_simbolos()
{
    // Apaga todos os elementos presentes na tabela
    // TODO: Revisar a classe tabela_hash para que isto seja feito em seu desconstrutor
    for (int i = 0; i < tamanho_tbl; ++i)
        if (tabela[i])
        {
            hash_bkt *tmp = tabela[i];

            while (tmp)
            {
                delete (registro_tabela_simbolos*)tmp->elemento;
                tmp = tmp->prox;
            }
        }
}

// Insere um registro na tabela
// Retorna um ponteiro para o registro inserido
registro_tabela_simbolos* tabela_simbolos::inserir(token_type_t tipo_token, std::string lexema)
{
    registro_tabela_simbolos* obj = new registro_tabela_simbolos(tipo_token, lexema);

    hash_bkt* retorno = tabela_hash::inserir(lexema, obj);

    return (registro_tabela_simbolos*)retorno->elemento;
}

// Busca um elemento com a chave "lexema" na tabela
// Retorna um ponteiro para o registro encontrado ou NULL, caso nao tenha sido encontrado
registro_tabela_simbolos* tabela_simbolos::buscar(std::string lexema)
{
    hash_bkt* retorno = tabela_hash::buscar(lexema);

    if (retorno == NULL) return NULL;

    return (registro_tabela_simbolos*)retorno->elemento;
}
