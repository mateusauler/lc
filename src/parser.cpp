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

static std::string converte_hex(unsigned long valor)
{
	std::stringstream stream;
	stream << "0x" << std::uppercase << std::hex << valor;
	return stream.str();
}

unsigned long parser::aloca(unsigned long bytes)
{
	endereco += bytes;
	return endereco - bytes;
}

std::string parser::novo_rotulo()
{
	return "." + std::to_string(rotulo++);
}

unsigned long parser::novo_tmp(unsigned long bytes)
{
	end_tmp += bytes;
	return end_tmp - bytes;
}

void parser::concatena_saida(std::string texto)
{
	if (arq_saida)
		fprintf(arq_saida, "%s", texto.c_str());
}

void parser::exec_parser()
{
	if (!nome_arq_saida.empty())
		arq_saida = fopen(nome_arq_saida.c_str(), "w");

	try
	{
		proximo_token();
		prog();
	}
	catch (const erro_fonte& e)
	{
		if (arq_saida)
		{
			fclose(arq_saida);
			std::remove(nome_arq_saida.c_str());
		}

		throw e;
	}

	if (arq_saida)
		fclose(arq_saida);
}

void parser::consome_token(tipo_token_t token_esperado)
{
	if      (token_lido->tipo_token == token_esperado) proximo_token();
	else if (token_lido->tipo_token == TK_EOF) throw eof_inesperado(num_linha);
	else                                       throw token_invalido(token_lido->lex, num_linha);
}

void parser::prog()
{
	// {DecVar|DecConst} main BlocoCmd EOF

	concatena_saida
	(
		"section .data\n"
		"dsec:\n"
		"	resb 0x4000 ; Temporarios\n"
	);

	// {DecVar|DecConst}
	while (token_lido->tipo_token != TK_RES_MAIN)
	{
		if (token_lido->tipo_token == TK_RES_FINAL) dec_const(); // final
		else                                        dec_var();   // (int | char | boolean)
	}

	concatena_saida
	(
		"\n"
		"section .text\n"
		"	global _start\n"
		"\n"
		"_start:\n"
		"; Inicio do programa\n"
	);

	consome_token(TK_RES_MAIN); // main
	bloco_cmd();
	consome_token(TK_EOF); // EOF

	concatena_saida
	(
		"\n"
		"; Fim do programa\n"
		"	mov RAX, 60\n"
		"	mov RDX, 0\n"
		"	syscall"
	);
}

void parser::dec_var()
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
	var(tipo);

	// {, Var}
	while(token_lido->tipo_token == TK_OP_VIRGULA)
	{
		consome_token(TK_OP_VIRGULA); // ,
		// Ação 4
		var(tipo);
	}

	consome_token(TK_FIM_DECL); // ;
}

void parser::dec_const()
{
	// final ID = [-] CONST ;

	consome_token(TK_RES_FINAL); // final

	registro_tabela_simbolos *simbolo = token_lido->simbolo;
	std::string lex = token_lido->lex;
	int linha_erro = num_linha;
	bool nega = false;

	consome_token(TK_ID); // ID

	// Ação 5
	if (simbolo->classe != CL_NULL)
		throw id_ja_declarado(lex, linha_erro);

	simbolo->classe = CL_CONST;

	consome_token(TK_OP_EQ); // =

	// [-]
	if (token_lido->tipo_token == TK_OP_MENOS)
	{
		consome_token(TK_OP_MENOS);
		// Ação 29
		nega = true;
	}

	tipo_dados_t tipo_constante = token_lido->tipo_constante;
	linha_erro = num_linha;

	std::string lex_const = token_lido->lex;

	consome_token(TK_CONST); // CONST

	// Ação 6
	if (nega && tipo_constante != TP_INT)
		throw tipo_incompativel(linha_erro);

	if (tipo_constante == TP_STR || tipo_constante == TP_NULL)
		throw tipo_incompativel(linha_erro);

	// Gera a alocacao da constante e preenche o valor

	simbolo->tipo = tipo_constante;
	simbolo->endereco = aloca(byte_tipo(simbolo->tipo));

	std::string valor;

	if (simbolo->tipo == TP_BOOL)
		valor = std::to_string(lex_const == "TRUE");
	else
		valor = (nega ? "-" : "") + lex_const;

	// Aloca 1 ou 2 bytes (dependendo do tipo da constante) e preenche o valor
	concatena_saida((tipo_constante == TP_CHAR ? "	db " : "	dw ") + valor + " ; " + lex + "\n");

	consome_token(TK_FIM_DECL); // ;
}

