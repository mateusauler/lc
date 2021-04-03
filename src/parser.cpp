#include "parser.h"

parser::parser(FILE *f)
{
    lxr = new lexer(f);
}

void parser::execParser()
{
    ultimo_token = new token_t(lxr->proximo_token());
    prog();
}

void parser::consomeToken(token_type_t token)
{
    if (ultimo_token->tipo == token)
    {
        if (ultimo_token->tipo != TK_EOF)
            *ultimo_token = lxr->proximo_token();
    }
    else if (ultimo_token->tipo == TK_EOF) throw eof_inesperado();
    else                                   throw token_invalido(ultimo_token->lex);
}

void parser::prog()
{
    while (ultimo_token->tipo != TK_RES_MAIN)
    {
        if (TK_RES_FINAL == ultimo_token->tipo) decConst();
        else                                    decVar();
    }

    consomeToken(TK_RES_MAIN);
    blocoCmd();
    consomeToken(TK_EOF);
}

void parser::decVar()
{
    if      (ultimo_token->tipo == TK_RES_INT)  consomeToken(TK_RES_INT);
    else if (ultimo_token->tipo == TK_RES_CHAR) consomeToken(TK_RES_CHAR);
    else                                        consomeToken(TK_RES_BOOLEAN);

    var();

    while(ultimo_token->tipo == TK_OP_COMMA)
    {
        consomeToken(TK_OP_COMMA);
        var();
    }

    consomeToken(TK_END_STATEMENT);
}

void parser::decConst()
{
    consomeToken(TK_RES_FINAL);
    consomeToken(TK_ID);
    consomeToken(TK_OP_EQ);

    if      (ultimo_token->tipo == TK_OP_PLUS)  consomeToken(TK_OP_PLUS);
    else if (ultimo_token->tipo == TK_OP_MINUS) consomeToken(TK_OP_MINUS);

    consomeToken(TK_CONST);
    consomeToken(TK_END_STATEMENT);
}

void parser::var()
{
    consomeToken(TK_ID);

    if (ultimo_token->tipo == TK_OP_ATTRIB)
    {
        consomeToken(TK_OP_ATTRIB);

        if      (ultimo_token->tipo == TK_OP_PLUS)  consomeToken(TK_OP_PLUS);
        else if (ultimo_token->tipo == TK_OP_MINUS) consomeToken(TK_OP_MINUS);

        consomeToken(TK_CONST);
    }
    else if (ultimo_token->tipo == TK_BRA_O_SQR)
    {
        consomeToken(TK_BRA_O_SQR);
        consomeToken(TK_CONST);
        consomeToken(TK_BRA_C_SQR);
    }
}

void parser::blocoCmd()
{
    consomeToken(TK_BRA_O_CUR);

    while (ultimo_token->tipo != TK_BRA_C_CUR) cmdT();

    consomeToken(TK_BRA_C_CUR);
}

void parser::cmdS()
{
    if (ultimo_token->tipo == TK_ID)
    {
        consomeToken(TK_ID);

        if (ultimo_token->tipo == TK_BRA_O_SQR)
        {
            consomeToken(TK_BRA_O_SQR);
            exp();
            consomeToken(TK_BRA_C_SQR);
        }

        consomeToken(TK_OP_ATTRIB);
        exp();
    }
    else if (ultimo_token->tipo == TK_RES_READLN)
    {
        consomeToken(TK_RES_READLN);
        consomeToken(TK_BRA_O_PAR);
        consomeToken(TK_ID);

        if (ultimo_token->tipo == TK_BRA_O_SQR)
        {
            consomeToken(TK_BRA_O_SQR);
            exp();
            consomeToken(TK_BRA_C_SQR);
        }

        consomeToken(TK_BRA_C_PAR);

    }
    else
    {
        if (ultimo_token->tipo == TK_RES_WRITE) consomeToken(TK_RES_WRITE);
        else                                    consomeToken(TK_RES_WRITELN);

        consomeToken(TK_BRA_O_PAR);
        exp();

        while (ultimo_token->tipo == TK_OP_COMMA)
        {
            consomeToken(TK_OP_COMMA);
            exp();
        }
        consomeToken(TK_BRA_C_PAR);
    }
}

