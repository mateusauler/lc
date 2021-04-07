#include <iostream>
#include "parser.h"

using namespace std;

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
        p->exec_parser();
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