void parser::var(tipo_dados_t tipo)
{
	// ID [:= [-] CONST | "[" CONST "]" ]

	registro_tabela_simbolos *simbolo = token_lido->simbolo;
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
	if (simbolo->classe != CL_NULL)
		throw id_ja_declarado(lex, linha_erro);

	simbolo->classe = CL_VAR;
	simbolo->tipo = tipo;

	switch (token_lido->tipo_token)
	{
		case TK_OP_ATRIB: // := [-] CONST

			consome_token(TK_OP_ATRIB); // :=

			// [-]
			if (token_lido->tipo_token == TK_OP_MENOS)
			{
				consome_token(TK_OP_MENOS);
				// Ação 30
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

			// Gera a alocacao da variavel e preenche o valor

			simbolo->endereco = aloca(byte_tipo(simbolo->tipo));
			if (simbolo->tipo == TP_BOOL)
				valor = std::to_string(lex_const == "TRUE");
			else
				valor = (nega ? "-" : "") + lex_const;

			// Aloca 1 ou 2 bytes (dependendo do tipo da variavel) e preenche o valor
			concatena_saida((tipo == TP_CHAR ? "	db " : "	dw ") + valor + " ; " + lex + "\n");

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

			simbolo->tam = valor_array;

			consome_token(TK_GRU_F_COL); // ]

			// Aloca os bytes necessarios para o vetor
			simbolo->endereco = aloca(byte_tipo(simbolo->tipo) * simbolo->tam);
			concatena_saida((tipo == TP_CHAR ? "	resb " : "	resw ") + std::to_string(simbolo->tam) + " ; " + lex_id + "\n");

			break;

		default:
			// Aloca uma variavel vazia
			simbolo->endereco = aloca(byte_tipo(simbolo->tipo));
			concatena_saida((tipo == TP_CHAR ? "	resb 1 ; " : "	resw 1 ; ") + lex + "\n");
			break;
	}
}

void parser::bloco_cmd()
{
	// "{" {CmdT} "}"

	consome_token(TK_GRU_A_CHA); // {

	// {CmdT}
	while (token_lido->tipo_token != TK_GRU_F_CHA)
		cmd_t();

	consome_token(TK_GRU_F_CHA); // }
}

void parser::cmd_s()
{
	// ID [ "[" Exp "]" ] := Exp
	// readln "(" ID [ "[" Exp "]" ] ")"
	// (write | writeln) "(" Exp {, Exp} ")"

	tipo_dados_t tipo_exp;
	int tamanho_exp, tamanho, linha_erro;
	unsigned long endereco = 0;

	if (token_lido->tipo_token == TK_ID) // ID [ "[" Exp "]" ] := Exp
	{
		registro_tabela_simbolos *simbolo = token_lido->simbolo;
		std::string lex = token_lido->lex;
		linha_erro = num_linha;

		consome_token(TK_ID); // ID

		// Ação 10
		if (simbolo->classe == CL_NULL)  throw id_nao_declarado(lex, linha_erro);
		if (simbolo->classe == CL_CONST) throw classe_id_incompativel(lex, linha_erro);

		tamanho = simbolo->tam;

		concatena_saida("\n; Inicio da atribuicao a [" + lex + "]\n");

		// [ "[" Exp "]" ]
		if (token_lido->tipo_token == TK_GRU_A_COL)
		{
			// Gera o calculo do endereco + desvio do vetor

			consome_token(TK_GRU_A_COL); // [

			linha_erro = num_linha;

			concatena_saida
			(
				"\n"
				"; Calcula endereco + desvio do vetor [" + lex + "]\n"
				"\n"
				"; Inicio do calculo do desvio de [" + lex + "]\n"
			);

			end_tmp = 0;
			exp(tipo_exp, tamanho_exp, endereco);

			// Ação 11
			if (tipo_exp != TP_INT || tamanho_exp > 0 || tamanho == 0)
				throw tipo_incompativel(linha_erro);

			tamanho = 0;

			concatena_saida
			(
				"\n"
				"; Fim do calculo do desvio de [" + lex + "]\n"
				"\n"
				"	mov RBX, 0\n"
				"	mov BX, [dsec+" + converte_hex(endereco) + "] ; Recupera desvio (calculado pela expressao)\n"
			);

			if (simbolo->tipo == TP_INT || simbolo->tipo == TP_BOOL)
				concatena_saida("	add BX, BX ; int e boolean ocupa 2 bytes\n");

			concatena_saida
			(
				"	add RBX, dsec+" + converte_hex(simbolo->endereco) + " ; Combina endereco base com desvio\n"
				"	push RBX ; Armazena endereco na pilha\n"
				"\n"
				"; Fim do calculo de endereco do vetor [" + lex + "]\n"
			);

			consome_token(TK_GRU_F_COL); // ]
		}
		else
		{
			// Recupera o endereco inicial da variavel e armazena ele na pilha
			concatena_saida
			(
				"\n"
				"; Armazena endereco da variavel [" + lex + "] na pilha\n"
				"	mov RBX, dsec+" + converte_hex(simbolo->endereco) + " ; Recupera endereco da variavel [" + lex + "]\n"
				"	push RBX ; Armazena endereco na pilha\n"
			);
		}

		consome_token(TK_OP_ATRIB); // :=

		linha_erro = num_linha;

		// Calcula o valor a ser atribuido
		concatena_saida
		(
			"\n"
			"; Inicio do calculo do valor da atribuicao a [" + lex + "]\n"
		);

		end_tmp = 0;
		exp(tipo_exp, tamanho_exp, endereco);

		concatena_saida
		(
			"\n"
			"; Fim do calculo do valor da atribuicao a [" + lex + "]\n"
		);

		// Ação 12
		if (tipo_exp != simbolo->tipo)
		{
			if ((simbolo->tipo != TP_CHAR && tipo_exp != TP_STR))
				throw tipo_incompativel(linha_erro);

			if (tamanho < tamanho_exp)
				throw tam_vet_excede_max(linha_erro);

			// Atribuicao de constante string a vetor de caracteres
			// Copia os caracteres da constante para o vetor

			std::string rot_copia_str = novo_rotulo();

			concatena_saida
			(
				"; Copia de constante string para [" + lex + "]\n"
				"\n"
				"	pop RDI ; Endereco do vetor\n"
				"	mov RSI, dsec+" + converte_hex(endereco) + " ; Endereco da origem\n"
				"\n" +
				rot_copia_str + ":\n"
				"	mov AL, [RSI] ; Le o proximo caractere\n"
				"	mov [RDI], AL ; Armazena este caractere em [" + lex + "]\n"
				"	inc RDI\n"
				"	inc RSI\n"
				"	cmp AL, 0 ; Compara com fim de string\n"
				"	jne " + rot_copia_str + " ; Continua copiando, se nao for o final da string\n"
				"\n"
				"; Fim do copia de constante string para [" + lex + "]\n"
			);
		}
		else if (tamanho > 0)
		{
			if (simbolo->tipo != TP_CHAR || tamanho_exp == 0)
				throw tipo_incompativel(linha_erro);

			if (tamanho < tamanho_exp)
				throw tam_vet_excede_max(linha_erro);

			// Atribuicao de um vetor de caracteres a outro vetor de caracteres
			// Copia os caracteres de um vetor para outro

			std::string rot_copia_vet = novo_rotulo();

			concatena_saida
			(
				"; Copia de vetor de char para [" + lex + "]\n"
				"\n"
				"	pop RDI ; Endereco do vetor\n"
				"	mov RSI, dsec+" + converte_hex(endereco) + " ; Endereco da origem\n"
				"\n" +
				rot_copia_vet + ":\n"
				"	mov AL, [RSI] ; Le o proximo caractere\n"
				"	mov [RDI], AL ; Armazena este caractere em [" + lex + "]\n"
				"	inc RDI\n"
				"	inc RSI\n"
				"	cmp AL, 0 ; Compara com o fim de string\n"
				"	jne " + rot_copia_vet + " ; Continua copiando, se nao for o final da string\n"
				"\n"
				"; Fim do copia de vetor de char para [" + lex + "]\n"
			);
		}
		else if (tamanho_exp > 0)
			throw tipo_incompativel(linha_erro);
		else
		{
			// Atribuicao escalar de uma variavel
			// Copia o conteudo calculado pela expressao e guarda na memoria

			if (simbolo->tipo == TP_CHAR)
			{
				// Caso for char, deve-se copiar somente 1 byte
				concatena_saida
				(
					"\n"
					"	pop RDI ; Recupera endereco de [" + lex + "]\n"
					"	mov CL, [dsec+" + converte_hex(endereco) + "] ; Recupera resultado da expressao\n"
					"	mov [RDI], CL ; Armazena resultado na memoria\n"
				);
			}
			else
			{
				// Caso for int ou boolean, deve-se copiar 2 bytes
				concatena_saida
				(
					"\n"
					"	pop RDI ; Recupera endereco de [" + lex + "]\n"
					"	mov CX, [dsec+" + converte_hex(endereco) + "] ; Recupera resultado da expressao\n"
					"	mov [RDI], CX ; Armazena resultado na memoria\n"
				);
			}
		}

		concatena_saida
		(
			"\n"
			"; Fim da atribuicao a [" + lex + "]\n"
		);
	}
	else if (token_lido->tipo_token == TK_RES_READLN) // readln "(" ID [ "[" Exp "]" ] ")"
	{
		consome_token(TK_RES_READLN); // readln
		consome_token(TK_GRU_A_PAR);  // (

		registro_tabela_simbolos *simbolo = token_lido->simbolo;
		std::string lex = token_lido->lex;
		linha_erro = num_linha;

		consome_token(TK_ID); // ID

		// Ação 13
		if (simbolo->classe == CL_NULL)  throw id_nao_declarado(lex, linha_erro);
		if (simbolo->classe == CL_CONST) throw classe_id_incompativel(lex, linha_erro);
		if (simbolo->tipo   == TP_BOOL)  throw tipo_incompativel(linha_erro);

		tamanho = simbolo->tam;

		// [ "[" Exp "]" ]
		if (token_lido->tipo_token == TK_GRU_A_COL)
		{
			consome_token(TK_GRU_A_COL); // [

			linha_erro = num_linha;

			concatena_saida
			(
				"\n"
				"; Calculo do desvio de [" + lex + "]\n"
			);

			end_tmp = 0;
			exp(tipo_exp, tamanho_exp, endereco);

			// Ação 14
			if (tipo_exp != TP_INT || tamanho_exp > 0 || tamanho == 0)
				throw tipo_incompativel(linha_erro);

			tamanho = 0;

			consome_token(TK_GRU_F_COL); // ]

			concatena_saida
			(
				"\n"
				"	mov RDI, dsec+" + converte_hex(simbolo->endereco) + "\n"
				"	mov RAX, 0\n"
				"	mov AX, [dsec+" + converte_hex(endereco) + "]\n"
			);

			if (simbolo->tipo != TP_CHAR)
				concatena_saida("	add AX, AX\n");

			concatena_saida
			(
				"	add RDI, RAX\n"
				"; Fim do calculo do desvio de [" + lex + "]\n"
				"\n"
			);
		}
		else
			concatena_saida("	mov RDI, dsec+" + converte_hex(simbolo->endereco) + "\n");

		// Ação 32
		if (tamanho > 0 && simbolo->tipo != TP_CHAR)
			throw tipo_incompativel(linha_erro);

		if (tamanho == 0)
		{
			if (simbolo->tipo == TP_CHAR)
			{
				unsigned long buffer_leitura = novo_tmp(1), buffer_limpeza = novo_tmp(1);
				std::string rot_fim_leitura = novo_rotulo(), rot_limpa_buffer = novo_rotulo();

				concatena_saida
				(
					"; Executa a leitura de N caracteres e armazena-os no endereço especificado\n"
					"	mov RAX, 0                                              ; Chamada de leitura\n"
					"	mov RSI, RDI                                            ; Endereço inicial da string\n"
					"	mov RDI, 0                                              ; Ler da entrada padrão\n"
					"	mov RDX, 1                                              ; Tamanho da string (incluindo o delimitador)\n"
					"	syscall                                                 ; Chama o kernel\n"
					"\n"
					"; Verifica se é necessário limpar o buffer do sistema\n"
					"	dec RSI\n"
					"	add RAX, RSI                                            ; RAX contém o número de caracteres lidos. Então, ao somar o endereço de memória - 1, obtém-se o endereço do último caractere (tipicamente a quebra de linha)\n"
					"	mov RBX, RAX                                            ; Armazena este endereço em RBX, pois RAX será reutilizado posteriormente\n"
					"	mov AL, [RBX]                                           ; Lê o caractere na última posição\n"
					"	cmp AL, 0xa                                             ; Verifica se ele é uma quebra de linha (\\n)\n"
					"	je " + rot_fim_leitura + "                                           ; Caso não for, o buffer do sistema ainda possui caracteres restantes\n"
					"\n"
					"; Realiza a limpeza do buffer de leitura do sistema operacional 1 byte por vez\n" +
					rot_limpa_buffer + ":\n"
					"	mov RAX, 0                                              ; Chamada de leitura\n"
					"	mov RDI, 0                                              ; Ler da entrada padrão\n"
					"	mov RSI, dsec+" + converte_hex(buffer_limpeza) + "           ; Primeira posição livre dos temporários (1 byte reservado)\n"
					"	mov RDX, 1                                              ; Ler 1 byte\n"
					"	syscall                                                 ; Chama o kernel\n"
					"\n"
					"	cmp RAX, 0\n"
					"	je " + rot_fim_leitura + "\n"
					"\n"
					"	mov AL, [dsec+" + converte_hex(buffer_leitura) + "]          ; Carrega o caractere lido\n"
					"	cmp AL, 0xa                                             ; Compara com a quebra de linha\n"
					"	jne " + rot_limpa_buffer + "                                         ; Caso seja diferente, ainda restam caracteres no buffer\n"
					"\n" +
					rot_fim_leitura + ":\n"
					"; Preenche o caractere final da string com o delimitador\n"
					"	mov AL, 0                                               ; Carrega o delimitador em al\n"
					"	mov [RBX], AL                                           ; Armazena o delimitador na memória\n"
				);
			}
			else
			{
				unsigned long buffer_leitura = novo_tmp(21);
				unsigned long buffer_limpeza = novo_tmp(1);

				std::string rot_leitura        = novo_rotulo();
				std::string rot_limpeza        = novo_rotulo();
				std::string rot_loop_conversao = novo_rotulo();
				std::string rot_fim_conversao  = novo_rotulo();

				concatena_saida
				(
					"	push RDI"
					"; Executa a leitura de N caracteres e armazena-os no endereço especificado\n"
					"	mov RAX, 0                                              ; Chamada de leitura\n"
					"	mov RDI, 0                                              ; Ler da entrada padrão\n"
					"	mov RSI, dsec+" + converte_hex(buffer_leitura) + "           ; Endereço inicial do buffer\n"
					"	mov RDX, 21                                             ; Reservados 21 caracteres para o maior número de 64 bits não assinalado ou assinalado + o sinal. Além do delimitador de string\n"
					"	syscall                                                 ; Chama o kernel\n"
					"\n"
					"; Verifica se é necessário limpar o buffer do sistema\n"
					"	add RAX, dsec+" + converte_hex(buffer_leitura - 1) + "       ; RAX contém o número de caracteres lidos. Então, ao somar o endereço do buffer - 1, obtém-se o endereço do último caractere (tipicamente a quebra de linha)\n"
					"	mov RBX, RAX                                            ; Armazena este endereço em RBX, pois RAX será reutilizado posteriormente\n"
					"	mov AL, [RBX]                                           ; Lê o caractere na última posição\n"
					"	cmp AL, 0xa                                             ; Verifica se ele é uma quebra de linha (\\n)\n"
					"	je " +  rot_leitura + "                                           ; Caso não for, o buffer do sistema ainda possui caracteres restantes\n"
					"\n"
					"; Realiza a limpeza do buffer de leitura do sistema operacional 1 byte por vez\n" +
					rot_limpeza + ":\n"
					"	mov RAX, 0                                              ; Chamada de leitura\n"
					"	mov RDI, 0                                              ; Ler da entrada padrão\n"
					"	mov RSI, dsec+" + converte_hex(buffer_limpeza) + "           ; Primeira posição livre dos temporários (1 byte reservado)\n"
					"	mov RDX, 1                                              ; Ler 1 byte\n"
					"	syscall                                                 ; Chama o kernel\n"
					"\n"
					"	cmp RAX, 0\n"
					"	je " + rot_leitura + "\n"
					"\n"
					"	mov AL, [dsec+" + converte_hex(buffer_limpeza) + "]          ; Carrega o caractere lido\n"
					"	cmp AL, 0xa                                             ; Compara com a quebra de linha\n"
					"	jne " + rot_limpeza + "                                         ; Caso seja diferente, ainda restam caracteres no buffer\n"
					"\n" +
					rot_leitura + ":\n"
					"; Preenche o caractere final da string com o delimitador\n"
					"	mov AL, 0                                               ; Carrega o delimitador em al\n"
					"	mov [RBX], AL                                           ; Armazena o delimitador na memória\n"
					"\n"
					"; Conversão da string lida para inteiro\n"
					"	mov AX, 0                                              ; RAX será utilizado como acumulador\n"
					"	mov BX, 0                                              ; RBX será utilizado para ler os caracteres\n"
					"	mov CX, 1                                              ; RCX será utilizado para armazenar o sinal\n"
					"	mov RSI, dsec+" + converte_hex(buffer_leitura) + "           ; Endereço da string (na região dos temporários)\n"
					"	mov BL, [RSI]                                           ; Carrega o primeiro caractere\n"
					"	cmp BL, '-'                                             ; Verifica se é um sinal -\n"
					"	jne " + rot_loop_conversao + "                                       ; Caso não seja, inicia a conversão\n"
					"\n"
					"	mov CX, -1                                             ; Armazena o sinal negativo em RCX\n"
					"	inc RSI                                                 ; Incrementa o ponteiro da string\n"
					"\n" +
					rot_loop_conversao + ":\n"
					"	mov BL, [RSI]                                           ; Lê o próximo caractere da string\n"
					"	cmp BL, 0                                               ; Verifica se ele é o fim de string\n"
					"	je " + rot_fim_conversao + "                                         ; Caso seja, acabou a conversão\n"
					"\n"
					"	mov DX, 10                                             ; Carrega a base\n"
					"	imul DX                                                ; Multiplica RDX:RAX por RDX (o valor em RDX será ignorado)\n"
					"	sub BL, '0'                                             ; Converte o dígito para seu valor numérico\n"
					"	add AX, BX                                            ; Adiciona o valor recuperado ao acumulador\n"
					"	inc RSI                                                 ; Incrementa o ponteiro da string\n"
					"\n"
					"	jmp " + rot_loop_conversao + "                          ; Continua a conversão\n"
					"\n" +
					rot_fim_conversao + ":\n"
					"	imul CX                                                ; Multiplica o número calculado pelo seu sinal\n"
					"	pop RDI\n"
					"	mov [RDI], AX                                     ; Armazena o número convertido\n"
				);
			}
		}
		else
		{
			int tamanho_buffer = 255;

			if (simbolo->tam < tamanho_buffer)
				tamanho_buffer = simbolo->tam;

			unsigned long buffer_limpeza = novo_tmp(1);

			std::string rot_fim_leitura  = novo_rotulo();
			std::string rot_limpa_buffer = novo_rotulo();

			concatena_saida
			(
				"; Executa a leitura de N caracteres e armazena-os no endereço especificado\n"
				"	mov RAX, 0                                              ; Chamada de leitura\n"
				"	mov RDI, 0                                              ; Ler da entrada padrão\n"
				"	mov RSI, dsec+" + converte_hex(simbolo->endereco) + "        ; Endereço inicial da string\n"
				"	mov RDX, " + std::to_string(tamanho_buffer) + "         ; Tamanho da string (incluindo o delimitador)\n"
				"	syscall                                                 ; Chama o kernel\n"
				"\n"
				"; Verifica se é necessário limpar o buffer do sistema\n"
				"	add RAX, dsec+" + converte_hex(simbolo->endereco - 1) + "    ; RAX contém o número de caracteres lidos. Então, ao somar o endereço de memória - 1, obtém-se o endereço do último caractere (tipicamente a quebra de linha)\n"
				"	mov RBX, RAX                                            ; Armazena este endereço em RBX, pois RAX será reutilizado posteriormente\n"
				"	mov AL, [RBX]                                           ; Lê o caractere na última posição\n"
				"	cmp AL, 0xa                                             ; Verifica se ele é uma quebra de linha (\\n)\n"
				"	je " + rot_fim_leitura + "                                           ; Caso não for, o buffer do sistema ainda possui caracteres restantes\n"
				"\n"
				"; Realiza a limpeza do buffer de leitura do sistema operacional 1 byte por vez\n" +
				rot_limpa_buffer + ":\n"
				"	mov RAX, 0                                              ; Chamada de leitura\n"
				"	mov RDI, 0                                              ; Ler da entrada padrão\n"
				"	mov RSI, dsec+" + converte_hex(buffer_limpeza) + "                                       ; Primeira posição livre dos temporários (1 byte reservado)\n"
				"	mov RDX, 1                                              ; Ler 1 byte\n"
				"	syscall                                                 ; Chama o kernel\n"
				"\n"
				"	cmp RAX, 0\n"
				"	je " + rot_fim_leitura + "\n"
				"\n"
				"	mov AL, [dsec+" + converte_hex(buffer_limpeza) + "]          ; Carrega o caractere lido\n"
				"	cmp AL, 0xa                                             ; Compara com a quebra de linha\n"
				"	jne " + rot_limpa_buffer + "                                         ; Caso seja diferente, ainda restam caracteres no buffer\n"
				"\n" +
				rot_fim_leitura + ":\n"
				"; Preenche o caractere final da string com o delimitador\n"
				"	mov AL, 0                                               ; Carrega o delimitador em al\n"
				"	mov [RBX], AL                                           ; Armazena o delimitador na memória\n"
			);
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
		exp(tipo_exp, tamanho_exp, endereco);

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

					concatena_saida
					(
						"; Escreve o char para a saída padrão\n"
						"	mov RDX, 1\n"
						"	mov RAX, 1                                              ; Chamada de escrita\n"
						"	mov RDI, 1                                              ; Escrever para a saída padrão\n"
						"	mov RSI, dsec+" + converte_hex(endereco) + "                 ; Move o endereço do char para o registrador fonte (source)\n"
						"	syscall                                                 ; Chama o kernel\n"
					);
				}
				else
				{
					// Imprimir uma string

					std::string rot_loop_str = novo_rotulo();

					concatena_saida
					(
						"; Calcula o tamanho da string\n"
						"	mov RSI, dsec+" + converte_hex(endereco) + "                 ; Move o endereço da string para o registrador fonte (source)\n"
						"	mov RDX, RSI                                            ; Recupera o endereço inicial da string\n" +
						rot_loop_str + ":                                           ; Loop para o cálculo do tamanho\n"
						"	mov AL, [RDX]                                           ; Lê o caractere atual\n"
						"	inc RDX                                                 ; Incrementa o ponteiro da string\n"
						"	cmp AL, 0                                               ; Verifica se o caractere lido é o byte 0 (fim de string)\n"
						"	jne " + rot_loop_str + "                                ; Caso não seja, continua o loop\n"
						"\n"
						"	sub RDX, RSI                                            ; Subtrai o endereço inicial da string pelo endereço final, o registrador RDX será utilizado pela chamada de sistema como o tamanho da string\n"
						"	dec RDX                                                 ; Desconsidera o byte nulo ao final da string\n"
						"\n"
						"; Escreve a string para a saída padrão\n"
						"	mov RAX, 1                                              ; Chamada de escrita\n"
						"	mov RDI, 1                                              ; Escrever para a saída padrão\n"
						"	syscall                                                 ; Chama o kernel\n"
					);
				}
			}
			else
			{
				// Imprimir um inteiro

				unsigned long temp_impressao = novo_tmp(21);

				std::string rot_loop_conversao = novo_rotulo();
				std::string rot_loop_monta_str = novo_rotulo();
				std::string rot_loop_str       = novo_rotulo();

				concatena_saida
				(
					"	mov RCX, 0                                              ; Inicializa contador para 0\n"
					"	mov BX, 10                                              ; Armazena o divisor\n"
					"	mov AX, [dsec+" + converte_hex(endereco) + "]                ; Carrega o número\n"
					"	mov RDI, dsec+" + converte_hex(temp_impressao) + "           ; Carrega o endereço do buffer (primeiro endereço livre no espaço de temporários)\n"
					"	cmp AX, -1                                              ; Verifica se o número é negativo\n"
					"	jg " + rot_loop_conversao + "                           ; Caso não for, começa a conversão\n"
					"	mov DL, '-'                                             ; Carrega o caractere - no byte baixo do registrador RDX\n"
					"	mov [RDI], DL                                           ; Armazena o sinal na primeira posição do buffer\n"
					"	inc RDI                                                 ; Incrementa o ponteiro para a próxima posição disponível\n"
					"	neg AX                                                  ; Inverte o número, para deixá-lo positivo\n"
					"\n" +
					rot_loop_conversao + ":\n"
					"	mov DX, 0                                               ; Limpa a parte alta do dividendo\n"
					"	idiv BX                                                 ; Divide RDX:RAX por RBX (contendo o valor 10) e armazena o resultado em RAX e o resto em RDX\n"
					"	push DX                                                 ; Empurra o resto na pilha (dígito menos significativo)\n"
					"	inc RCX                                                 ; Incrementa o contador\n"
					"	cmp AX, 0                                               ; Verifica se ainda resta valor para converter\n"
					"	jne " + rot_loop_conversao + "                          ; Continua a conversão caso necessário\n"
					"\n" +
					rot_loop_monta_str + ":\n"
					"	pop AX                                                  ; Recupera o próximo dígito\n"
					"	add AX, '0'                                             ; Adiciona o valor do caractere '0', para obter o valor do caractere do dígito\n"
					"	mov [RDI], AL                                           ; Escreve o caractere na próxima posição livre do buffer\n"
					"	inc RDI                                                 ; Incrementa o ponteiro do buffer\n"
					"	dec RCX                                                 ; Decrementa o contador\n"
					"	cmp RCX, 0                                              ; Verifica se ainda resta dígitos na pilha\n"
					"	jg " + rot_loop_monta_str + "                                      ; Continua a escrita, caso necessário\n"
					"\n"
					"	mov [RDI], byte 0                                       ; Escreve o marcador de fim de string no buffer\n"
					"\n"
					"; Calcula o tamanho da string\n"
					"	mov RSI, dsec+" + converte_hex(temp_impressao) + "           ; Move o endereço da string para o registrador fonte (source)\n"
					"	mov RDX, RSI                                            ; Recupera o endereço inicial do buffer + deslocamento\n" +
					rot_loop_str + ":                                                    ; Loop para o cálculo do tamanho\n"
					"	mov AL, [RDX]                                           ; Lê o caractere atual\n"
					"	inc RDX                                                 ; Incrementa o ponteiro da string\n"
					"	cmp AL, 0                                               ; Verifica se o caractere lido é o byte 0 (fim de string)\n"
					"	jne " + rot_loop_str + "                                             ; Caso não seja, continua o loop\n"
					"\n"
					"	sub RDX, RSI                                            ; Subtrai o endereço inicial da string pelo endereço final, o registrador RDX será utilizado pela chamada de sistema como o tamanho da string\n"
					"	dec RDX                                                 ; Desconsidera o byte nulo ao final da string\n"
					"\n"
					"; Escreve a string para a saída padrão\n"
					"	mov RAX, 1                                              ; Chamada de escrita\n"
					"	mov RDI, 1                                              ; Escrever para a saída padrão\n"
					"	syscall                                                 ; Chama o kernel\n"
				);
			}
		};

		gera_impressao();

		// {, Exp}
		while (token_lido->tipo_token == TK_OP_VIRGULA) // ,
		{
			consome_token(TK_OP_VIRGULA);

			linha_erro = num_linha;

			end_tmp = 0;
			exp(tipo_exp, tamanho_exp, endereco);

			// Ação 34
			if (tipo_exp == TP_BOOL || (tamanho_exp > 0 && tipo_exp == TP_INT))
				throw tipo_incompativel(linha_erro);

			gera_impressao();
		}

		if (nova_linha)
		{
			concatena_saida
			(
				"	push byte 0x0a\n"
				"	mov RDX, 1\n"
				"	mov RAX, 1                                              ; Chamada de escrita\n"
				"	mov RDI, 1                                              ; Escrever para a saída padrão\n"
				"	mov RSI, RSP                                            ; Move o endereço da string para o registrador fonte (source)\n"
				"	syscall                                                 ; Chama o kernel\n"
				"	add RSP, 1\n"
			);
		}

		consome_token(TK_GRU_F_PAR); // )
	}
}

