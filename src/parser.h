#pragma once

#include "lexer.h"

class parser : public lexer
{

public:

	parser(FILE *fonte, std::string saida) : lexer(fonte), arq_saida(saida) {}

	// Executa o parsing no arquivo passado para o construtor
	void exec_parser();

private:

	// Arquivo de saida
	std::string arq_saida;

	// Verifica se o token lido tem o tipo esperado e le o proximo
	void consome_token(tipo_token_t token_esperado);

	// Simbolo inicial da gramatica
	void prog(std::string& destino);

	// Declaracao de variaveis
	void dec_var(std::string& destino);

	// Declaracao de constantes
	void dec_const(std::string& destino);

	// Lista de variaveis declaradas
	void var(tipo_dados_t tipo, std::string& destino);

	// Bloco de comandos
	void bloco_cmd(std::string& destino);

	// Comando simples
	void cmd_s(std::string& destino);

	// Comando `for`
	void cmd_for(std::string& destino);

	// Comando `if`
	void cmd_if(std::string& destino);

	// Comando sem terminacao
	void cmd(std::string& destino);

	// Comando terminado
	void cmd_t(std::string& destino);

	// Expressao
	void exp(tipo_dados_t& tipo, int& tamanho, std::string& destino, int& endereco);

	// Lista de somas
	void soma(tipo_dados_t& tipo, int& tamanho, std::string& destino, int& endereco);

	// Termo de somas (lista de multiplicacoes)
	void termo(tipo_dados_t& tipo, int& tamanho, std::string& destino, int& endereco);

	// Fator de multiplicacoes
	void fator(tipo_dados_t& tipo, int& tamanho, std::string& destino, int& endereco);

};
