#include <cstring>
#include <sstream>
#include "excessoes.h"

const char * char_invalido::what() const throw()
{
    return "caractere invalido.";
}

const char * eof_inesperado::what() const throw()
{
    return "fim de arquivo nao esperado.";
}

lex_nao_identificado::lex_nao_identificado(std::string l)
{
    std::stringstream stream;
    stream << "lexema nao identificado [" << l << "].";

    std::string tmp = stream.str();
    msg = new char [tmp.length()];
    strcpy(msg, tmp.c_str());
}

const char * lex_nao_identificado::what() const throw()
{
    return msg;
}

token_invalido::token_invalido(std::string l)
{
    std::stringstream stream;
    stream << "token nao esperado [" << l << "].";

    std::string tmp = stream.str();
    msg = new char [tmp.length()];
    strcpy(msg, tmp.c_str());
}

const char * token_invalido::what() const throw()
{
    return msg;
}
