#include <sstream>

#include "parser.h"
#include "excessoes.h"

static int byte_tipo(tipo_dados_t tipo)
{
	switch(tipo)
	{
		case TP_INT:
		case TP_BOOL:
			return 2;

		case TP_CHAR:
			return 1;

		default:
			return 0;
	}
}

static std::string converte_hex(int valor)
{
	std::stringstream stream;
	stream << 0 << std::uppercase << std::hex << valor;
	return stream.str() + 'h';
}

static int aloca(int bytes)
{
	static int endereco = 0x4000;
	endereco += bytes;
	return endereco - bytes;
}

static std::string novo_rotulo()
{
	static int rotulo = 0;
	return "R" + std::to_string(rotulo) + ":";
}

static int end_tmp = 0;

static int novo_tmp(int bytes)
{
	end_tmp += bytes;
	return end_tmp - bytes;
}

static std::string gera_alocacao(tipo_dados_t tipo, int tamanho, std::string dado = "", std::string nome = "", bool cabecalho = false)
{
	std::string codigo;

	if (cabecalho)
		codigo += "dseg SEGMENT PUBLIC\n";

	codigo += tipo == TP_CHAR || tipo == TP_STR
		? "	byte "
		: "	sword ";

	if (dado.empty())
		dado = tamanho ? converte_hex(tamanho) + " DUP(?)" : "?";

	codigo += dado + (nome.empty() ? "" : "		; " + nome) + "\n";

	if (cabecalho)
		codigo += "dseg ENDS\n";

	return codigo;
}

void parser::exec_parser()
{
	std::string programa;
	proximo_token();
	prog(programa);

	if (!arq_saida.empty())
	{
		FILE *f = fopen(arq_saida.c_str(), "w");
		fprintf(f, "%s", programa.c_str());
		fclose(f);
	}
}

void parser::consome_token(tipo_token_t token_esperado)
{
	if      (token_lido->tipo_token == token_esperado) proximo_token();
	else if (token_lido->tipo_token == TK_EOF) throw eof_inesperado(num_linha);
	else                                       throw token_invalido(token_lido->lex, num_linha);
}

void parser::prog(std::string& destino)
{
	// {DecVar|DecConst} main BlocoCmd EOF

	destino +=
		"sseg SEGMENT STACK\n"
		"	byte 4000h DUP(?)		; Pilha\n"
		"sseg ENDS\n"
		"\n"
		"dseg SEGMENT PUBLIC\n"
		"	byte 4000h DUP(?)		; Temporarios\n";

	// {DecVar|DecConst}
	while (token_lido->tipo_token != TK_RES_MAIN)
	{
		if (token_lido->tipo_token == TK_RES_FINAL) dec_const(destino); // final
		else                                        dec_var(destino);   // (int | char | boolean)
	}

	destino +=
		"dseg ENDS\n"
		"\n"
		"cseg SEGMENT PUBLIC\n"
		"	ASSUME CS:cseg, DS:dseg\n"
		"\n"
		"strt:\n"
		"	mov AX, dseg\n"
		"	mov DS, AX\n"
		"\n";

	consome_token(TK_RES_MAIN); // main
	bloco_cmd(destino);
	consome_token(TK_EOF); // EOF

	destino +=
		"\n"
		"	mov AH, 4Ch\n"
		"	int 21h\n"
		"cseg ENDS\n"
		"END strt";
}

void parser::dec_var(std::string& destino)
{
	// (int | char | boolean) Var {, Var} ;
	tipo_dados_t tipo;

	// (int | char | boolean)
	switch (token_lido->tipo_token)
	{
		case TK_RES_INT: // int
			consome_token(TK_RES_INT);
			// Ação 1
			tipo = TP_INT;
			break;

		case TK_RES_CHAR: // char
			consome_token(TK_RES_CHAR);
			// Ação 2
			tipo = TP_CHAR;
			break;

		default: // boolean
			consome_token(TK_RES_BOOLEAN);
			// Ação 3
			tipo = TP_BOOL;
			break;
	}

	// Ação 4
	var(tipo, destino);

	// {, Var}
	while(token_lido->tipo_token == TK_OP_VIRGULA)
	{
		consome_token(TK_OP_VIRGULA); // ,
		// Ação 4
		var(tipo, destino);
	}

	consome_token(TK_FIM_DECL); // ;
}

