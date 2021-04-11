#include "tabela_hash.h"

tabela_hash::tabela_hash(int tamanho)
{
    if (tamanho <= 0)
        tamanho = 128;

    tamanho_tbl = tamanho;
    tabela = new hash_bkt *[tamanho];

    for (int i = 0; i < tamanho; i++)
        tabela[i] = nullptr;
}

tabela_hash::~tabela_hash()
{
    for (int i = 0; i < tamanho_tbl; ++i)
    {
        if (tabela[i])
        {

            hash_bkt *tmp = tabela[i];
            hash_bkt *next;

            while (tmp)
            {
                next = tmp->prox;
                delete tmp;
                tmp = next;
            }
        }
    }

    delete[] tabela;
}

int tabela_hash::hash_function(std::string chave)
{
    return std::hash<std::string>()(chave) % tamanho_tbl;
}

hash_bkt* tabela_hash::inserir(std::string chave, void* elemento)
{
    int hash_chave = hash_function(chave);

    hash_bkt *obj = new hash_bkt(elemento, chave);

    if (tabela[hash_chave] == nullptr)
        tabela[hash_chave] = obj;
    else
    {
        hash_bkt* tmp = tabela[hash_chave];

        while(tmp->prox != nullptr)
            tmp = tmp->prox;

        tmp->prox = obj;
    }

    return obj;
}

hash_bkt* tabela_hash::pesquisar(std::string chave)
{
    int hash_chave = hash_function(chave);

    if (tabela[hash_chave] == nullptr)
        return nullptr;

    hash_bkt* tmp = tabela[hash_chave];

    while (true)
    {
        if (chave == tmp->chave)
            return tmp;

        if (tmp->prox != nullptr)
            tmp = tmp->prox;
        else
            return nullptr;
    }
}
