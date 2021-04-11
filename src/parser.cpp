#include "parser.h"
#include "excessoes.h"

parser::parser(FILE *f)
{
    lxr = new lexer(f);
}

parser::~parser()
{
    delete lxr;
}

void parser::exec_parser()
{
    ultimo_token = lxr->proximo_token();
    prog();
}

void parser::consome_token(token_type_t token_esperado)
{
    if (ultimo_token.tipo == token_esperado)
    {
        if (ultimo_token.tipo != TK_EOF)
            ultimo_token = lxr->proximo_token();
    }
    else if (ultimo_token.tipo == TK_EOF) throw eof_inesperado();
    else                                  throw token_invalido(ultimo_token.lex);
}

// {DecVar|DecConst} main BlocoCmd EOF
void parser::prog()
{
    // {DecVar|DecConst}
    while (ultimo_token.tipo != TK_RES_MAIN)
    {
        if (ultimo_token.tipo == TK_RES_FINAL) dec_const(); // final
        else                                   dec_var();   // (int | char | boolean)
    }

    consome_token(TK_RES_MAIN); // main
    bloco_cmd();
    consome_token(TK_EOF); // EOF
}

// (int | char | boolean) Var {, Var} ;
void parser::dec_var()
{
    // (int | char | boolean)
    if      (ultimo_token.tipo == TK_RES_INT)  consome_token(TK_RES_INT);     // int
    else if (ultimo_token.tipo == TK_RES_CHAR) consome_token(TK_RES_CHAR);    // char
    else                                       consome_token(TK_RES_BOOLEAN); // boolean

    var();

    // {, Var}
    while(ultimo_token.tipo == TK_OP_COMMA)
    {
        consome_token(TK_OP_COMMA); // ,
        var();
    }

    consome_token(TK_END_STATEMENT); // ;
}

// final ID = [+|-] CONST ;
void parser::dec_const()
{
    consome_token(TK_RES_FINAL); // final
    consome_token(TK_ID);        // ID
    consome_token(TK_OP_EQ);     // =

    if      (ultimo_token.tipo == TK_OP_PLUS)  consome_token(TK_OP_PLUS);  // +
    else if (ultimo_token.tipo == TK_OP_MINUS) consome_token(TK_OP_MINUS); // -

    consome_token(TK_CONST);         // CONST
    consome_token(TK_END_STATEMENT); // ;
}

// ID [:= [+|-] CONST | "[" CONST "]" ]
void parser::var()
{
    consome_token(TK_ID); // ID

    if (ultimo_token.tipo == TK_OP_ATTRIB) // := [+|-] CONST
    {
        consome_token(TK_OP_ATTRIB); // :=

        if      (ultimo_token.tipo == TK_OP_PLUS)  consome_token(TK_OP_PLUS);  // +
        else if (ultimo_token.tipo == TK_OP_MINUS) consome_token(TK_OP_MINUS); // -

        consome_token(TK_CONST); // CONST
    }
    else if (ultimo_token.tipo == TK_BRA_O_SQR) // "[" CONST "]"
    {
        consome_token(TK_BRA_O_SQR); // [
        consome_token(TK_CONST);     // CONST
        consome_token(TK_BRA_C_SQR); // ]
    }
}

// "{" {CmdT} "}"
void parser::bloco_cmd()
{
    consome_token(TK_BRA_O_CUR); // {

    // {CmdT}
    while (ultimo_token.tipo != TK_BRA_C_CUR) cmd_t();

    consome_token(TK_BRA_C_CUR); // }
}

// ID [ "[" Exp "]" ] := Exp
// readln "(" ID [ "[" Exp "]" ] ")"
// (write | writeln) "(" Exp {, Exp} ")"
void parser::cmd_s()
{
    if (ultimo_token.tipo == TK_ID) // ID [ "[" Exp "]" ] := Exp
    {
        consome_token(TK_ID); // ID

        // [ "[" Exp "]" ]
        if (ultimo_token.tipo == TK_BRA_O_SQR)
        {
            consome_token(TK_BRA_O_SQR); // [
            exp();
            consome_token(TK_BRA_C_SQR); // ]
        }

        consome_token(TK_OP_ATTRIB); // :=
        exp();
    }
    else if (ultimo_token.tipo == TK_RES_READLN) // readln "(" ID [ "[" Exp "]" ] ")"
    {
        consome_token(TK_RES_READLN); // readln
        consome_token(TK_BRA_O_PAR);  // (
        consome_token(TK_ID);         // ID

        // [ "[" Exp "]" ]
        if (ultimo_token.tipo == TK_BRA_O_SQR)
        {
            consome_token(TK_BRA_O_SQR); // [
            exp();
            consome_token(TK_BRA_C_SQR); // ]
        }

        consome_token(TK_BRA_C_PAR); // )

    }
    else // (write | writeln) "(" Exp {, Exp} ")"
    {
        if (ultimo_token.tipo == TK_RES_WRITE) consome_token(TK_RES_WRITE);   // write
        else                                   consome_token(TK_RES_WRITELN); // writeln

        consome_token(TK_BRA_O_PAR); // (
        exp();

        // {, Exp}
        while (ultimo_token.tipo == TK_OP_COMMA) // ,
        {
            consome_token(TK_OP_COMMA);
            exp();
        }

        consome_token(TK_BRA_C_PAR); // )
    }
}