void parser::dec_const(std::string& destino)
{
	// final ID = [-] CONST ;

	consome_token(TK_RES_FINAL); // final

	registro_tabela_simbolos *rt = token_lido->simbolo;
	std::string lex = token_lido->lex;
	int linha_erro = num_linha;
	bool nega = false;

	consome_token(TK_ID); // ID

	// Ação 5
	if (rt->classe != CL_NULL)
		throw id_ja_declarado(lex, linha_erro);

	rt->classe = CL_CONST;

	consome_token(TK_OP_EQ); // =

	// [-]
	if (token_lido->tipo_token == TK_OP_MENOS)
	{
		consome_token(TK_OP_MENOS);
		nega = true;
	}

	tipo_dados_t tipo_constante = token_lido->tipo_constante;
	linha_erro = num_linha;

	std::string lex_const = token_lido->lex;

	consome_token(TK_CONST); // CONST

	// Ação 6
	rt->tipo = tipo_constante;

	if (nega && rt->tipo != TP_INT)
		throw tipo_incompativel(linha_erro);

	if (rt->tipo == TP_STR || rt->tipo == TP_NULL)
		throw tipo_incompativel(linha_erro);

	rt->endereco = aloca(byte_tipo(rt->tipo));

	std::string valor;

	if (rt->tipo == TP_BOOL)
		valor = converte_hex(lex_const == "TRUE");
	else
		valor = (nega ? "-" : "") + lex_const;

	destino += gera_alocacao(rt->tipo, 0, valor, lex);

	consome_token(TK_FIM_DECL); // ;
}

void parser::var(tipo_dados_t tipo, std::string& destino)
{
	// ID [:= [-] CONST | "[" CONST "]" ]

	registro_tabela_simbolos *rt = token_lido->simbolo;
	std::string lex = token_lido->lex;
	std::string lex_id = lex;
	std::string lex_const;
	std::string valor = "";

	tipo_dados_t tipo_constante;

	int valor_array;
	int linha_erro = num_linha;
	bool nega = false;

	consome_token(TK_ID); // ID

	// Ação 7
	if (rt->classe != CL_NULL)
		throw id_ja_declarado(lex, linha_erro);

	rt->classe = CL_VAR;
	rt->tipo = tipo;

	switch (token_lido->tipo_token)
	{
		case TK_OP_ATRIB: // := [-] CONST

			consome_token(TK_OP_ATRIB); // :=

			// [-]
			if (token_lido->tipo_token == TK_OP_MENOS)
			{
				consome_token(TK_OP_MENOS);
				nega = true;
			}

			tipo_constante = token_lido->tipo_constante;
			linha_erro = num_linha;

			lex_const = token_lido->lex;

			consome_token(TK_CONST); // CONST

			// Ação 8

			if (nega && tipo_constante != TP_INT)
				throw tipo_incompativel(linha_erro);

			if (tipo_constante != tipo)
				throw tipo_incompativel(linha_erro);

			rt->endereco = aloca(byte_tipo(rt->tipo));
			if (rt->tipo == TP_BOOL)
				valor = converte_hex(lex_const == "TRUE");
			else
				valor = (nega ? "-" : "") + lex_const;

			destino += gera_alocacao(tipo, 0, valor, lex_id);

			break;

		case TK_GRU_A_COL: // "[" CONST "]"

			consome_token(TK_GRU_A_COL); // [

			tipo_constante = token_lido->tipo_constante;
			lex = token_lido->lex;
			linha_erro = num_linha;

			consome_token(TK_CONST); // CONST

			// Ação 9
			valor_array = std::atoi(lex.c_str());

			if (tipo_constante != TP_INT)
				throw tipo_incompativel(linha_erro);

			if (valor_array == 0)
				throw tipo_incompativel(linha_erro);

			if (valor_array * byte_tipo(tipo) > 8192)
				throw tam_vet_excede_max(linha_erro);

			rt->tam = valor_array;

			consome_token(TK_GRU_F_COL); // ]

			rt->endereco = aloca(byte_tipo(rt->tipo) * rt->tam);
			destino += gera_alocacao(tipo, rt->tam, "", lex_id);

			break;

		default:
			rt->endereco = aloca(byte_tipo(rt->tipo));
			destino += gera_alocacao(tipo, 0, "", lex_id);
			break;
	}
}

