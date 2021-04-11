#include <iostream>
#include "parser.h"

int
main(int argc, char* argv[])
{
    FILE *f;

    if (argc == 2)
        f = fopen(argv[1], "r");
    else
        f = stdin;

    if (!f)
        return 1;

    parser *p = nullptr;

    try
    {
        p = new parser(f);
        p->exec_parser();
    }
    catch (const std::exception& e)
    {
        std::cout << p->lxr->num_linha << std::endl;
        std::cout << e.what() << std::endl;

        fclose(f);
        delete p;
        
        return 1;
    }

    fclose(f);

	if (p)
	    std::cout << p->lxr->num_linha << " linhas compiladas." << std::endl;

    delete p;

    return 0;
}
