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

#include <iostream>

#include "parser.h"
#include "excessoes.h"

int main(int argc, char* argv[])
{
	FILE *arq_fonte = stdin;
	std::string arq_saida = "";

	// Determina de onde sera lido o programa fonte e para onde ira o codigo gerado
	switch (argc)
	{
		case 3:
			arq_saida = argv[2];

		case 2:
			arq_fonte = fopen(argv[1], "r");
			break;

		default:
			break;
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

		erro = true;
	}

	fclose(arq_fonte);

	if (!erro)
		std::cout << p.get_linha() << " linhas compiladas." << std::endl;

	return 0;
}
