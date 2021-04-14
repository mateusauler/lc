#include <iostream>
#include "parser.h"

int
main(int argc, char* argv[])
{
    FILE *f;

    // Se um nome de arquivo nao for passado, ler do stdin
    if (argc == 2)
        f = fopen(argv[1], "r");
    else
        f = stdin;

    // Caso houver algum erro ao abrir o arquivo
    if (!f)
        return 1;

    parser p(f);
    bool erro = false; // Flag de erro

    try
    {
        // Executa o parser
        p.exec_parser();
    }
    catch (const std::exception& e)
    {
        // Imprime o numero da linha, seguido da mensagem de erro
        std::cout << p.num_linha << std::endl;
        std::cout << e.what() << std::endl;

        erro = true;
    }

    fclose(f);

	if (!erro)
	    std::cout << p.num_linha << " linhas compiladas." << std::endl;

    return 0;
}
