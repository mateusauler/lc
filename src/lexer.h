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

#include "tabela_simbolos.h"

struct token_t
{
	tipo_token_t tipo_token;
	std::string lex = "";

	registro_tabela_simbolos* simbolo = nullptr;

	tipo_dados_t tipo_constante = TP_NULL;
	int tam_constante = 0;
};

class lexer
{

public:

	tabela_simbolos tbl_simbolos;

	lexer(FILE *f) : arq_fonte(f) {}
	~lexer();

	// Retorna o numero da linha atual do arquivo fonte
	int get_linha() const;

	// Le o proximo token do arquivo fonte
	void proximo_token();

	// Ultimo token gerado pelo analisador lexico
	token_t *token_lido = nullptr;

protected:

	int num_linha = 1; // Numero da linha atual do arquivo fonte

private:

	FILE *arq_fonte;

};