void parser::cmd_for()
{
	// for "(" [Cmd {, Cmd}] ; Exp ; [Cmd {, Cmd}] ")" (CmdT | BlocoCmd)

	tipo_dados_t tipo_exp;
	int tamanho_exp;
	unsigned long endereco;
	bool epilogo = false;

	std::string rot_exp = novo_rotulo();
	std::string rot_fim = novo_rotulo();
	std::string rot_epilogo;

	consome_token(TK_RES_FOR);   // for
	consome_token(TK_GRU_A_PAR); // (

	concatena_saida
	(
		"\n"
		"; Inicio do loop\n"
	);

	// [Cmd {, Cmd}]
	if (token_lido->tipo_token != TK_FIM_DECL)
	{
		cmd();

		// {, Cmd}
		while (token_lido->tipo_token == TK_OP_VIRGULA)
		{
			consome_token(TK_OP_VIRGULA); // ,
			cmd();
		}
	}
	consome_token(TK_FIM_DECL); // ;

	int linha_erro = num_linha;

	concatena_saida
	(
		"\n" +
		rot_exp + ":\n"
		"\n"
		"; Expressao do loop\n"
	);

	end_tmp = 0;
	exp(tipo_exp, tamanho_exp, endereco);

	// Ação 15
	if (tipo_exp != TP_BOOL || tamanho_exp > 0)
		throw tipo_incompativel(linha_erro);

	concatena_saida
	(
		"\n"
		"; Verifica se a expressao foi verdadeira\n"
		"	mov AX, [dsec+" + converte_hex(endereco) + "]\n"
		"	cmp AX, 1\n"
		"	jne " + rot_fim + "\n"
	);

	consome_token(TK_FIM_DECL); // ;

	// [Cmd {, Cmd}]
	if (token_lido->tipo_token != TK_GRU_F_PAR)
	{
		epilogo = true;
		std::string rot_bloco = novo_rotulo();
		rot_epilogo = novo_rotulo();

		concatena_saida
		(
			"	jmp " + rot_bloco + "\n"
			"; Inicio do epilogo do loop\n" +
			rot_epilogo + ":\n"
		);

		cmd();

		// {, Cmd}
		while (token_lido->tipo_token == TK_OP_VIRGULA)
		{
			consome_token(TK_OP_VIRGULA); // ,
			cmd();
		}

		concatena_saida
		(
			"\n"
			"; Fim do epilogo do loop\n"
			"	jmp " + rot_exp + "\n" +
			rot_bloco + ":\n"
		);
	}

	consome_token(TK_GRU_F_PAR); // )

	// (CmdT | BlocoCmd)
	if (token_lido->tipo_token == TK_GRU_A_CHA) bloco_cmd();
	else                                        cmd_t();

	concatena_saida
	(
		"	jmp " + (epilogo ? rot_epilogo : rot_exp) + "\n"
		"; Final do loop\n" +
		rot_fim + ":\n"
	);
}

