#pragma once

#include <string>

class erro_fonte : public std::exception
{
protected:
    std::string msg;

public:
    erro_fonte(std::string m) : msg(m) {}

    const char * what() const throw() { return msg.c_str(); }
};

class char_invalido : public erro_fonte
{
public:
    char_invalido() : erro_fonte("caractere invalido.") {}
};

class lex_nao_identificado : public erro_fonte
{
public:
    lex_nao_identificado(std::string l) : erro_fonte("lexema nao identificado [" + l + "].") {}
};

class token_invalido : public erro_fonte
{
public:
    token_invalido(std::string l) : erro_fonte("token nao esperado [" + l + "].") {}
};

class eof_inesperado : public erro_fonte
{
public:
    eof_inesperado() : erro_fonte("fim de arquivo nao esperado.") {}
};
