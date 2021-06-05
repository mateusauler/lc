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

#define exc_simples(N, M) \
class N : public erro_fonte \
{ \
public: \
	N(const int l) : erro_fonte(M, l) {} \
}

#define exc_lexema(N, M) \
class N : public erro_fonte \
{ \
public: \
	N(std::string lex, const int l) : erro_fonte(M, l) {} \
}

class erro_fonte : public std::exception
{
protected:
	std::string msg;

public:
	const int linha_erro;

	erro_fonte(std::string m, const int l) : msg(m), linha_erro(l) {}

	const char * what() const throw() { return msg.c_str(); }
};

// Lexico
exc_simples(char_invalido,         "caractere invalido.");
exc_lexema(lex_nao_identificado,   "lexema nao identificado [" + lex + "].");
exc_simples(eof_inesperado,        "fim de arquivo nao esperado.");

// Sintatico
exc_lexema(token_invalido,         "token nao esperado [" + lex + "].");

// Semantico
exc_lexema(id_nao_declarado,       "identificador nao declarado [" + lex + "].");
exc_lexema(id_ja_declarado,        "identificador ja declarado [" + lex + "].");
exc_lexema(classe_id_incompativel, "classe de identificador incompativel [" + lex + "].");
exc_simples(tipo_incompativel,     "tipos incompativeis.");
exc_simples(tam_vet_excede_max,    "tamanho do vetor excede o maximo permitido.");
