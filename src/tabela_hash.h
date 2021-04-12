#pragma once

#include <sstream>

struct hash_bkt
{
    void *elemento;

    hash_bkt *prox;

    std::string chave;

    hash_bkt()
    {
        this->prox = NULL;
    }

    hash_bkt(void* e, std::string c)
    {
        this->chave = c;
        this->elemento = e;
        this->prox = NULL;
    }
};

class tabela_hash
{

protected:
    hash_bkt **tabela;
    int tamanho_tbl;

public:
    tabela_hash(int tamanho);
    ~tabela_hash();
    int calcula_hash(std::string chave);
    hash_bkt* inserir(std::string chave, void* elemento);
    hash_bkt* buscar(std::string chave);

};
