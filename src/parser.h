#ifndef PARSER_H
#define PARSER_H

#include "main.h"
#include <exception>

class parser
{

public:

    void execParser();

private:

    token_t* ultimo_token;

    void consomeToken( token_type_t token );
    void prog();
    void decVar();
    void decConst();
    void blocoCmd();
    void cmdS();
    void cmdFor();
    void cmdIf();
    void cmdT();
    void exp();
    void soma();
    void termo();
    void fator();

};

#endif
