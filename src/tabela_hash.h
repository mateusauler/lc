#pragma once

#include <sstream>
#include <list>

struct hash_bkt
{
    void *elemento;

    hash_bkt *prox;

    std::string chave;

    hash_bkt() {
        this->prox = NULL;
    }

    hash_bkt(void* e, std::string c) {
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
    int hash_function(std::string chave);
    hash_bkt* inserir(std::string chave, void* elemento);
    hash_bkt* pesquisar(std::string chave);

};
