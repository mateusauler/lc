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

#include <sstream>

#include "lexer.h"
#include "excessoes.h"

// Caractere valido
#define CHAR_VALIDO(c) ((c == '\t' || c == '\n' || c == '\r' || c == EOF) || \
					   (c >= ' '   && c <= '"') || \
					   (c >= '\''  && c <= '[') || \
					   (c >= 'a'   && c <= '{') || \
					   (c == '$'   || c == '%') || \
					   (c == ']'   || c == '_') || \
					   (c == '}'))

#define LETRA(c)  ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
#define DIGITO(c) (c >= '0' && c <= '9')

lexer::~lexer()
{
	delete token_lido;
}

int lexer::get_linha() const
{
	return num_linha;
}

void lexer::proximo_token()
{
	estado_t estado = ES_INICIO;

	char c;
	bool backtrack = false;

	if (token_lido && token_lido->tipo_token == TK_EOF)
		return;

	delete token_lido;

	token_lido = new token_t();

	while (estado != ES_FIM)
	{
		c = fgetc(arq_fonte);

		// O unico estado que aceita EOF e o estado inicial
		if (c == EOF && estado != ES_INICIO)
			throw eof_inesperado(num_linha);

		// Caractere invalido
		if (!CHAR_VALIDO(c))
			throw char_invalido(num_linha);

		switch (estado)
		{
			// Simbolo inicial
			// Ignora espacos e novas linhas (a contabilizacao de linhas e feita depois)
			// Qualquer caractere que nao e especificado explicitamente, e considerado erro
			case ES_INICIO:
				// Limpa o lexema
				token_lido->lex.clear();
				switch(c)
				{
					case '_':
						estado = ES_ID_UNDERLINE;
						break;

					case '0':
						estado = ES_CONST_HEX_INICIO;
						break;

					case '\'':
						estado = ES_CONST_CHAR_INICIO;
						break;

					case '"':
						estado = ES_CONST_STR_INTERNO;
						break;

					case '/':
						estado = ES_OP_BARRA;
						break;

					case ':':
						estado = ES_OP_ATRIB_INICIO;
						break;

					case '<':
						estado = ES_OP_LT;
						break;

					case '>':
						estado = ES_OP_GT;
						break;

					case '{':
						token_lido->tipo_token = TK_GRU_A_CHA;
						estado = ES_FIM;
						break;

					case '}':
						token_lido->tipo_token = TK_GRU_F_CHA;
						estado = ES_FIM;
						break;

					case '[':
						token_lido->tipo_token = TK_GRU_A_COL;
						estado = ES_FIM;
						break;

					case ']':
						token_lido->tipo_token = TK_GRU_F_COL;
						estado = ES_FIM;
						break;

					case '(':
						token_lido->tipo_token = TK_GRU_A_PAR;
						estado = ES_FIM;
						break;

					case ')':
						token_lido->tipo_token = TK_GRU_F_PAR;
						estado = ES_FIM;
						break;

					case '%':
						token_lido->tipo_token = TK_OP_PORCENTO;
						estado = ES_FIM;
						break;

					case '*':
						token_lido->tipo_token = TK_OP_MUL;
						estado = ES_FIM;
						break;

					case '+':
						token_lido->tipo_token = TK_OP_MAIS;
						estado = ES_FIM;
						break;

					case '-':
						token_lido->tipo_token = TK_OP_MENOS;
						estado = ES_FIM;
						break;

					case ',':
						token_lido->tipo_token = TK_OP_VIRGULA;
						estado = ES_FIM;
						break;

					case ';':
						token_lido->tipo_token = TK_FIM_DECL;
						estado = ES_FIM;
						break;

					case '=':
						token_lido->tipo_token = TK_OP_EQ;
						estado = ES_FIM;
						break;

					case EOF:
						token_lido->tipo_token = TK_EOF;
						estado = ES_FIM;
						break;

					case ' ':
					case '\r':
					case '\n':
					case '\t':
						break;

					default:
						if      (LETRA(c))  estado = ES_ID_NOME;
						else if (DIGITO(c)) estado = ES_CONST_NUM;
						else throw lex_nao_identificado(token_lido->lex + c, num_linha);
						break;
				}
				break;

			// {_}+
			case ES_ID_UNDERLINE:
				if (LETRA(c) || DIGITO(c))
					estado = ES_ID_NOME;
				else if (c != '_') // Somente {_}+ e invalido
					throw lex_nao_identificado(token_lido->lex, num_linha);

				break;

			// {_}(LETRA | DIGITO){LETRA | DIGITO | _}
			case ES_ID_NOME:
				if (!LETRA(c) && !DIGITO(c) && c != '_')
				{
					token_lido->tipo_token = TK_ID;
					estado = ES_FIM;
					backtrack = true;
				}
				break;

			// 0
			case ES_CONST_HEX_INICIO:
				if (DIGITO(c)) // 0(0-9)
					estado = ES_CONST_HEX_NUM1;
				else if (c >= 'A' && c <= 'F') // 0(A-F)
					estado = ES_CONST_HEX_ALPHA1;
				else // 0
				{
					token_lido->tipo_token = TK_CONST;
					token_lido->tipo_constante = TP_INT;

					estado = ES_FIM;
					backtrack = true;
				}
				break;

			// 0(A-F)
			case ES_CONST_HEX_ALPHA1:
				if (DIGITO(c) || (c >= 'A' && c <= 'F')) // 0(A-F)(A-F | 0-9)
					estado = ES_CONST_HEX_ALPHA2;
				else
					throw lex_nao_identificado(token_lido->lex, num_linha);

				break;

			// 0(0-9)
			case ES_CONST_HEX_NUM1:
				if (DIGITO(c)) // 0(0-9)(0-9)
					estado = ES_CONST_HEX_NUM2;
				else if (c >= 'A' && c <= 'F') // 0(0-9)(A-F)
					estado = ES_CONST_HEX_ALPHA2;
				else
				{
					token_lido->tipo_token = TK_CONST;
					token_lido->tipo_constante = TP_INT;

					estado = ES_FIM;
					backtrack = true;
				}
				break;

			// 0 ((A-F)(A-F | 0-9) | (A-F | 0-9)(A-F))
			case ES_CONST_HEX_ALPHA2:
				if (c == 'h') // 0 ((A-F)(A-F | 0-9) | (A-F | 0-9)(A-F)) h
				{
					token_lido->tipo_token = TK_CONST;
					token_lido->tipo_constante = TP_CHAR;

					estado = ES_FIM;
				}
				else throw lex_nao_identificado(token_lido->lex, num_linha);

				break;

			// 0(0-9)(0-9)
			case ES_CONST_HEX_NUM2:
				if (DIGITO(c)) // 0(0-9)(0-9)(0-9)
					estado = ES_CONST_NUM;
				else if (c == 'h') // 0(0-9)(0-9)h
				{
					token_lido->tipo_token = TK_CONST;
					token_lido->tipo_constante = TP_CHAR;

					estado = ES_FIM;
				}
				else // 0(0-9)(0-9)
				{
					token_lido->tipo_token = TK_CONST;
					token_lido->tipo_constante = TP_INT;

					estado = ES_FIM;
					backtrack = true;
				}
				break;

			// {0-9}+
			case ES_CONST_NUM:
				if (!DIGITO(c))
				{
					token_lido->tipo_token = TK_CONST;
					token_lido->tipo_constante = TP_INT;

					estado = ES_FIM;
					backtrack = true;
				}
				break;

			// '
			case ES_CONST_CHAR_INICIO:
				switch (c)
				{
					case '\n':
					case '\r':
					case '\'':
						throw char_invalido(num_linha);

					default:
						estado = ES_CONST_CHAR_INTERNO;
						break;
				}
				break;

			// '(caractere)
			case ES_CONST_CHAR_INTERNO:
				if (c == '\'') // '(caractere)'
				{
					token_lido->tipo_token = TK_CONST;
					token_lido->tipo_constante = TP_CHAR;

					estado = ES_FIM;
				}
				else throw lex_nao_identificado(token_lido->lex, num_linha);
				break;

			// "{caractere}
			case ES_CONST_STR_INTERNO:
				switch (c)
				{
					case '"': // "{caractere}"
						token_lido->tipo_token = TK_CONST;
						token_lido->tipo_constante = TP_STR;
						estado = ES_FIM;
						break;

					case '$':
					case '\n':
					case '\r':
						throw lex_nao_identificado(token_lido->lex, num_linha);

					default:
						break;
				}
				break;

			// /
			case ES_OP_BARRA:
				if (c == '*') // /*
					estado = ES_COMENTARIO;
				else // /
				{
					token_lido->tipo_token = TK_OP_BARRA;

					backtrack = true;
					estado = ES_FIM;
				}
				break;

			// /*{qualquer caractere}
			case ES_COMENTARIO:
				if (c == '*')
					estado = ES_COMENTARIO_FIM;
				break;

			// /*{qualquer caractere}{*}+
			case ES_COMENTARIO_FIM:
				switch (c)
				{
					case '/': // Fim do comentario
						estado = ES_INICIO;
						break;

					case '*': // Ainda pode finalizar o comentario
						break;

					default: // Comentario continua
						estado = ES_COMENTARIO;
						break;
				}
				break;

			// :
			case ES_OP_ATRIB_INICIO:
				if(c == '=') // :=
				{
					token_lido->tipo_token = TK_OP_ATRIB;
					estado = ES_FIM;
				}
				else throw lex_nao_identificado(token_lido->lex, num_linha);
				break;

			// <
			case ES_OP_LT:
				switch (c)
				{
					case '=': // <=
						token_lido->tipo_token = TK_OP_LE;
						break;

					case '>': // <>
						token_lido->tipo_token = TK_OP_NE;
						break;

					default: // <
						token_lido->tipo_token = TK_OP_LT;
						backtrack = true;
						break;
				}

				estado = ES_FIM;

				break;

			// >
			case ES_OP_GT:
				switch (c)
				{
					case '=': // >=
						token_lido->tipo_token = TK_OP_GE;
						break;

					default: // >
						token_lido->tipo_token = TK_OP_GT;
						backtrack = true;
						break;
				}

				estado = ES_FIM;

				break;

			default:
				break;
		} // switch (estado)

		// Caso nao tenha lido um caractere que nao pertence ao lexema atual
		if (!backtrack)
		{
			if (c == '\n')
				num_linha++;

			token_lido->lex += c;
		}
	}

	if (backtrack)
		fseek(arq_fonte, -1, SEEK_CUR);

	int lex_len = token_lido->lex.length();

	if (token_lido->tipo_token == TK_ID)
	{
		// Se o lexema tiver mais de 32 caracteres, ignora a partir do 33
		if (lex_len > 32)
			token_lido->lex.erase(32, lex_len - 32);

		token_lido->simbolo = tbl_simbolos.buscar(token_lido->lex);

		if (!token_lido->simbolo) // Caso seja um ID novo
			token_lido->simbolo = tbl_simbolos.inserir(token_lido->tipo_token, token_lido->lex);
		else
		{
			// Pode ser uma palavra reservada
			token_lido->tipo_token = token_lido->simbolo->tipo_token;

			// Se estiver na tabela de simbolos e for constante, e boleano
			if (token_lido->tipo_token == TK_CONST)
				token_lido->tipo_constante = TP_BOOL;
		}
	}

	if (token_lido->tipo_token == TK_CONST)
	{
		switch (token_lido->tipo_constante)
		{
			case TP_STR:
				token_lido->tam_constante = lex_len - 1;
				break;

			default:
				token_lido->tam_constante = 0;
				break;
		}
	}
}
