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
	return "R" + std::to_string(rotulo++);
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

	codigo += dado + (nome.empty() ? "" : " ; " + nome) + "\n";

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
		"; Inicio do programa\n";

	consome_token(TK_RES_MAIN); // main
	bloco_cmd(destino);
	consome_token(TK_EOF); // EOF

	destino +=
		"\n"
		"; Fim do programa\n"
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

	// Gera a alocacao da constante e preenche o valor
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

			// Gera a alocacao da variavel e preenche o valor
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

			if (tipo_constante != TP_INT || valor_array == 0)
				throw tipo_incompativel(linha_erro);

			if (valor_array * byte_tipo(tipo) > 8192)
				throw tam_vet_excede_max(linha_erro);

			rt->tam = valor_array;

			consome_token(TK_GRU_F_COL); // ]

			// Aloca os bytes necessarios para o vetor
			rt->endereco = aloca(byte_tipo(rt->tipo) * rt->tam);
			destino += gera_alocacao(tipo, rt->tam, "", lex_id);

			break;

		default:
			// Aloca uma variavel vazia
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
	int tamanho_exp, tamanho, linha_erro, endereco = 0;

	if (token_lido->tipo_token == TK_ID) // ID [ "[" Exp "]" ] := Exp
	{
		rt = token_lido->simbolo;
		lex = token_lido->lex;
		linha_erro = num_linha;

		consome_token(TK_ID); // ID

		// Ação 10
		if (rt->classe == CL_NULL)  throw id_nao_declarado(lex, linha_erro);
		if (rt->classe == CL_CONST) throw classe_id_incompativel(lex, linha_erro);

		tamanho = rt->tam;

		destino += "\n; Inicio da atribuicao a [" + lex + "]\n";

		// [ "[" Exp "]" ]
		if (token_lido->tipo_token == TK_GRU_A_COL)
		{
			// Gera o calculo do endereco + desvio do vetor

			consome_token(TK_GRU_A_COL); // [

			linha_erro = num_linha;

			destino +=
				"\n"
				"; Calcula endereco + desvio do vetor [" + lex + "]\n"
				"\n"
				"; Inicio do calculo do desvio de [" + lex + "]\n";

			end_tmp = 0;
			exp(tipo_exp, tamanho_exp, destino, endereco);

			destino +=
				"\n"
				"; Fim do calculo do desvio de [" + lex + "]\n";

			// Ação 11
			if (tipo_exp != TP_INT || tamanho_exp > 0 || tamanho == 0)
				throw tipo_incompativel(linha_erro);

			tamanho = 0;

			destino +=
				"\n"
				"	mov BX, DS:[" + converte_hex(endereco) + "] ; Recupera desvio (calculado pela expressao)\n";

			if (rt->tipo == TP_INT || rt->tipo == TP_BOOL)
				destino += "	add BX, BX ; int e boolean ocupa 2 bytes\n";

			destino +=
				"	add BX, " + converte_hex(rt->endereco) + " ; Combina endereco base com desvio\n"
				"	push BX ; Armazena endereco na pilha\n"
				"\n"
				"; Fim do calculo de endereco do vetor [" + lex + "]\n";

			consome_token(TK_GRU_F_COL); // ]
		}
		else
		{
			// Recupera o endereco inicial da variavel e armazena ele na pilha
			destino +=
				"\n"
				"; Armazena endereco da variavel [" + lex + "] na pilha\n"
				"	mov BX, " + converte_hex(rt->endereco) + " ; Recupera endereco da variavel [" + lex + "]\n"
				"	push BX ; Armazena endereco na pilha\n";
		}

		consome_token(TK_OP_ATRIB); // :=

		linha_erro = num_linha;

		// Calcula o valor a ser atribuido
		destino +=
			"\n"
			"; Inicio do calculo do valor da atribuicao a [" + lex + "]\n";
		end_tmp = 0;
		exp(tipo_exp, tamanho_exp, destino, endereco);
		destino +=
			"\n"
			"; Fim do calculo do valor da atribuicao a [" + lex + "]\n";

		// Ação 12
		if (tipo_exp != rt->tipo)
		{
			if ((rt->tipo != TP_CHAR && tipo_exp != TP_STR))
				throw tipo_incompativel(linha_erro);

			if (tamanho < tamanho_exp)
				throw tam_vet_excede_max(linha_erro);

			// Atribuicao de constante string a vetor de caracteres
			// Copia os caracteres da constante para o vetor

			std::string rot_copia_str = novo_rotulo();

			destino +=
				"; Copia de constante string para [" + lex + "]\n"
				"\n"
				"	pop DI ; Endereco do vetor\n"
				"	mov SI, " + converte_hex(endereco) + " ; Endereco da origem\n";

			destino +=  "\n" + rot_copia_str + ":\n";

			destino +=
				"	mov AL, DS:[SI] ; Le o proximo caractere\n"
				"	mov DS:[DI], AL ; Armazena este caractere em [" + lex + "]\n"
				"	add DI, 1\n"
				"	add SI, 1\n"
				"	cmp AL, 024h ; Compara com $\n"
				"	jne " + rot_copia_str + " ; Continua copiando, se nao for o final da string\n"
				"\n"
				"; Fim do copia de constante string para [" + lex + "]\n";
		}
		else if (tamanho > 0)
		{
			if (rt->tipo != TP_CHAR || tamanho_exp == 0)
				throw tipo_incompativel(linha_erro);

			if (tamanho < tamanho_exp)
				throw tam_vet_excede_max(linha_erro);

			// Atribuicao de um vetor de caracteres a outro vetor de caracteres
			// Copia os caracteres de um vetor para outro

			std::string rot_copia_vet = novo_rotulo();

			destino +=
				"; Copia de vetor de char para [" + lex + "]\n"
				"\n"
				"	pop DI ; Endereco do vetor\n"
				"	mov SI, " + converte_hex(endereco) + " ; Endereco da origem\n";

			destino += "\n" + rot_copia_vet + ":\n";

			destino +=
				"	mov AL, DS:[SI] ; Le o proximo caractere\n"
				"	mov DS:[DI], AL ; Armazena este caractere em [" + lex + "]\n"
				"	add DI, 1\n"
				"	add SI, 1\n"
				"	cmp AL, 024h ; Compara com $\n"
				"	jne " + rot_copia_vet + " ; Continua copiando, se nao for o final da string\n"
				"\n"
				"; Fim do copia de vetor de char para [" + lex + "]\n";
		}
		else if (tamanho_exp > 0)
			throw tipo_incompativel(linha_erro);
		else
		{
			// Atribuicao escalar de uma variavel
			// Copia o conteudo calculado pela expressao e guarda na memoria

			if (rt->tipo == TP_CHAR)
			{
				// Caso for char, deve-se copiar somente 1 byte
				destino +=
					"\n"
					"	pop DI ; Recupera endereco\n"
					"	mov CL, DS:[" + converte_hex(endereco) + "] ; Recupera resultado da expressao\n"
					"	mov DS:[DI], CL ; Armazena resultado na memoria\n";
			}
			else
			{
				// Caso for int ou boolean, deve-se copiar 2 bytes
				destino +=
					"\n"
					"	pop DI ; Recupera endereco\n"
					"	mov CX, DS:[" + converte_hex(endereco) + "] ; Recupera resultado da expressao\n"
					"	mov DS:[DI], CX ; Armazena resultado na memoria\n";
			}
		}

		destino +=
			"\n"
			"; Fim da atribuicao a [" + lex + "]\n";
	}
	else if (token_lido->tipo_token == TK_RES_READLN) // readln "(" ID [ "[" Exp "]" ] ")"
	{
		consome_token(TK_RES_READLN); // readln
		consome_token(TK_GRU_A_PAR);  // (

		rt = token_lido->simbolo;
		linha_erro = num_linha;
		lex = token_lido->lex;

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

			end_tmp = 0;
			exp(tipo_exp, tamanho_exp, destino, endereco);

			// Ação 14
			if (tipo_exp != TP_INT || tamanho_exp > 0 || tamanho == 0)
				throw tipo_incompativel(linha_erro);

			tamanho = 0;

			consome_token(TK_GRU_F_COL); // ]

			destino +=
				"	mov DI, " + converte_hex(rt->endereco) + "\n"
				"	mov AX, DS:[" + converte_hex(endereco) + "]\n";

			if (rt->tipo != TP_CHAR)
				destino += "	add AX, AX\n";

			destino +=
				"	add DI, AX\n";
		}
		else
			destino += "	mov DI, " + converte_hex(rt->endereco) + "\n";

		// Ação 32
		if (tamanho > 0 && rt->tipo != TP_CHAR)
			throw tipo_incompativel(linha_erro);

		if (tamanho == 0)
		{
			if (rt->tipo == TP_CHAR)
			{
				int buffer_leitura = novo_tmp(4);
				destino +=
					"	mov DX, " + converte_hex(buffer_leitura) + "\n"
					"	mov AL, 04h\n"
					"	mov DS:[" + converte_hex(buffer_leitura) + "], AL\n"
					"	mov AH, 0Ah\n"
					"	int 21h\n"
					"	mov AH, 02h\n"
					"	mov DL, 0Dh\n"
					"	int 21h\n"
					"	mov DL, 0Ah\n"
					"	int 21h\n"
					"	mov AL, DS:[" + converte_hex(buffer_leitura) + "]\n"
					"	mov DS:[" + converte_hex(rt->endereco) + "], AL\n";
			}
			else
			{
				int buffer_leitura = novo_tmp(258);

				std::string rot_sinal = novo_rotulo(), rot_loop = novo_rotulo(), rot_fim = novo_rotulo();

				destino +=
					"	push DI\n"
					"	mov DX, " + converte_hex(buffer_leitura) + "\n"
					"	mov AL, 0FFh\n"
					"	mov DS:[" + converte_hex(buffer_leitura) + "], AL\n"
					"; Executa leitura\n"
					"	mov AH, 0Ah\n"
					"	int 21h\n"
					"; Imprime nova linha\n"
					"	mov AH, 02h\n"
					"	mov DL, 0Dh\n"
					"	int 21h\n"
					"	mov DL, 0Ah\n"
					"	int 21h\n"
					"\n"
					"	mov DI, " + converte_hex(buffer_leitura + 2) +" ;posição do string\n"
					"	mov AX, 0 ;acumulador\n"
					"	mov CX, 10 ;base decimal\n"
					"	mov DX, 1 ;valor sinal +\n"
					"	mov BH, 0\n"
					"	mov BL, DS:[DI] ;caractere\n"
					"	cmp BX, 02Dh ;verifica sinal\n"
					"	jne " + rot_sinal + " ;se não negativo\n"
					"	mov DX, -1 ;valor sinal -\n"
					"	add DI, 1 ;incrementa base\n"
					"	mov BL, DS:[DI] ;próximo caractere\n" +
					rot_sinal + ":\n"
					"	push DX ;empilha sinal\n"
					"	mov DX, 0 ;reg. multiplicação\n" +
					rot_loop + ":\n"
					"	cmp BX, 0Dh ;verifica fim string\n"
					"	je " + rot_fim + " ;salta se fim string\n"
					"	imul CX ;mult. 10\n"
					"	add BX, -48 ;converte caractere\n"
					"	add AX, BX ;soma valor caractere\n"
					"	add DI, 1 ;incrementa base\n"
					"	mov BH, 0\n"
					"	mov BL, DS:[DI] ;próximo caractere\n"
					"	jmp " + rot_loop + " ;loop\n" +
					rot_fim + ":\n"
					"	pop CX ;desempilha sinal\n"
					"	imul CX ;mult. sinal;\n"
					"	pop DI\n"
					"	mov DS:[DI], AX\n";
			}
		}
		else
		{
			int tamanho_buffer = 258;

			if (rt->tam < 255)
				tamanho_buffer = rt->tam + 3;

			int buffer_leitura = novo_tmp(tamanho_buffer);

			std::string rot_loop = novo_rotulo(), rot_fim = novo_rotulo();

			destino +=
				"	mov DX, " + converte_hex(buffer_leitura) + "\n"
				"	mov AL, " + converte_hex(tamanho_buffer - 3) + "\n"
				"	mov DS:[" + converte_hex(buffer_leitura) + "], AL\n"
				"	mov AH, 0Ah\n"
				"	int 21h\n"
				"	mov AH, 02h\n"
				"	mov DL, 0Dh\n"
				"	int 21h\n"
				"	mov DL, 0Ah\n"
				"	int 21h\n"
				"	mov AH, 0\n"
				"	mov DI, " + converte_hex(rt->endereco) + "\n"
				"	mov SI, " + converte_hex(buffer_leitura + 2) + "\n" +
				rot_loop + ":\n"
				"	mov AL, DS:[SI]\n"
				"	cmp AL, 0Dh\n"
				"	je " + rot_fim + "\n"
				"	mov DS:[DI], AL\n"
				"	add DI, 1\n"
				"	add SI, 1\n"
				"	jmp " + rot_loop + "\n" +
				rot_fim + ":\n"
				"	mov AL, 024h\n"
				"	mov DS:[DI], AL\n";
		}

		consome_token(TK_GRU_F_PAR); // )
	}
	else // (write | writeln) "(" Exp {, Exp} ")"
	{
		bool nova_linha = token_lido->tipo_token == TK_RES_WRITELN; // Armazena se deve imprimir nova linha

		if (token_lido->tipo_token == TK_RES_WRITE) consome_token(TK_RES_WRITE);   // write
		else                                        consome_token(TK_RES_WRITELN); // writeln

		consome_token(TK_GRU_A_PAR); // (

		linha_erro = num_linha;

		end_tmp = 0;
		exp(tipo_exp, tamanho_exp, destino, endereco);

		// Ação 33
		if (tipo_exp == TP_BOOL || (tamanho_exp > 0 && tipo_exp == TP_INT))
			throw tipo_incompativel(linha_erro);

		auto gera_impressao = [&]()
		{
			if (tipo_exp == TP_CHAR || tipo_exp == TP_STR)
			{
				if (tamanho_exp == 0)
				{
					// Imprimir um unico caractere

					destino +=
						"	mov DL, DS:[" + converte_hex(endereco) + "]\n"
						"	mov AH, 02h\n"
						"	int 21h\n";
				}
				else
				{
					// Imprimir uma string

					destino +=
						"	mov DX, " + converte_hex(endereco) + "\n"
						"	mov AH, 09h\n"
						"	int 21h\n";
				}
			}
			else
			{
				// Imprimir um inteiro

				int temp_impressao = novo_tmp(7);

				std::string rot_divisor  = novo_rotulo();
				std::string rot_divide   = novo_rotulo();
				std::string rot_converte = novo_rotulo();

				destino +=
					"\n"
					"	mov AX, DS:[" + converte_hex(endereco) + "] ; Carrega valor de do inteiro\n"
					"	mov DI, " + converte_hex(temp_impressao) + " ; Endereco da string para impressao\n"
					"	mov CX, 0 ; Contador\n"
					"	cmp AX, 0 ; Verifica sinal\n"
					"	jge " + rot_divisor + " ; Salta se numero positivo\n"
					"\n"
					"	mov BL, 2Dh ; Senao, escreve sinal –\n"
					"	mov DS:[DI], BL\n"
					"	add DI, 1 ; Incrementa indice\n"
					"	neg AX ; Toma modulo do numero\n"
					"\n" +
					rot_divisor + ":\n"
					"	mov BX, 10 ; Divisor\n"
					"\n" +
					rot_divide + ":\n"
					"	add CX, 1 ; Incrementa contador\n"
					"	mov DX, 0 ; Estende 32bits p/ div.\n"
					"	idiv BX ; Divide DXAX por BX\n"
					"	push DX ; Empilha valor do resto\n"
					"	cmp AX, 0 ; Verifica se quoc. e 0\n"
					"	jne " + rot_divide + " ; se nao e 0, continua\n"
					"\n" +
					rot_converte +":\n"
					"	pop DX ; desempilha valor\n"
					"	add DX, 30h ; transforma em caractere\n"
					"	mov DS:[DI], DL ; escreve caractere\n"
					"	add DI, 1 ; incrementa base\n"
					"	add CX, -1 ; decrementa contador\n"
					"	cmp CX, 0 ; verifica pilha vazia\n"
					"	jne " + rot_converte + " ; se não pilha vazia, loop\n";

				destino +=
					"\n"
					";grava fim de string\n"
					"\n"
					"	mov DL, 024h ; fim de string\n"
					"	mov DS:[DI], DL ; grava '$'\n"
					"\n"
					";exibe string\n"
					"\n"
					"	mov DX, " + converte_hex(temp_impressao) + "\n"
					"	mov AH, 09h\n"
					"	int 21h\n";
			}
		};

		gera_impressao();

		// {, Exp}
		while (token_lido->tipo_token == TK_OP_VIRGULA) // ,
		{
			consome_token(TK_OP_VIRGULA);

			linha_erro = num_linha;

			end_tmp = 0;
			exp(tipo_exp, tamanho_exp, destino, endereco);

			// Ação 34
			if (tipo_exp == TP_BOOL || (tamanho_exp > 0 && tipo_exp == TP_INT))
				throw tipo_incompativel(linha_erro);

			gera_impressao();
		}

		if (nova_linha)
		{
			destino +=
				"	mov AH, 02h\n"
				"	mov DL, 0Dh\n"
				"	int 21h\n"
				"	mov DL, 0Ah\n"
				"	int 21h\n";
		}

		consome_token(TK_GRU_F_PAR); // )
	}
}

