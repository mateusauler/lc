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

#include "tabela_simbolos.h"

tabela_simbolos::tabela_simbolos()
{
	// Inicializa a tabela de simbolos com as palavras reservadas
	inserir(TK_RES_FINAL,   "final");
	inserir(TK_RES_INT,     "int");
	inserir(TK_RES_CHAR,    "char");
	inserir(TK_RES_BOOLEAN, "boolean");
	inserir(TK_RES_IF,      "if");
	inserir(TK_RES_ELSE,    "else");
	inserir(TK_RES_THEN,    "then");
	inserir(TK_RES_FOR,     "for");
	inserir(TK_RES_AND,     "and");
	inserir(TK_RES_OR,      "or");
	inserir(TK_RES_NOT,     "not");
	inserir(TK_CONST,       "FALSE");
	inserir(TK_CONST,       "TRUE");
	inserir(TK_RES_WRITE,   "write");
	inserir(TK_RES_WRITELN, "writeln");
	inserir(TK_RES_READLN,  "readln");
	inserir(TK_RES_MAIN,    "main");
}

registro_tabela_simbolos* tabela_simbolos::inserir(tipo_token_t tipo_token, std::string lexema)
{
	registro_tabela_simbolos* obj = new registro_tabela_simbolos(tipo_token, lexema);

	hash_bkt<registro_tabela_simbolos>* retorno = tabela_hash::inserir(lexema, obj);

	return (registro_tabela_simbolos*)retorno->elemento;
}

registro_tabela_simbolos* tabela_simbolos::buscar(std::string lexema)
{
	hash_bkt<registro_tabela_simbolos>* retorno = tabela_hash::buscar(lexema);

	if (retorno == NULL) return NULL;

	return (registro_tabela_simbolos*)retorno->elemento;
}