void parser::cmdFor()
{
    consomeToken(TK_RES_FOR);
    consomeToken(TK_BRA_O_PAR);

    if (ultimo_token->tipo != TK_END_STATEMENT)
    {
        cmdS();
        while (ultimo_token->tipo == TK_OP_COMMA)
        {
            consomeToken(TK_OP_COMMA);
            cmdS();
        }
    }
    consomeToken(TK_END_STATEMENT);

    exp();
    consomeToken(TK_END_STATEMENT);

    if(ultimo_token->tipo != TK_BRA_C_PAR)
    {
        cmdS();
        while (ultimo_token->tipo == TK_OP_COMMA)
        {
            consomeToken(TK_OP_COMMA);
            cmdS();
        }
    }

    consomeToken(TK_BRA_C_PAR);

    if (ultimo_token->tipo == TK_BRA_O_CUR) blocoCmd();
    else                                    cmdT();
}

void parser::cmdIf()
{
    consomeToken(TK_RES_IF);
    consomeToken(TK_BRA_O_PAR);

    exp();

    consomeToken(TK_BRA_C_PAR);

    consomeToken(TK_RES_THEN);

    if (ultimo_token->tipo == TK_BRA_O_CUR) blocoCmd();
    else                                    cmdT();

    if (ultimo_token->tipo == TK_RES_ELSE)
    {
        consomeToken(TK_RES_ELSE);

        if (ultimo_token->tipo == TK_BRA_O_CUR) blocoCmd();
        else                                    cmdT();
    }

}

void parser::cmdT()
{
    if      (ultimo_token->tipo == TK_END_STATEMENT) consomeToken(TK_END_STATEMENT);
    else if (ultimo_token->tipo == TK_RES_FOR)       cmdFor();
    else if (ultimo_token->tipo == TK_RES_IF)        cmdIf();
    else
    {
        cmdS();
        consomeToken(TK_END_STATEMENT);
    }
}

void parser::exp()
{
    soma();

    if (
           (ultimo_token->tipo == TK_OP_EQ) ||
           (ultimo_token->tipo == TK_OP_NE) ||
           (ultimo_token->tipo == TK_OP_GT) ||
           (ultimo_token->tipo == TK_OP_LT) ||
           (ultimo_token->tipo == TK_OP_GE) ||
           (ultimo_token->tipo == TK_OP_LE)
       )
    {
        consomeToken(ultimo_token->tipo);
        soma();
    }
}

void parser::soma()
{

    if      (ultimo_token->tipo == TK_OP_PLUS)  consomeToken(TK_OP_PLUS);
    else if (ultimo_token->tipo == TK_OP_MINUS) consomeToken(TK_OP_MINUS);

    termo();

    while (
            (ultimo_token->tipo == TK_OP_PLUS)  ||
            (ultimo_token->tipo == TK_OP_MINUS) ||
            (ultimo_token->tipo == TK_RES_OR)
          )
    {
        consomeToken(ultimo_token->tipo);
        termo();
    }
}

void parser::termo()
{
    fator();

    while (
            (ultimo_token->tipo == TK_OP_MUL)     ||
            (ultimo_token->tipo == TK_OP_SLASH)   ||
            (ultimo_token->tipo == TK_OP_PERCENT) ||
            (ultimo_token->tipo == TK_RES_AND)
          )
    {
        consomeToken(ultimo_token->tipo);
        fator();
    }
}

void parser::fator()
{
    if (ultimo_token->tipo == TK_RES_NOT)
    {
        consomeToken(TK_RES_NOT);
        fator();
    }
    else if (ultimo_token->tipo == TK_BRA_O_PAR)
    {
        consomeToken(TK_BRA_O_PAR);
        exp();
        consomeToken(TK_BRA_C_PAR);
    }
    else if (ultimo_token->tipo == TK_ID)
    {
        consomeToken(TK_ID);

        if(ultimo_token->tipo == TK_BRA_O_SQR)
        {
            consomeToken(TK_BRA_O_SQR);
            exp();
            consomeToken(TK_BRA_C_SQR);
        }
    }
    else consomeToken(TK_CONST);
}