void parser::cmd_for(std::string& destino)
{
	// for "(" [Cmd {, Cmd}] ; Exp ; [Cmd {, Cmd}] ")" (CmdT | BlocoCmd)

	tipo_dados_t tipo_exp;
	int tamanho_exp, endereco;
	std::string codigo_final = "", rot_loop = novo_rotulo(), rot_fim = novo_rotulo();

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

	destino += rot_loop + ":\n";

	end_tmp = 0;
	exp(tipo_exp, tamanho_exp, destino, endereco);

	// Ação 15
	if (tipo_exp != TP_BOOL || tamanho_exp > 0)
		throw tipo_incompativel(linha_erro);

	destino +=
		"	mov AX, DS:[" + converte_hex(endereco) + "]\n"
		"	cmp AX, 00h\n"
		"	je " + rot_fim + "\n";

	consome_token(TK_FIM_DECL); // ;

	// [Cmd {, Cmd}]
	if (token_lido->tipo_token != TK_GRU_F_PAR)
	{
		cmd(codigo_final);

		// {, Cmd}
		while (token_lido->tipo_token == TK_OP_VIRGULA)
		{
			consome_token(TK_OP_VIRGULA); // ,
			cmd(codigo_final);
		}
	}

	consome_token(TK_GRU_F_PAR); // )

	// (CmdT | BlocoCmd)
	if (token_lido->tipo_token == TK_GRU_A_CHA) bloco_cmd(destino);
	else                                        cmd_t(destino);

	destino += codigo_final;
	destino +=
		"	jmp " + rot_loop + "\n" +
		rot_fim + ":\n";
}

