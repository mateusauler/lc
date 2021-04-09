#pragma once

#include "lexer.h"

class parser
{

public:

    lexer *lxr;

    parser(FILE *f);

    void exec_parser();

private:

    token_t* ultimo_token;

    void consome_token(token_type_t token_esperado);
    void prog();
    void dec_var();
    void dec_const();
    void var();
    void bloco_cmd();
    void cmd_s();
    void cmd_for();
    void cmd_if();
    void cmd_t();
    void exp();
    void soma();
    void termo();
    void fator();

};