void parser::bloco_cmd(std::string& destino)
{
	// "{" {CmdT} "}"

	consome_token(TK_GRU_A_CHA); // {

	// {CmdT}
	while (token_lido->tipo_token != TK_GRU_F_CHA)
		cmd_t(destino);

	consome_token(TK_GRU_F_CHA); // }
}

void parser::cmd_s(std::string& destino)
{
	// ID [ "[" Exp "]" ] := Exp
	// readln "(" ID [ "[" Exp "]" ] ")"
	// (write | writeln) "(" Exp {, Exp} ")"

	registro_tabela_simbolos *rt;
	std::string lex;

	tipo_dados_t tipo_exp;
	int tamanho_exp, tamanho, linha_erro;

	switch (token_lido->tipo_token)
	{
		case TK_ID: // ID [ "[" Exp "]" ] := Exp

			rt = token_lido->simbolo;
			lex = token_lido->lex;
			linha_erro = num_linha;

			consome_token(TK_ID); // ID

			// Ação 10
			if (rt->classe == CL_NULL)  throw id_nao_declarado(lex, linha_erro);
			if (rt->classe == CL_CONST) throw classe_id_incompativel(lex, linha_erro);

			tamanho = rt->tam;

			// [ "[" Exp "]" ]
			if (token_lido->tipo_token == TK_GRU_A_COL)
			{
				consome_token(TK_GRU_A_COL); // [

				linha_erro = num_linha;

				exp(tipo_exp, tamanho_exp, destino);

				// Ação 11
				if (tipo_exp != TP_INT || tamanho_exp > 0 || tamanho == 0)
					throw tipo_incompativel(linha_erro);

				tamanho = 0;

				consome_token(TK_GRU_F_COL); // ]
			}

			consome_token(TK_OP_ATRIB); // :=

			linha_erro = num_linha;

			exp(tipo_exp, tamanho_exp, destino);

			// Ação 12
			if (tipo_exp != rt->tipo)
			{
				if (rt->tipo != TP_CHAR && tipo_exp != TP_STR)
					throw tipo_incompativel(linha_erro);

				if (tamanho < tamanho_exp)
					throw tipo_incompativel(linha_erro);
			}
			else if (tamanho > 0)
			{
				if (rt->tipo == TP_CHAR)
				{
					if (tamanho_exp == 0)
						throw tipo_incompativel(linha_erro);

					if (tamanho < tamanho_exp)
						throw tipo_incompativel(linha_erro);
				}
				else
					throw tipo_incompativel(linha_erro);
			}
			else if (tamanho_exp > 0)
				throw tipo_incompativel(linha_erro);

			break;

		case TK_RES_READLN: // readln "(" ID [ "[" Exp "]" ] ")"

			consome_token(TK_RES_READLN); // readln
			consome_token(TK_GRU_A_PAR);  // (

			rt = token_lido->simbolo;
			linha_erro = num_linha;

			consome_token(TK_ID); // ID

			// Ação 13
			if (rt->classe == CL_NULL)  throw id_nao_declarado(lex, linha_erro);
			if (rt->classe == CL_CONST) throw classe_id_incompativel(lex, linha_erro);
			if (rt->tipo == TP_BOOL)    throw tipo_incompativel(linha_erro);

			tamanho = rt->tam;

			// [ "[" Exp "]" ]
			if (token_lido->tipo_token == TK_GRU_A_COL)
			{
				consome_token(TK_GRU_A_COL); // [

				linha_erro = num_linha;

				exp(tipo_exp, tamanho_exp, destino);

				// Ação 14
				if (tipo_exp != TP_INT || tamanho_exp > 0 || tamanho == 0)
					throw tipo_incompativel(linha_erro);

				tamanho = 0;

				consome_token(TK_GRU_F_COL); // ]
			}

			// Ação 32
			if (tamanho > 0 && rt->tipo != TP_CHAR)
				throw tipo_incompativel(linha_erro);

			consome_token(TK_GRU_F_PAR); // )
			break;

		default: // (write | writeln) "(" Exp {, Exp} ")"

			if (token_lido->tipo_token == TK_RES_WRITE) consome_token(TK_RES_WRITE);   // write
			else                                        consome_token(TK_RES_WRITELN); // writeln

			consome_token(TK_GRU_A_PAR); // (

			linha_erro = num_linha;

			exp(tipo_exp, tamanho_exp, destino);

			// Ação 33
			if (tipo_exp == TP_BOOL || (tamanho_exp > 0 && tipo_exp == TP_INT))
				throw tipo_incompativel(linha_erro);

			// {, Exp}
			while (token_lido->tipo_token == TK_OP_VIRGULA) // ,
			{
				consome_token(TK_OP_VIRGULA);

				linha_erro = num_linha;

				exp(tipo_exp, tamanho_exp, destino);

				// Ação 34
				if (tipo_exp == TP_BOOL || (tamanho_exp > 0 && tipo_exp == TP_INT))
					throw tipo_incompativel(linha_erro);
			}

			consome_token(TK_GRU_F_PAR); // )
			break;
	}
}

