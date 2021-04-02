#include "tabela_simbolos.h"
#include "parser.h"
#include "excessoes.h"
#include "main.h"

void imprimir_erro(string lex, tipo_erro_t terro, int n_linha);

int
main(int argc, char* argv[])
{
    FILE *f;

    if (argc == 2)
        f = fopen(argv[1], "r");
    else
        f = stdin;

    if (!f)
        return -1;

    parser *p;

    try
    {
        p = new parser(f);
        p->execParser();
    }
    catch (const excProgramaFonte& e)
    {
        fclose(f);
        imprimir_erro(e.lex, e.terro, p->lxr->num_linha);
        return -1;
    }

    fclose(f);

    cout << p->lxr->num_linha << " linhas compiladas." << endl;

    return 0;
}

void
imprimir_erro(string lex, tipo_erro_t erro, int n_linha)
{
    stringstream mensagem;
    switch (erro)
    {
        case ERR_OK:
            break;

        case ERR_CHAR_INVALIDO:
            mensagem << "caractere invalido.";
            break;

        case ERR_LEX_NAO_IDENTIFICADO:
            mensagem << "lexema nao identificado [" << lex << "].";
            break;

        case ERR_TOKEN_NAO_ESPERADO:
            mensagem << "token nao esperado [" << lex << "].";
            break;

        case ERR_EOF_INESPERADO:
            mensagem << "fim de arquivo nao esperado.";
            break;
    }

    cerr << n_linha << endl;
    cerr << mensagem.str() << endl;
}