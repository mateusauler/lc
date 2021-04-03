#ifndef EXCESSOES_H
#define EXCESSOES_H

#include <exception>
#include <cstring>
#include "main.h"

struct char_invalido : public exception
{
    const char * what() const throw();
};

struct lex_nao_identificado : public exception
{
    char *msg;
    lex_nao_identificado(string l);
    const char * what() const throw();
};

struct token_invalido : public exception
{
    char *msg;
    token_invalido(string l);
    const char * what() const throw();
};

struct eof_inesperado : public exception
{
    const char * what() const throw();
};

#endif