void parser::cmd_for(std::string& destino)
{
	// for "(" [Cmd {, Cmd}] ; Exp ; [Cmd {, Cmd}] ")" (CmdT | BlocoCmd)

	tipo_dados_t tipo_exp;
	int tamanho_exp;

	consome_token(TK_RES_FOR);   // for
	consome_token(TK_GRU_A_PAR); // (

	// [Cmd {, Cmd}]
	if (token_lido->tipo_token != TK_FIM_DECL)
	{
		cmd(destino);

		// {, Cmd}
		while (token_lido->tipo_token == TK_OP_VIRGULA)
		{
			consome_token(TK_OP_VIRGULA); // ,
			cmd(destino);
		}
	}
	consome_token(TK_FIM_DECL); // ;

	int linha_erro = num_linha;

	exp(tipo_exp, tamanho_exp, destino);

	// Ação 15
	if (tipo_exp != TP_BOOL || tamanho_exp > 0)
		throw tipo_incompativel(linha_erro);

	consome_token(TK_FIM_DECL); // ;

	// [Cmd {, Cmd}]
	if (token_lido->tipo_token != TK_GRU_F_PAR)
	{
		cmd(destino);

		// {, Cmd}
		while (token_lido->tipo_token == TK_OP_VIRGULA)
		{
			consome_token(TK_OP_VIRGULA); // ,
			cmd(destino);
		}
	}

	consome_token(TK_GRU_F_PAR); // )

	// (CmdT | BlocoCmd)
	if (token_lido->tipo_token == TK_GRU_A_CHA) bloco_cmd(destino);
	else                                        cmd_t(destino);
}

void parser::cmd_if(std::string& destino)
{
	// if "(" Exp ")" then (CmdT | BlocoCmd) [else (CmdT | BlocoCmd)]

	tipo_dados_t tipo_exp;
	int tamanho_exp;

	consome_token(TK_RES_IF);    // if
	consome_token(TK_GRU_A_PAR); // (

	int linha_erro = num_linha;

	exp(tipo_exp, tamanho_exp, destino);

	// Ação 16
	if (tipo_exp != TP_BOOL || tamanho_exp > 0)
		throw tipo_incompativel(linha_erro);

	consome_token(TK_GRU_F_PAR); // )

	consome_token(TK_RES_THEN);  // then

	// (CmdT | BlocoCmd)
	if (token_lido->tipo_token == TK_GRU_A_CHA) bloco_cmd(destino);
	else                                        cmd_t(destino);

	// [else (CmdT | BlocoCmd)]
	if (token_lido->tipo_token == TK_RES_ELSE)
	{
		consome_token(TK_RES_ELSE); // else

		// (CmdT | BlocoCmd)
		if (token_lido->tipo_token == TK_GRU_A_CHA) bloco_cmd(destino);
		else                                        cmd_t(destino);
	}

}

