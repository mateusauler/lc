#pragma once

#include <string>

struct char_invalido : public std::exception
{
    const char * what() const throw();
};

struct lex_nao_identificado : public std::exception
{
    char *msg;
    lex_nao_identificado(std::string l);
    ~lex_nao_identificado();
    const char * what() const throw();
};

struct token_invalido : public std::exception
{
    char *msg;
    token_invalido(std::string l);
    ~token_invalido();
    const char * what() const throw();
};

struct eof_inesperado : public std::exception
{
    const char * what() const throw();
};
