#include <sstream>

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
    public:
    hash_bkt **tabela;
    int tamanho_tbl;

    int hash_function(string chave)
    {
        return chave[0] % this->tamanho_tbl;
    }

    hash_tbl()
    {
        hash_tbl(128);
    }

    hash_tbl(int tamanho)
    {
        hash_bkt bucket[tamanho];
        this->tamanho_tbl = tamanho;
        this->tabela = (hash_bkt**)&bucket;

        for (int i = 0; i < tamanho; i++)
            this->tabela[i] = NULL;
    }

    hash_bkt* inserir(string chave, void* elemento)
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

    hash_bkt* pesquisar(string chave)
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

    private:

};