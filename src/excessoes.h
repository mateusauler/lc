#ifndef EXCESSOES_H
#define EXCESSOES_H

#include <exception>
#include "main.h"

struct excProgramaFonte : public exception
{
    string l;
    tipo_erro_t terro;

    excProgramaFonte(string _l, tipo_erro_t _terro)
    {
        l = _l;
        terro = _terro;
    }

    excProgramaFonte(tipo_erro_t _terro)
    {
        l = "";
        terro = _terro;
    }
};

#endif
