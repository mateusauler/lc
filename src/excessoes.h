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

class id_nao_declarado : public erro_fonte
{
public:
    id_nao_declarado(std::string l) : erro_fonte("identificador nao declarado [" + l + "].") {}
};

class id_ja_declarado : public erro_fonte
{
public:
    id_ja_declarado(std::string l) : erro_fonte("identificador ja declarado [" + l + "].") {}
};

class classe_id_incompativel : public erro_fonte
{
public:
    classe_id_incompativel(std::string l) : erro_fonte("classe de identificador incompativel [" + l + "].") {}
};

class tipo_incompativel : public erro_fonte
{
public:
    tipo_incompativel() : erro_fonte("tipos incompativeis.") {}
};

class tam_vet_excede_max : public erro_fonte
{
public:
    tam_vet_excede_max() : erro_fonte("tamanho do vetor excede o maximo permitido.") {}
};
