#include "excessoes.h"

const char * char_invalido::what() const throw()
{
    return "caractere invalido.";
}

const char * eof_inesperado::what() const throw()
{
    return "fim de arquivo nao esperado.";
}

lex_nao_identificado::lex_nao_identificado(string l)
{
    stringstream stream;
    stream << "lexema nao identificado [" << l << "].";

    string tmp = stream.str();
    msg = (char*) malloc(tmp.length() * sizeof(char));
    strcpy(msg, tmp.c_str());
}

lex_nao_identificado::~lex_nao_identificado()
{
    free(msg);
}

const char * lex_nao_identificado::what() const throw()
{
    return msg;
}

token_invalido::token_invalido(string l)
{
    stringstream stream;
    stream << "token nao esperado [" << l << "].";

    string tmp = stream.str();
    msg = (char*) malloc(tmp.length() * sizeof(char));
    strcpy(msg, tmp.c_str());
}

token_invalido::~token_invalido()
{
    free(msg);
}

const char * token_invalido::what() const throw()
{
    return msg;
}
