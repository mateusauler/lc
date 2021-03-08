#ifndef HASH_TBL_H
#define HASH_TBL_H

#include <sstream>
#include <list>

using namespace std;

typedef struct hash_bkt hash_bkt;
struct hash_bkt {
	void *elemento;
	
    hash_bkt *prox;
    
    string chave;
    
    hash_bkt() {
        this->prox = NULL;
    }
    
    hash_bkt(void* e, string c) {
        this->chave = c;
        this->elemento = e;
        this->prox = NULL;
    }
};

class hash_tbl
{

protected:
    hash_bkt **tabela;
    int tamanho_tbl;

public:
    hash_tbl(int tamanho);
    int hash_function(string chave);
    hash_bkt* inserir(string chave, void* elemento);
    hash_bkt* pesquisar(string chave);
    list<hash_bkt> listar_elementos();

};

#endif