// for "(" [CmdS {, CmdS}] ; Exp ; [CmdS {, CmdS}] ")" (CmdT | BlocoCmd)
void parser::cmd_for()
{
    consome_token(TK_RES_FOR);   // for
    consome_token(TK_BRA_O_PAR); // (

    // [CmdS {, CmdS}]
    if (ultimo_token.tipo != TK_END_STATEMENT)
    {
        cmd_s();

        // {, CmdS}
        while (ultimo_token.tipo == TK_OP_COMMA)
        {
            consome_token(TK_OP_COMMA); // ,
            cmd_s();
        }
    }
    consome_token(TK_END_STATEMENT); // ;

    exp();
    consome_token(TK_END_STATEMENT); // ;

    // [CmdS {, CmdS}]
    if(ultimo_token.tipo != TK_BRA_C_PAR)
    {
        cmd_s();

        // {, CmdS}
        while (ultimo_token.tipo == TK_OP_COMMA)
        {
            consome_token(TK_OP_COMMA); // ,
            cmd_s();
        }
    }

    consome_token(TK_BRA_C_PAR); // )

    // (CmdT | BlocoCmd)
    if (ultimo_token.tipo == TK_BRA_O_CUR) bloco_cmd();
    else                                   cmd_t();
}

// if "(" Exp ")" then (CmdT | BlocoCmd) [else (CmdT | BlocoCmd)]
void parser::cmd_if()
{
    consome_token(TK_RES_IF);    // if
    consome_token(TK_BRA_O_PAR); // (

    exp();

    consome_token(TK_BRA_C_PAR); // )

    consome_token(TK_RES_THEN);  // then

    // (CmdT | BlocoCmd)
    if (ultimo_token.tipo == TK_BRA_O_CUR) bloco_cmd();
    else                                   cmd_t();

    // [else (CmdT | BlocoCmd)]
    if (ultimo_token.tipo == TK_RES_ELSE)
    {
        consome_token(TK_RES_ELSE); // else

        // (CmdT | BlocoCmd)
        if (ultimo_token.tipo == TK_BRA_O_CUR) bloco_cmd();
        else                                   cmd_t();
    }

}

// [CmdS] ; | CmdFor | CmdIf
void parser::cmd_t()
{
    if      (ultimo_token.tipo == TK_END_STATEMENT) consome_token(TK_END_STATEMENT); // ;
    else if (ultimo_token.tipo == TK_RES_FOR)       cmd_for();                       // for
    else if (ultimo_token.tipo == TK_RES_IF)        cmd_if();                        // if
    else
    {
        cmd_s();
        consome_token(TK_END_STATEMENT); // ;
    }
}

// Soma [(=|<>|>|<|>=|<=) Soma]
void parser::exp()
{
    soma();

    // [(=|<>|>|<|>=|<=) Soma]
    if (
           (ultimo_token.tipo == TK_OP_EQ) || // =
           (ultimo_token.tipo == TK_OP_NE) || // <>
           (ultimo_token.tipo == TK_OP_GT) || // >
           (ultimo_token.tipo == TK_OP_LT) || // <
           (ultimo_token.tipo == TK_OP_GE) || // >=
           (ultimo_token.tipo == TK_OP_LE)    // <=
       )
    {
        consome_token(ultimo_token.tipo); // (=|<>|>|<|>=|<=)
        soma();
    }
}

// [+|-] Termo {(+|-|or) Termo}
void parser::soma()
{
    // [+|-]
    if      (ultimo_token.tipo == TK_OP_PLUS)  consome_token(TK_OP_PLUS);  // +
    else if (ultimo_token.tipo == TK_OP_MINUS) consome_token(TK_OP_MINUS); // -

    termo();

    // {(+|-|or) Termo}
    while (
            (ultimo_token.tipo == TK_OP_PLUS)  || // +
            (ultimo_token.tipo == TK_OP_MINUS) || // -
            (ultimo_token.tipo == TK_RES_OR)      // or
          )
    {
        consome_token(ultimo_token.tipo); // (+|-|or)
        termo();
    }
}

// Fator {(*|/|%|and) Fator}
void parser::termo()
{
    fator();

    // {(*|/|%|and) Fator}
    while (
            (ultimo_token.tipo == TK_OP_MUL)     || // *
            (ultimo_token.tipo == TK_OP_SLASH)   || // /
            (ultimo_token.tipo == TK_OP_PERCENT) || // %
            (ultimo_token.tipo == TK_RES_AND)       // and
          )
    {
        consome_token(ultimo_token.tipo); // (*|/|%|and)
        fator();
    }
}

// not Fator | "(" Exp ")" | ID [ "[" Exp "]" ] | CONST
void parser::fator()
{
    if (ultimo_token.tipo == TK_RES_NOT) // not Fator
    {
        consome_token(TK_RES_NOT); // not
        fator();
    }
    else if (ultimo_token.tipo == TK_BRA_O_PAR) // "(" Exp ")"
    {
        consome_token(TK_BRA_O_PAR); // (
        exp();
        consome_token(TK_BRA_C_PAR); // )
    }
    else if (ultimo_token.tipo == TK_ID) // ID [ "[" Exp "]" ]
    {
        consome_token(TK_ID); // ID

        // [ "[" Exp "]" ]
        if(ultimo_token.tipo == TK_BRA_O_SQR)
        {
            consome_token(TK_BRA_O_SQR); // [
            exp();
            consome_token(TK_BRA_C_SQR); // ]
        }
    }
    else consome_token(TK_CONST); // CONST
}
