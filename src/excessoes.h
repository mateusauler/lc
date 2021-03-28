#ifndef EXCESSOES_H
#define EXCESSOES_H

#include <exception>
#include "main.h"

struct excProgramaFonte : public exception
{
    string lex;
    tipo_erro_t terro;

    excProgramaFonte(string _l, tipo_erro_t _terro)
    {
        lex = _l;
        terro = _terro;
    }

    excProgramaFonte(tipo_erro_t _terro)
    {
        lex = "";
        terro = _terro;
    }
};

#endif
