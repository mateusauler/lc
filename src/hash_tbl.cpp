#include "hash_tbl.h"

hash_tbl::hash_tbl(int tamanho)
{
    if (tamanho <= 0)
        tamanho = 128;

    hash_bkt bucket[tamanho];
    this->tamanho_tbl = tamanho;
    this->tabela = (hash_bkt**)&bucket;

    for (int i = 0; i < tamanho; i++)
        this->tabela[i] = NULL;
}

int hash_tbl::hash_function(string chave)
{
    return chave[0] % this->tamanho_tbl;
}

hash_bkt* hash_tbl::inserir(string chave, void* elemento)
{
    int hash_chave = this->hash_function(chave);
    
    hash_bkt *obj = new hash_bkt(elemento, chave);

    if (this->tabela[hash_chave] == NULL)
        this->tabela[hash_chave] = obj;    
    else
    {
        hash_bkt* tmp = this->tabela[hash_chave];

        while(tmp->prox != NULL)
            tmp = tmp->prox;

        tmp->prox = obj;
    }

    return obj;
}

hash_bkt* hash_tbl::pesquisar(string chave)
{
    int hash_chave = this->hash_function(chave);
    
    if(this->tabela[hash_chave] == NULL)
        return NULL;

    hash_bkt* tmp = this->tabela[hash_chave];

    while(true)
    {
        if (chave == tmp->chave)
            return tmp;

        if (tmp->prox != NULL)
            tmp = tmp->prox;
        else 
            return NULL;
    }
}

list<hash_bkt> hash_tbl::listar_elementos()
{
    list<hash_bkt> *l = new list<hash_bkt>();

    for (int i = 0; i < tamanho_tbl; i++)
    {
        hash_bkt *tmp = tabela[i];

        while (tmp != NULL)
        {
            l->push_back(*tmp);
            tmp = tmp->prox;
        }
    }

    return *l;
}