void parser::cmd_if(std::string& destino)
{
	// if "(" Exp ")" then (CmdT | BlocoCmd) [else (CmdT | BlocoCmd)]

	tipo_dados_t tipo_exp;
	int tamanho_exp, endereco;

	std::string rot_caso_falso = novo_rotulo();

	consome_token(TK_RES_IF);    // if
	consome_token(TK_GRU_A_PAR); // (

	int linha_erro = num_linha;

	end_tmp = 0;
	exp(tipo_exp, tamanho_exp, destino, endereco);

	// Ação 16
	if (tipo_exp != TP_BOOL || tamanho_exp > 0)
		throw tipo_incompativel(linha_erro);

	consome_token(TK_GRU_F_PAR); // )

	consome_token(TK_RES_THEN);  // then

	// Gera comparacao com jump para o caso_falso
	destino +=
		"	mov AX, DS:[" + converte_hex(endereco) + "]\n"
		"	cmp AX, 00h\n"
		"	je " + rot_caso_falso + "\n";

	// (CmdT | BlocoCmd)
	if (token_lido->tipo_token == TK_GRU_A_CHA) bloco_cmd(destino);
	else                                        cmd_t(destino);

	// [else (CmdT | BlocoCmd)]
	if (token_lido->tipo_token == TK_RES_ELSE)
	{
		std::string rot_fim = novo_rotulo();

		consome_token(TK_RES_ELSE); // else

		// Gera jmp para o fim
		// Gera rotulo do caso_falso
		destino	+=
			"	jmp " + rot_fim + "\n" +
			rot_caso_falso + ":\n";

		// (CmdT | BlocoCmd)
		if (token_lido->tipo_token == TK_GRU_A_CHA) bloco_cmd(destino);
		else                                        cmd_t(destino);

		// Gera rotulo do fim
		destino += rot_fim + ":\n";
	}
	else
	{
		// Gera o rotulo do caso_falso
		destino += rot_caso_falso + ":\n";
	}
}