void parser::cmd_if()
{
	// if "(" Exp ")" then (CmdT | BlocoCmd) [else (CmdT | BlocoCmd)]

	tipo_dados_t tipo_exp;
	int tamanho_exp;
	unsigned long endereco;

	std::string rot_caso_falso = novo_rotulo();

	consome_token(TK_RES_IF);    // if
	consome_token(TK_GRU_A_PAR); // (

	int linha_erro = num_linha;

	concatena_saida
	(
		"\n"
		"; Expressao do if\n"
	);

	end_tmp = 0;
	exp(tipo_exp, tamanho_exp, endereco);

	// Ação 16
	if (tipo_exp != TP_BOOL || tamanho_exp > 0)
		throw tipo_incompativel(linha_erro);

	consome_token(TK_GRU_F_PAR); // )

	consome_token(TK_RES_THEN);  // then

	// Gera comparacao com jump para o caso_falso
	concatena_saida
	(
		"\n"
		"; Verifica resultado da expressao\n"
		"	mov AX, [dsec+" + converte_hex(endereco) + "]\n"
		"	cmp AX, 1\n"
		"	jne " + rot_caso_falso + "\n"
	);

	// (CmdT | BlocoCmd)
	if (token_lido->tipo_token == TK_GRU_A_CHA) bloco_cmd();
	else                                        cmd_t();

	// [else (CmdT | BlocoCmd)]
	if (token_lido->tipo_token == TK_RES_ELSE)
	{
		std::string rot_fim = novo_rotulo();

		consome_token(TK_RES_ELSE); // else

		// Gera jmp para o fim
		// Gera rotulo do caso_falso
		concatena_saida
		(
			"	jmp " + rot_fim + "\n"
			"; Else\n" +
			rot_caso_falso + ":\n"
		);

		// (CmdT | BlocoCmd)
		if (token_lido->tipo_token == TK_GRU_A_CHA) bloco_cmd();
		else                                        cmd_t();

		// Gera rotulo do fim
		concatena_saida
		(
			"; Fim else\n"+
			rot_fim + ":\n"
		);
	}
	else
	{
		// Gera o rotulo do caso_falso
		concatena_saida(rot_caso_falso + ":\n");
	}
}

