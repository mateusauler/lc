#include "tabela_simbolos.h"
#include "parser.h"
#include "excessoes.h"
#include "main.h"

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

    parser *p;

    try
    {
        p = new parser(f);
        p->execParser();
    }
    catch (const exception& e)
    {
        fclose(f);
        cerr << p->lxr->num_linha << endl;
        cerr << e.what() << endl;
        return 1;
    }

    fclose(f);

    cout << p->lxr->num_linha << " linhas compiladas." << endl;

    return 0;
}
