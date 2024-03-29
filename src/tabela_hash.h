// Copyright 2021 - The lc compiler developers
//
// This file is part of The lc compiler.
//
// The lc compiler is free software: you can redistribute it and/or modify it
// under the terms of the GNU Affero General Public License version 3 as published by
// the Free Software Foundation.
//
// The lc compiler is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with The lc compiler. If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <string>
#include <cstring>

template <typename T>
struct hash_bkt
{
	T *elemento; // Conteudo armazenado
	hash_bkt *prox = nullptr;
	std::string chave; // Chave do elemento

	hash_bkt() {}

	hash_bkt(T* e, std::string c)
		: elemento(e), chave(c) {}
};

template<typename T>
class tabela_hash
{

protected:

	hash_bkt<T> **tabela;
	const unsigned int tamanho_tbl; // Numero de buckets da tabela

public:

	tabela_hash(const unsigned int tamanho = 128);

	~tabela_hash();

	// Calcula a hash de uma string
	int calcula_hash(std::string chave);

	/*
	 * Insere um item com conteudo "elemento" e chave "chave".
	 * Retorna um ponteiro para o objeto encapsulado.
	 */
	hash_bkt<T>* inserir(std::string chave, T* elemento);

	/*
	 * Busca um elemento com a chave "chave" na tabela.
	 * Retorna um ponteiro para o registro na tabela.
	 */
	hash_bkt<T>* buscar(std::string chave);

};

template<typename T>
tabela_hash<T>::tabela_hash(const unsigned int tamanho) : tamanho_tbl(tamanho)
{
	tabela = new hash_bkt<T> *[tamanho];

	// Todos os itens da tabela sao inicialmente um ponteiro nulo
	memset(tabela, 0, tamanho * sizeof(void*));
}

template<typename T>
tabela_hash<T>::~tabela_hash()
{
	// Limpa todos os membros de cada entrada da tabela
	for (unsigned int i = 0; i < tamanho_tbl; ++i)
	{
		if (tabela[i])
		{
			hash_bkt<T> *tmp = tabela[i];
			hash_bkt<T> *prox;

			while (tmp)
			{
				prox = tmp->prox;
				delete tmp->elemento;
				delete tmp;
				tmp = prox;
			}
		}
	}

	delete[] tabela;
}

template<typename T>
int tabela_hash<T>::calcula_hash(std::string chave)
{
	return std::hash<std::string>()(chave) % tamanho_tbl;
}

template<typename T>
hash_bkt<T>* tabela_hash<T>::inserir(std::string chave, T *elemento)
{
	// Calcula a hash da chave
	int hash_chave = calcula_hash(chave);

	// Encapsula o elemento em um "bucket"
	hash_bkt<T> *obj = new hash_bkt<T>(elemento, chave);

	if (tabela[hash_chave] == nullptr) // Caso nao exista, sera a nova cabeca
		tabela[hash_chave] = obj;
	else
	{
		// Navega ate o final da lista para inserir a entrada

		hash_bkt<T>* tmp = tabela[hash_chave];

		while(tmp->prox != nullptr)
			tmp = tmp->prox;

		tmp->prox = obj;
	}

	return obj;
}

template<typename T>
hash_bkt<T>* tabela_hash<T>::buscar(std::string chave)
{
	// Calcula a hash desta chave
	int hash_chave = calcula_hash(chave);

	// Caso a entrada na posicao esprada esteja vazia, o elemento buscado nao existe
	if (tabela[hash_chave] == nullptr)
		return nullptr;

	hash_bkt<T>* tmp = tabela[hash_chave];

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