void parser::cmd_t()
{
	// [CmdS] ; | CmdFor | CmdIf

	concatena_saida("\n; " + std::to_string(num_linha) + "\n");

	switch (token_lido->tipo_token)
	{
		case TK_RES_FOR: // for
			cmd_for();
			break;

		case TK_RES_IF: // if
			cmd_if();
			break;

		default:
			if (token_lido->tipo_token != TK_FIM_DECL) // [CmdS]
				cmd_s();

			consome_token(TK_FIM_DECL); // ;
			break;
	}
}

void parser::cmd()
{
	// CmdS | CmdFor | CmdIf

	switch (token_lido->tipo_token)
	{
		case TK_RES_FOR: // for
			cmd_for();
			break;

		case TK_RES_IF: // if
			cmd_if();
			break;

		default:
			cmd_s();
			break;
	}
}

void parser::exp(tipo_dados_t &tipo, int &tamanho, unsigned long& endereco)
{
	// Soma [(=|<>|>|<|>=|<=) Soma]

	tipo_dados_t tipo_soma;
	int tamanho_soma;
	unsigned long endereco_soma;

	int linha_erro;

	tipo_token_t operador;

	std::string reg_a = "AX", reg_b = "BX", rot_verdadeiro, rot_fim;

	auto compara_strings = [&]()
	{
		std::string rot_loop  = novo_rotulo();
		std::string rot_falso = novo_rotulo();
		rot_fim = novo_rotulo();

		concatena_saida
		(
			"\n"
			"; Comparacao de strings\n"
			"	mov DX, 1\n"
			"	mov RSI, dsec+" + converte_hex(endereco) + "\n"
			"	mov RDI, dsec+" + converte_hex(endereco_soma) + "\n" +
			rot_loop + ":\n"
			"	mov AL, [RSI]\n"
			"	mov BL, [RDI]\n"
			"	cmp AL, BL\n"
			"	jne " + rot_falso + "\n"
			"	cmp AL, 0\n"
			"	je " + rot_fim + "\n"
			"	add RSI, 1\n"
			"	add RDI, 1\n"
			"	jmp " + rot_loop + "\n" +
			rot_falso + ":\n"
			"	mov DX, 0\n" +
			rot_fim + ":\n"
		);

		endereco = novo_tmp(2);

		concatena_saida
		(
			"	mov [dsec+" + converte_hex(endereco) + "], DX\n"
			"; Fim da comparacao de strings\n"
		);
	};

	// Ação 17
	soma(tipo, tamanho, endereco);

	// [(=|<>|>|<|>=|<=) Soma]
	if
	(
		token_lido->tipo_token == TK_OP_EQ || // =
		token_lido->tipo_token == TK_OP_NE || // <>
		token_lido->tipo_token == TK_OP_GT || // >
		token_lido->tipo_token == TK_OP_LT || // <
		token_lido->tipo_token == TK_OP_GE || // >=
		token_lido->tipo_token == TK_OP_LE    // <=
	)
	{
		operador = token_lido->tipo_token;

		consome_token(token_lido->tipo_token); // (=|<>|>|<|>=|<=)

		linha_erro = num_linha;

		soma(tipo_soma, tamanho_soma, endereco_soma);

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

			// Comparacao de vetor de char com constante string
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

			// Comparacao de vetores de char ou constantes string
			compara_strings();
		}
		else
		{
			// Comparacao de escalares

			concatena_saida
			(
				"\n"
				"; Comparacao de escalares\n"
			);

			if (tipo == TP_CHAR)
			{
				reg_a = "AL";
				reg_b = "BL";

				concatena_saida
				(
					"	mov AH, 0\n"
					"	mov BH, 0\n"
				);
			}

			rot_verdadeiro = novo_rotulo();
			rot_fim        = novo_rotulo();

			concatena_saida
			(
				"	mov " + reg_a + ", [dsec+" + converte_hex(endereco) + "]\n"
				"	mov " + reg_b + ", [dsec+" + converte_hex(endereco_soma) + "]\n"
				"	cmp " + reg_a + ", " + reg_b + "\n"
			);

			switch (operador)
			{
				case TK_OP_EQ: // =
					concatena_saida("	je " + rot_verdadeiro + "\n");
					break;

				case TK_OP_NE: // <>
					concatena_saida("	jne " + rot_verdadeiro + "\n");
					break;

				case TK_OP_GT: // >
					concatena_saida("	jg " + rot_verdadeiro + "\n");
					break;

				case TK_OP_LT: // <
					concatena_saida("	jl " + rot_verdadeiro + "\n");
					break;

				case TK_OP_GE: // >=
					concatena_saida("	jge " + rot_verdadeiro + "\n");
					break;

				case TK_OP_LE: // <=
					concatena_saida("	jle " + rot_verdadeiro + "\n");
					break;

				default:
					break;
			}

			endereco = novo_tmp(2);

			concatena_saida
			(
				"	mov AX, 0\n"
				"	jmp " + rot_fim + "\n" +
				rot_verdadeiro + ":\n"
				"	mov AX, 1\n" +
				rot_fim + ":\n"
				"	mov [dsec+" + converte_hex(endereco) + "], AX\n"
				"; Fim da comparacao de escalares\n"
			);
		}

		tipo = TP_BOOL;
		tamanho = 0;
	}
}

