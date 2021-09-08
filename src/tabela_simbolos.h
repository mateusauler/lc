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

#include "tabela_hash.h"
#include "enums.h"

struct registro_tabela_simbolos
{
	tipo_token_t tipo_token;
	std::string lexema;
	classe_t classe = CL_NULL;
	tipo_dados_t tipo = TP_NULL;
	int tam = 0;
	unsigned long endereco = 0;

	registro_tabela_simbolos() {}

	registro_tabela_simbolos(tipo_token_t t, std::string l)
		: tipo_token(t), lexema(l) {}
};

class tabela_simbolos : public tabela_hash<registro_tabela_simbolos>
{

public:

	tabela_simbolos();

	/*
	 * Insere um registro na tabela.
	 * Retorna um ponteiro para o registro inserido.
	 */
	registro_tabela_simbolos* inserir(tipo_token_t tipo_token, std::string lexema);

	/*
	 * Busca um elemento com a chave "lexema" na tabela.
	 * Retorna um ponteiro para o registro encontrado ou NULL, caso nao tenha sido encontrado.
	 */
	registro_tabela_simbolos* buscar(std::string lexema);

};
