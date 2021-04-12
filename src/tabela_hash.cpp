#include <cstring>
#include "tabela_hash.h"

tabela_hash::tabela_hash(int tamanho)
{
    if (tamanho <= 0)
        tamanho = 128;

    tamanho_tbl = tamanho;
    tabela = new hash_bkt *[tamanho];

    // Todos os itens da tabela sao um ponteiro para void
    memset(tabela, 0, tamanho * sizeof(void*));
}

tabela_hash::~tabela_hash()
{
    // Limpa todos os membros de cada entrada da tabela
    for (int i = 0; i < tamanho_tbl; ++i)
    {
        if (tabela[i])
        {

            hash_bkt *tmp = tabela[i];
            hash_bkt *prox;

            while (tmp)
            {
                prox = tmp->prox;
                delete tmp;
                tmp = prox;
            }
        }
    }

    delete[] tabela;
}

// Calcula a hash de uma string
int tabela_hash::calcula_hash(std::string chave)
{
    return std::hash<std::string>()(chave) % tamanho_tbl;
}

// Insere um item com conteudo "elemento" e chava "chave"
// E retorna um ponteiro para o objeto encapsulado
hash_bkt* tabela_hash::inserir(std::string chave, void* elemento)
{
    // Calcula a hash da chave
    int hash_chave = calcula_hash(chave);

    // Encapsula o elemento em um "bucket"
    hash_bkt *obj = new hash_bkt(elemento, chave);

    if (tabela[hash_chave] == nullptr) // Caso nao exista, sera a nova cabeca
        tabela[hash_chave] = obj;
    else
    {
        // Navega ate o final da lista para inserir a entrada

        hash_bkt* tmp = tabela[hash_chave];

        while(tmp->prox != nullptr)
            tmp = tmp->prox;

        tmp->prox = obj;
    }

    return obj;
}

// Busca um elemento com a chave "chave" na tabela
// Retorna um ponteiro para o registro na tabela
hash_bkt* tabela_hash::buscar(std::string chave)
{
    // Calcula a hash desta chave
    int hash_chave = calcula_hash(chave);

    // Caso a entrada na posicao esprada esteja vazia, o elemento buscado nao existe
    if (tabela[hash_chave] == nullptr)
        return nullptr;

    hash_bkt* tmp = tabela[hash_chave];

    // Busca ate a chave da entrada igualar a buscada, ou chegar ao final da lista
    while (true)
    {
        if (chave == tmp->chave)
            return tmp;

        if (tmp->prox == nullptr)
            return nullptr;

        tmp = tmp->prox;
    }
}