void parser::soma(tipo_dados_t &tipo, int &tamanho, unsigned long& endereco)
{
	// [-] Termo {(+|-|or) Termo}

	tipo_dados_t tipo_termo;
	int tamanho_termo;
	unsigned long endereco_termo;
	bool nega = false;

	tipo_token_t operador;

	// [-]
	if (token_lido->tipo_token == TK_OP_MENOS)
	{
		consome_token(TK_OP_MENOS);
		// Ação 31
		nega = true;
	}

	int linha_erro;

	// Ação 19
	linha_erro = num_linha;
	termo(tipo, tamanho, endereco);

	if (nega && (tipo != TP_INT || tamanho > 0)) throw tipo_incompativel(linha_erro);

	if (nega)
	{
		concatena_saida
		(
			"\n"
			"; Nega o primeiro termo\n"
			"	mov BX, [dsec+" + converte_hex(endereco) + "]\n"
			"	neg BX\n"
			"	mov [dsec+" + converte_hex(endereco) + "], BX\n"
		);
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

		termo(tipo_termo, tamanho_termo, endereco_termo);

		// Ação 20
		if (tipo != tipo_termo)
			throw tipo_incompativel(linha_erro);

		if (tamanho_termo != 0 || tamanho != 0)
			throw tipo_incompativel(linha_erro);

		switch (tipo)
		{
			case TP_INT:
				if (operador == TK_RES_OR)
					throw tipo_incompativel(linha_erro);
				break;

			case TP_BOOL:
				if (operador != TK_RES_OR)
					throw tipo_incompativel(linha_erro);
				break;

			default:
				throw tipo_incompativel(linha_erro);
		}

		tamanho = 0;

		concatena_saida
		(
			"	mov AX, [dsec+" + converte_hex(endereco) + "]\n"
			"	mov BX, [dsec+" + converte_hex(endereco_termo) + "]\n"
		);

		switch (operador)
		{
			case TK_OP_MAIS:  // +
				concatena_saida("	add AX, BX\n");
				break;

			case TK_OP_MENOS: // -
				concatena_saida("	sub AX, BX\n");
				break;

			case TK_RES_OR:   // or
				concatena_saida
				(
					"	neg AX\n"
					"	add AX, 1\n"
					"	neg BX\n"
					"	add BX, 1\n"
					"	mov DX, 0\n"
					"	imul BX\n"
					"	neg AX\n"
					"	add AX, 1\n"
				);
				break;

			default:
				break;
		}

		concatena_saida("	mov [dsec+" + converte_hex(endereco) + "], AX\n");
	}
}