void parser::cmd_t(std::string& destino)
{
	// [CmdS] ; | CmdFor | CmdIf

	destino += "\n; " + std::to_string(num_linha) + "\n";

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

void parser::exp(tipo_dados_t &tipo, int &tamanho, std::string& destino, int& endereco)
{
	// Soma [(=|<>|>|<|>=|<=) Soma]

	tipo_dados_t tipo_soma;
	int tamanho_soma;
	int linha_erro, endereco_soma;

	tipo_token_t operador;

	std::string reg_a = "AX", reg_b = "BX", rot_verdadeiro, rot_fim;

	auto compara_strings = [&]()
	{
		std::string rot_loop  = novo_rotulo();
		std::string rot_falso = novo_rotulo();
		rot_fim = novo_rotulo();

		destino +=
			"	mov DX, 01h\n"
			"	mov SI, " + converte_hex(endereco) + "\n"
			"	mov DI, " + converte_hex(endereco_soma) + "\n" +
			rot_loop + ":\n"
			"	mov AL, DS:[SI]\n"
			"	mov BL, DS:[DI]\n"
			"	cmp AL, BL\n"
			"	jne " + rot_falso + "\n"
			"	cmp AL, 024h\n"
			"	je " + rot_fim + "\n"
			"	add SI, 1\n"
			"	add DI, 1\n"
			"	jmp " + rot_loop + "\n" +
			rot_falso + ":\n"
			"	mov DX, 00h\n" +
			rot_fim + ":\n";

			endereco = novo_tmp(2);

			destino += "	mov DS:[" + converte_hex(endereco) + "], DX\n";
	};

	// Ação 17
	soma(tipo, tamanho, destino, endereco);

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

			soma(tipo_soma, tamanho_soma, destino, endereco_soma);

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

				compara_strings();
			}
			else if (tamanho > 0 || tamanho_soma > 0)
			{
				if (tipo == TP_CHAR || tipo == TP_STR)
				{
					if (tamanho == 0 || tamanho_soma == 0 || operador != TK_OP_EQ)
						throw tipo_incompativel(linha_erro);
				}
				else throw tipo_incompativel(linha_erro);

				compara_strings();
			}
			else
			{
				if (tipo == TP_CHAR)
				{
					reg_a = "AL";
					reg_b = "BL";

					destino +=
						"	mov AH, 0\n"
						"	mov BH, 0\n";
				}

				rot_verdadeiro = novo_rotulo();
				rot_fim = novo_rotulo();

				destino +=
					"	mov " + reg_a + ", DS:[" + converte_hex(endereco) + "]\n"
					"	mov " + reg_b + ", DS:[" + converte_hex(endereco_soma) + "]\n"
					"	cmp " + reg_a + ", " + reg_b + "\n";

				switch (operador)
				{
					case TK_OP_EQ: // =
						destino += "	je " + rot_verdadeiro + "\n";
						break;

					case TK_OP_NE: // <>
						destino += "	jne " + rot_verdadeiro + "\n";
						break;

					case TK_OP_GT: // >
						destino += "	jg " + rot_verdadeiro + "\n";
						break;

					case TK_OP_LT: // <
						destino += "	jl " + rot_verdadeiro + "\n";
						break;

					case TK_OP_GE: // >=
						destino += "	jge " + rot_verdadeiro + "\n";
						break;

					case TK_OP_LE: // <=
						destino += "	jle " + rot_verdadeiro + "\n";
						break;

					default:
						break;
				}

				endereco = novo_tmp(2);

				destino +=
					"	mov AX, 00h\n"
					"	jmp " + rot_fim + "\n" +
					rot_verdadeiro + ":\n"
					"	mov AX, 01h\n" +
					rot_fim + ":\n"
					"	mov DS:[" + converte_hex(endereco) + "], AX\n";
			}

			tipo = TP_BOOL;
			tamanho = 0;

			break;

		default:
			break;
	}
}