void parser::cmd_t(std::string& destino)
{
	// [CmdS] ; | CmdFor | CmdIf

	switch (token_lido->tipo_token)
	{
		case TK_RES_FOR: // for
			cmd_for(destino);
			break;

		case TK_RES_IF: // if
			cmd_if(destino);
			break;

		default:
			if (token_lido->tipo_token != TK_FIM_DECL) // [CmdS]
				cmd_s(destino);

			consome_token(TK_FIM_DECL); // ;
			break;
	}
}

void parser::cmd(std::string& destino)
{
	// CmdS | CmdFor | CmdIf

	switch (token_lido->tipo_token)
	{
		case TK_RES_FOR: // for
			cmd_for(destino);
			break;

		case TK_RES_IF: // if
			cmd_if(destino);
			break;

		default:
			cmd_s(destino);
			break;
	}
}

void parser::exp(tipo_dados_t &tipo, int &tamanho, std::string& destino)
{
	// Soma [(=|<>|>|<|>=|<=) Soma]

	tipo_dados_t tipo_soma;
	int tamanho_soma;
	int linha_erro;

	tipo_token_t operador;

	// Ação 17
	soma(tipo, tamanho, destino);

	// [(=|<>|>|<|>=|<=) Soma]
	switch (token_lido->tipo_token)
	{
		case TK_OP_EQ: // =
		case TK_OP_NE: // <>
		case TK_OP_GT: // >
		case TK_OP_LT: // <
		case TK_OP_GE: // >=
		case TK_OP_LE: // <=

			operador = token_lido->tipo_token;

			consome_token(token_lido->tipo_token); // (=|<>|>|<|>=|<=)

			linha_erro = num_linha;

			soma(tipo_soma, tamanho_soma, destino);

			// Ação 18
			if (tipo != tipo_soma)
			{
				if ((tipo == TP_CHAR && tipo_soma == TP_STR) ||
					(tipo == TP_STR  && tipo_soma == TP_CHAR))
				{
					if (tamanho == 0 || tamanho_soma == 0 || operador != TK_OP_EQ)
						throw tipo_incompativel(linha_erro);
				}
				else throw tipo_incompativel(linha_erro);
			}
			else if (tamanho > 0 || tamanho_soma > 0)
			{
				if (tipo == TP_CHAR)
				{
					if (tamanho == 0 || tamanho_soma == 0 || operador != TK_OP_EQ)
						throw tipo_incompativel(linha_erro);
				}
				else throw tipo_incompativel(linha_erro);
			}

			tipo = TP_BOOL;
			tamanho = 0;

		break;

		default:
			break;
	}
}

void parser::soma(tipo_dados_t &tipo, int &tamanho, std::string& destino)
{
	// [-] Termo {(+|-|or) Termo}

	tipo_dados_t tipo_termo;
	int tamanho_termo;
	bool nega = false;

	tipo_token_t operador;

	// [-]
	if (token_lido->tipo_token == TK_OP_MENOS)
	{
		consome_token(TK_OP_MENOS);
		nega = true;
	}

	int linha_erro;

	// Ação 19
	linha_erro = num_linha;
	termo(tipo, tamanho, destino);
	if (nega && (tipo != TP_INT || tamanho > 0)) throw tipo_incompativel(linha_erro);

	// {(+|-|or) Termo}
	while (true)
	{
		switch (token_lido->tipo_token)
		{
			case TK_OP_MAIS:  // +
			case TK_OP_MENOS: // -
			case TK_RES_OR:   // or

				operador = token_lido->tipo_token;

				consome_token(token_lido->tipo_token); // (+|-|or)

				linha_erro = num_linha;

				termo(tipo_termo, tamanho_termo, destino);

				// Ação 20
				if (tipo != tipo_termo)
					throw tipo_incompativel(linha_erro);

				if (tamanho_termo != 0 || tamanho != 0)
					throw tipo_incompativel(linha_erro);

				if (operador == TK_RES_OR)
				{
					if (tipo != TP_BOOL)
						throw tipo_incompativel(linha_erro);
				}
				else if (tipo != TP_INT)
					throw tipo_incompativel(linha_erro);

				tamanho = 0;

				break;

			default:
				return;
		}
	}
}