void parser::termo(tipo_dados_t &tipo, int &tamanho, unsigned long& endereco)
{
	// Fator {(*|/|%|and) Fator}

	tipo_dados_t tipo_fator;
	int tamanho_fator;
	unsigned long endereco_fator;

	tipo_token_t operador;

	int linha_erro;

	// Ação 21
	fator(tipo, tamanho, endereco);

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

		fator(tipo_fator, tamanho_fator, endereco_fator);

		// Ação 22
		if (tipo != tipo_fator)
			throw tipo_incompativel(linha_erro);

		if (tamanho_fator != 0 || tamanho != 0)
			throw tipo_incompativel(linha_erro);

		switch (tipo)
		{
			case TP_INT:
				if (operador == TK_RES_AND)
					throw tipo_incompativel(linha_erro);
				break;

			case TP_BOOL:
				if (operador != TK_RES_AND)
					throw tipo_incompativel(linha_erro);
				break;

			default:
				throw tipo_incompativel(linha_erro);
		}

		tamanho = 0;

		concatena_saida
		(
			"	mov AX, [dsec+" + converte_hex(endereco) + "]\n"
			"	mov BX, [dsec+" + converte_hex(endereco_fator) + "]\n"
		);

		switch (operador)
		{
			case TK_OP_MUL:  // *
			case TK_RES_AND: // and
				concatena_saida("	imul BX\n");
				break;

			case TK_OP_BARRA: // /
				concatena_saida
				(
					"	cwd\n"
					"	idiv BX\n"
				);
				break;

			case TK_OP_PORCENTO: // %
				concatena_saida
				(
					"	cwd\n"
					"	idiv BX\n"
					"	mov AX, DX\n"
				);
				break;

			default:
				break;
		}

		concatena_saida("	mov [dsec+" + converte_hex(endereco) + "], AX\n");
	}
}