void parser::soma(tipo_dados_t &tipo, int &tamanho, std::string& destino, int& endereco)
{
	// [-] Termo {(+|-|or) Termo}

	tipo_dados_t tipo_termo;
	int tamanho_termo, endereco_termo;
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
	termo(tipo, tamanho, destino, endereco);
	if (nega && (tipo != TP_INT || tamanho > 0)) throw tipo_incompativel(linha_erro);

	if (nega)
	{
		destino +=
			"	mov BX, DS:[" + converte_hex(endereco) + "]\n"
			"	neg BX\n"
			"	mov DS:[" + converte_hex(endereco) + "], BX\n";
	}

	// {(+|-|or) Termo}
	while
	(
		token_lido->tipo_token == TK_OP_MAIS  || // +
		token_lido->tipo_token == TK_OP_MENOS || // -
		token_lido->tipo_token == TK_RES_OR      // or
	)
	{
		operador = token_lido->tipo_token;

		consome_token(token_lido->tipo_token); // (+|-|or)

		linha_erro = num_linha;

		termo(tipo_termo, tamanho_termo, destino, endereco_termo);

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

		destino +=
			"	mov AX, DS:[" + converte_hex(endereco) + "]\n"
			"	mov BX, DS:[" + converte_hex(endereco_termo) + "]\n";

		switch (operador)
		{
			case TK_OP_MAIS:  // +
				destino += "	add AX, BX\n";
				break;

			case TK_OP_MENOS: // -
				destino += "	sub AX, BX\n";
				break;

			case TK_RES_OR:   // or
				destino +=
					"	neg AX\n"
					"	add AX, 1\n"
					"	neg BX\n"
					"	add BX, 1\n"
					"	mov DX, 0\n"
					"	imul BX\n"
					"	neg AX\n"
					"	add AX, 1\n";
				break;

			default:
				break;
		}

		destino += " mov DS:[" + converte_hex(endereco) + "], AX\n";
	}
}

