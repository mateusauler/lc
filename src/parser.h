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
	int endereco = 0x4000;

	// Primeiro endereco de memoria para temporarios disponivel
	int end_tmp = 0;

	// Proximo sufixo de rotulo
	int rotulo = 0;

	// Nome do arquivo de saida
	std::string nome_arq_saida;

	// Arquivo de saida
	FILE * arq_saida = nullptr;

	// Aloca um bloco de `bytes` bytes na memoria de variaveis e retorna o endereco do inicio deste bloco
	int aloca(int bytes);

	// Aloca um bloco de `bytes` bytes na memoria de temporarios e retorna o endereco do inicio deste bloco
	int novo_tmp(int bytes);

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
	void exp(tipo_dados_t& tipo, int& tamanho, int& endereco);

	// Lista de somas
	void soma(tipo_dados_t& tipo, int& tamanho, int& endereco);

	// Termo de somas (lista de multiplicacoes)
	void termo(tipo_dados_t& tipo, int& tamanho, int& endereco);

	// Fator de multiplicacoes
	void fator(tipo_dados_t& tipo, int& tamanho, int& endereco);

};
