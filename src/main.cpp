#include <iostream>

#include "parser.h"
#include "excessoes.h"

int main(int argc, char* argv[])
{
	FILE *arq_fonte, *arq_saida;

	// Se um nome de arquivo nao for passado, ler do stdin
	if (argc == 3)
	{
		arq_fonte = fopen(argv[1], "r");
		arq_saida = fopen(argv[2], "w");
	}
	else
	{
		arq_fonte = stdin;
		arq_saida = nullptr;
	}

	// Caso houver algum erro ao abrir o arquivo
	if (!arq_fonte)
		return 1;

	parser p(arq_fonte, arq_saida);
	bool erro = false; // Flag de erro

	try
	{
		// Executa o parser
		p.exec_parser();
	}
	catch (const erro_fonte& e)
	{
		// Imprime o numero da linha, seguido da mensagem de erro
		std::cout << e.linha_erro << std::endl;
		std::cout << e.what() << std::endl;

		remove(argv[2]);

		erro = true;
	}

	if (arq_saida)
		fclose(arq_saida);

	fclose(arq_fonte);

	if (!erro)
		std::cout << p.get_linha() << " linhas compiladas." << std::endl;

	return 0;
}
