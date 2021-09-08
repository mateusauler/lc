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

#include "lexer.h"

class parser : public lexer
{

public:

	parser(FILE *fonte, std::string saida) : lexer(fonte), nome_arq_saida(saida) {}

	// Executa o parsing no arquivo passado para o construtor
	void exec_parser();

private:

	// Primeiro endereco de memoria disponivel
	unsigned long endereco = 0x4000;

	// Primeiro endereco de memoria para temporarios disponivel
	unsigned long end_tmp = 0;

	// Proximo sufixo de rotulo
	int rotulo = 0;

	// Nome do arquivo de saida
	const std::string nome_arq_saida;

	// Arquivo de saida
	FILE *arq_saida = nullptr;

	// Aloca um bloco de `bytes` bytes na memoria de variaveis e retorna o endereco do inicio deste bloco
	unsigned long aloca(unsigned long bytes);

	// Aloca um bloco de `bytes` bytes na memoria de temporarios e retorna o endereco do inicio deste bloco
	unsigned long novo_tmp(unsigned long bytes);

	// Gera um novo rotulo unico
	std::string novo_rotulo();

	// Concatena `texto` ao arquivo de saida
	void concatena_saida(std::string texto);

	// Verifica se o token lido tem o tipo esperado e le o proximo
	void consome_token(tipo_token_t token_esperado);

	// Simbolo inicial da gramatica
	void prog();

	// Declaracao de variaveis
	void dec_var();

	// Declaracao de constantes
	void dec_const();

	// Lista de variaveis declaradas
	void var(tipo_dados_t tipo);

	// Bloco de comandos
	void bloco_cmd();

	// Comando simples
	void cmd_s();

	// Comando `for`
	void cmd_for();

	// Comando `if`
	void cmd_if();

	// Comando sem terminacao
	void cmd();

	// Comando terminado
	void cmd_t();

	// Expressao
	void exp(tipo_dados_t& tipo, int& tamanho, unsigned long& endereco);

	// Lista de somas
	void soma(tipo_dados_t& tipo, int& tamanho, unsigned long& endereco);

	// Termo de somas (lista de multiplicacoes)
	void termo(tipo_dados_t& tipo, int& tamanho, unsigned long& endereco);

	// Fator de multiplicacoes
	void fator(tipo_dados_t& tipo, int& tamanho, unsigned long& endereco);

};