void parser::termo(tipo_dados_t &tipo, int &tamanho, std::string& destino, int& endereco)
{
	// Fator {(*|/|%|and) Fator}

	tipo_dados_t tipo_fator;
	int tamanho_fator, endereco_fator;

	tipo_token_t operador;

	int linha_erro;

	// Ação 21
	fator(tipo, tamanho, destino, endereco);

	// {(*|/|%|and) Fator}
	while
	(
		token_lido->tipo_token == TK_OP_MUL      || // *
		token_lido->tipo_token == TK_OP_BARRA    || // /
		token_lido->tipo_token == TK_OP_PORCENTO || // %
		token_lido->tipo_token == TK_RES_AND        // and
	)
	{
		operador = token_lido->tipo_token;

		consome_token(token_lido->tipo_token); // (*|/|%|and)

		linha_erro = num_linha;

		fator(tipo_fator, tamanho_fator, destino, endereco_fator);

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

		destino +=
			"	mov AX, DS:[" + converte_hex(endereco) + "]\n"
			"	mov BX, DS:[" + converte_hex(endereco_fator) + "]\n";

		switch (operador)
		{
			case TK_OP_MUL:      // *
			case TK_RES_AND:     // and
				destino += "	imul BX\n";
				break;

			case TK_OP_BARRA:    // /
				destino +=
					"	cwd\n"
					"	idiv BX\n";
				break;

			case TK_OP_PORCENTO: // %
				destino +=
					"	cwd\n"
					"	idiv BX\n"
					"	mov AX, DX\n";
				break;

			default:
				break;
		}

		destino += "	mov DS:[" + converte_hex(endereco) + "], AX\n";
	}
}