void parser::fator(tipo_dados_t &tipo, int &tamanho, unsigned long& endereco)
{
	// not Fator | "(" Exp ")" | ID [ "[" Exp "]" ] | CONST

	tipo_dados_t tipo_exp;
	int tamanho_exp;

	tipo_dados_t tipo_fator;
	int tamanho_fator;

	int linha_erro;

	registro_tabela_simbolos *simbolo = token_lido->simbolo;
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

			fator(tipo_fator, tamanho_fator, endereco);

			// Ação 24
			if (tipo_fator != TP_BOOL || tamanho_fator > 0)
				throw tipo_incompativel(linha_erro);

			concatena_saida
			(
				"; Nega um fator\n"
				"	mov BX, [dsec+" + converte_hex(endereco) + "]\n"
				"	neg BX\n"
				"	add BX, 1\n"
				"	mov [dsec+" + converte_hex(endereco) + "], BX\n"
			);

			break;

		case TK_GRU_A_PAR: // "(" Exp ")"

			consome_token(TK_GRU_A_PAR); // (
			// Ação 25
			exp(tipo, tamanho, endereco);
			consome_token(TK_GRU_F_PAR); // )
			break;

		case TK_ID: // ID [ "[" Exp "]" ]

			linha_erro = num_linha;
			lex = token_lido->lex;

			consome_token(TK_ID); // ID

			// Ação 26
			if (simbolo->classe == CL_NULL)
				throw id_nao_declarado(lex, linha_erro);

			tipo    = simbolo->tipo;
			tamanho = simbolo->tam;

			concatena_saida
			(
				"\n"
				"; Carregamento de [" + lex + "]\n"
			);

			// [ "[" Exp "]" ]
			if (token_lido->tipo_token == TK_GRU_A_COL)
			{
				// Calcula o endereco + desvio do vetor e copia o valor da posicao para um temporario

				linha_erro = num_linha;

				consome_token(TK_GRU_A_COL); // [

				concatena_saida
				(
					"; Calcula endereco + desvio do vetor [" + lex + "]\n"
					"; Inicio do calculo do desvio do vetor [" + lex + "]\n"
				);

				exp(tipo_exp, tamanho_exp, endereco);

				concatena_saida
				(
					"\n"
					"; Fim do calculo do desvio do vetor [" + lex + "]\n"
				);

				// Ação 27
				if (simbolo->tam == 0 || tipo_exp != TP_INT || tamanho_exp > 0)
					throw tipo_incompativel(linha_erro);

				tamanho = 0;

				concatena_saida
				(
					"\n"
					"	mov RSI, 0\n"
					"	mov SI, [dsec+" + converte_hex(endereco) + "] ; Recupera desvio\n"
				);

				if (tipo == TP_INT || tipo == TP_BOOL)
					concatena_saida("	add RSI, RSI ; int e boolean ocupa 2 bytes\n");

				concatena_saida("	add RSI, dsec+" + converte_hex(simbolo->endereco) + " ; Combina endereco base com desvio\n");

				endereco = novo_tmp(1 + (tipo != TP_CHAR));

				concatena_saida
				(
					"	mov CX, [RSI] ; Recupera valor na posicao calculada\n"
					"	mov [dsec+" + converte_hex(endereco) + "], CX ; Armazena valor em um temporario\n"
					"\n"
					"; Fim do calculo do endereco + desvio do vetor [" + lex + "]\n"
					"\n"
				);

				consome_token(TK_GRU_F_COL); // ]
			}
			else if (tamanho == 0)
			{
				// Caso seja uma variavel escalar ou constante, copia ele para um temporario
				if (tipo == TP_CHAR)
				{
					endereco = novo_tmp(1);
					concatena_saida
					(
						"	mov BL, [dsec+" + converte_hex(simbolo->endereco) + "] ; Recupera valor de [" + lex + "]\n"
						"	mov [dsec+" + converte_hex(endereco) + "], BL ; Armazena valor em um temporario\n"
					);
				}
				else
				{
					endereco = novo_tmp(2);
					concatena_saida
					(
						"	mov BX, [dsec+" + converte_hex(simbolo->endereco) + "] ; Recupera valor de [" + lex + "]\n"
						"	mov [dsec+" + converte_hex(endereco) + "], BX ; Armazena valor em um temporario\n"
					);
				}
			}
			else // Se for um vetor, armazena seu endereco
				endereco = simbolo->endereco;

			concatena_saida("; Fim do carregamento de [" + lex + "]\n");

			break;

		default: // CONST

			// Ação 28
			tipo    = token_lido->tipo_constante;
			tamanho = token_lido->tam_constante;

			consome_token(TK_CONST);

			concatena_saida
			(
				"\n"
				"; Constante [" + lex + "]\n"
			);

			if (tipo == TP_STR)
			{
				endereco = aloca(tamanho);

				concatena_saida("section .data\n");

				if (lex.length() > 2) // Se nao for string vazia
					concatena_saida("	db " + lex + "\n");

				concatena_saida
				(
					"	db 0\n"
					"section .text\n"
				);
			}
			else
			{
				if (tipo == TP_BOOL)
					valor = std::to_string(lex == "TRUE");

				endereco = novo_tmp(byte_tipo(tipo));

				if (tipo == TP_CHAR)
				{
					// Caso for um caractere, deve-se movimentar 1 byte
					concatena_saida
					(
						"	mov BL, " + valor + "\n"
						"	mov [dsec+" + converte_hex(endereco) + "], BL\n"
					);
				}
				else
				{
					// Caso for um int ou boolean, deve-se movimentar 2 bytes
					concatena_saida
					(
						"	mov BX, " + valor + "\n"
						"	mov [dsec+" + converte_hex(endereco) + "], BX\n"
					);
				}
			}

			concatena_saida("; Fim da constante [" + lex + "]\n");

			break;
	}
}