void parser::termo(tipo_dados_t &tipo, int &tamanho, std::string& destino)
{
	// Fator {(*|/|%|and) Fator}

	tipo_dados_t tipo_fator;
	int tamanho_fator;

	tipo_token_t operador;

	int linha_erro;

	// Ação 21
	fator(tipo, tamanho, destino);

	// {(*|/|%|and) Fator}
	while (true)
	{
		switch (token_lido->tipo_token)
		{
			case TK_OP_MUL:      // *
			case TK_OP_BARRA:    // /
			case TK_OP_PORCENTO: // %
			case TK_RES_AND:     // and

				operador = token_lido->tipo_token;

				consome_token(token_lido->tipo_token); // (*|/|%|and)

				linha_erro = num_linha;

				fator(tipo_fator, tamanho_fator, destino);

				// Ação 22
				if (tipo != tipo_fator)
					throw tipo_incompativel(linha_erro);

				if (tamanho_fator != 0 || tamanho != 0)
					throw tipo_incompativel(linha_erro);

				if (operador == TK_RES_AND)
				{
					if (tipo != TP_BOOL)
						throw tipo_incompativel(linha_erro);
				}
				else if (tipo != TP_INT)
					throw tipo_incompativel(linha_erro);

				tamanho = 0;

				break;

			default:
				return;
		}
	}
}

void parser::fator(tipo_dados_t &tipo, int &tamanho, std::string& destino)
{
	// not Fator | "(" Exp ")" | ID [ "[" Exp "]" ] | CONST

	tipo_dados_t tipo_exp;
	int tamanho_exp;

	tipo_dados_t tipo_fator;
	int tamanho_fator;

	int linha_erro;

	registro_tabela_simbolos *rt = token_lido->simbolo;
	std::string lex = token_lido->lex;

	switch (token_lido->tipo_token)
	{
		case TK_RES_NOT: // not Fator

			consome_token(TK_RES_NOT); // not

			// Ação 23
			tipo = TP_BOOL;
			tamanho = 0;

			linha_erro = num_linha;

			fator(tipo_fator, tamanho_fator, destino);

			// Ação 24
			if (tipo_fator != TP_BOOL || tamanho_fator > 0)
				throw tipo_incompativel(linha_erro);

			break;

		case TK_GRU_A_PAR: // "(" Exp ")"

			consome_token(TK_GRU_A_PAR); // (
			exp(tipo_exp, tamanho_exp, destino);

			// Ação 25
			tipo = tipo_exp;
			tamanho = tamanho_exp;

			consome_token(TK_GRU_F_PAR); // )
			break;

		case TK_ID: // ID [ "[" Exp "]" ]

			linha_erro = num_linha;

			consome_token(TK_ID); // ID

			// Ação 26
			if (rt->classe != CL_NULL)
			{
				tipo = rt->tipo;
				tamanho = rt->tam;
			}
			else throw id_nao_declarado(lex, linha_erro);

			// [ "[" Exp "]" ]
			if (token_lido->tipo_token == TK_GRU_A_COL)
			{
				linha_erro = num_linha;

				consome_token(TK_GRU_A_COL); // [
				exp(tipo_exp, tamanho_exp, destino);

				// Ação 27
				if (rt->tam == 0 || tipo_exp != TP_INT || tamanho_exp > 0)
					throw tipo_incompativel(linha_erro);

				tamanho = 0;

				consome_token(TK_GRU_F_COL); // ]
			}
			break;

		default: // CONST

			// Ação 28
			tipo = token_lido->tipo_constante;
			tamanho = token_lido->tam_constante;

			consome_token(TK_CONST);

			break;
	}
}