void parser::fator(tipo_dados_t &tipo, int &tamanho, std::string& destino, int& endereco)
{
	// not Fator | "(" Exp ")" | ID [ "[" Exp "]" ] | CONST

	tipo_dados_t tipo_exp;
	int tamanho_exp;

	tipo_dados_t tipo_fator;
	int tamanho_fator;

	int linha_erro;

	registro_tabela_simbolos *rt = token_lido->simbolo;
	std::string lex = token_lido->lex;
	std::string valor = lex;

	switch (token_lido->tipo_token)
	{
		case TK_RES_NOT: // not Fator

			consome_token(TK_RES_NOT); // not

			// Ação 23
			tipo = TP_BOOL;
			tamanho = 0;

			linha_erro = num_linha;

			fator(tipo_fator, tamanho_fator, destino, endereco);

			// Ação 24
			if (tipo_fator != TP_BOOL || tamanho_fator > 0)
				throw tipo_incompativel(linha_erro);

			destino +=
				"	mov BX, DS:[" + converte_hex(endereco) + "]\n"
				"	neg BX\n"
				"	add BX, 1\n"
				"	mov DS:[" + converte_hex(endereco) + "], BX\n";

			break;

		case TK_GRU_A_PAR: // "(" Exp ")"

			consome_token(TK_GRU_A_PAR); // (
			// Ação 25
			exp(tipo, tamanho, destino, endereco);
			consome_token(TK_GRU_F_PAR); // )
			break;

		case TK_ID: // ID [ "[" Exp "]" ]

			linha_erro = num_linha;
			lex = token_lido->lex;

			consome_token(TK_ID); // ID

			// Ação 26
			if (rt->classe != CL_NULL)
			{
				tipo = rt->tipo;
				tamanho = rt->tam;
			}
			else throw id_nao_declarado(lex, linha_erro);

			destino +=
				"\n"
				"; Leitura de [" + lex + "]\n";

			// [ "[" Exp "]" ]
			if (token_lido->tipo_token == TK_GRU_A_COL)
			{
				// Calcula o endereco + desvio do vetor e copia o valor da posicao para um temporario

				linha_erro = num_linha;

				consome_token(TK_GRU_A_COL); // [

				destino +=
					"; Calcula endereco + desvio do vetor [" + lex + "]\n"
					"; Inicio do calculo do desvio do vetor [" + lex + "]\n";

				exp(tipo_exp, tamanho_exp, destino, endereco);

				destino +=
					"\n"
					"; Fim do calculo do desvio do vetor [" + lex + "]\n";

				// Ação 27
				if (rt->tam == 0 || tipo_exp != TP_INT || tamanho_exp > 0)
					throw tipo_incompativel(linha_erro);

				tamanho = 0;

				destino +=
					"\n"
					"	mov BX, DS:[" + converte_hex(endereco) + "] ; Recupera desvio\n";

				if (tipo == TP_INT || tipo == TP_BOOL)
					destino += "	add BX, BX ; int e boolean ocupa 2 bytes\n";

				destino += "	add BX, " + converte_hex(rt->endereco) + " ; Combina endereco base com desvio\n";

				endereco = novo_tmp(1 + (tipo != TP_CHAR));

				destino +=
					"	mov CX, DS:[BX] ; Recupera valor na posicao calculada\n"
					"	mov DS:[" + converte_hex(endereco) + "], CX ; Armazena valor em um temporario\n"
					"\n"
					"; Fim do calculo do endereco + desvio do vetor [" + lex + "]\n"
					"\n";

				consome_token(TK_GRU_F_COL); // ]
			}
			else if (tamanho == 0)
			{
				// Caso seja uma variavel escalar ou constante, copia ele para um temporario
				if (tipo == TP_CHAR)
				{
					endereco = novo_tmp(1);
					destino +=
						"	mov BL, DS:[" + converte_hex(rt->endereco) + "] ; Recupera valor de [" + lex + "]\n"
						"	mov DS:[" + converte_hex(endereco) + "], BL ; Armazena valor em um temporario\n";
				}
				else
				{
					endereco = novo_tmp(2);
					destino +=
						"	mov BX, DS:[" + converte_hex(rt->endereco) + "] ; Recupera valor de [" + lex + "]\n"
						"	mov DS:[" + converte_hex(endereco) + "], BX ; Armazena valor em um temporario\n";
				}
			}
			else // Se for um vetor, armazena seu endereco
				endereco = rt->endereco;

			destino +=
				"; Fim da leitura de [" + lex + "]\n";

			break;

		default: // CONST

			// Ação 28
			tipo    = token_lido->tipo_constante;
			tamanho = token_lido->tam_constante;

			consome_token(TK_CONST);

			destino +=
				"\n"
				"; Constante [" + lex + "]\n";

			if (tipo == TP_STR)
			{
				endereco = aloca(tamanho);

				if (lex.length() > 2) // Se nao for string vazia
					destino += gera_alocacao(TP_CHAR, tamanho, lex, "", true);

				destino += gera_alocacao(TP_CHAR, tamanho, "\"$\"", "", true);
			}
			else
			{
				if (tipo == TP_BOOL)
					valor = converte_hex(lex == "TRUE");

				endereco = novo_tmp(byte_tipo(tipo));

				if (tipo == TP_CHAR)
				{
					// Caso for um caractere, deve-se movimentar 1 byte
					destino +=
						"	mov BL, " + valor + "\n"
						"	mov DS:[" + converte_hex(endereco) + "], BL\n";
				}
				else
				{
					// Caso for um int ou boolean, deve-se movimentar 2 bytes
					destino +=
						"	mov BX, " + valor + "\n"
						"	mov DS:[" + converte_hex(endereco) + "], BX\n";
				}
			}

			destino += "; Fim da constante [" + lex + "]\n";

			break;
	}
}
