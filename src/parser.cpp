#include "parser.h"
#include "excessoes.h"

// Executa o parsing no arquivo passado para o construtor
void parser::exec_parser()
{
    token_lido = proximo_token();
    prog();
}

// Verifica se o token lido tem o tipo esperado e le o proximo
void parser::consome_token(token_type_t token_esperado)
{
    if (token_lido.tipo == token_esperado)
    {
        if (token_lido.tipo != TK_EOF)
            token_lido = proximo_token();
    }
    else if (token_lido.tipo == TK_EOF) throw eof_inesperado();
    else                                throw token_invalido(token_lido.lex);
}

// {DecVar|DecConst} main BlocoCmd EOF
void parser::prog()
{
    // {DecVar|DecConst}
    while (token_lido.tipo != TK_RES_MAIN)
    {
        if (token_lido.tipo == TK_RES_FINAL) dec_const(); // final
        else                                 dec_var();   // (int | char | boolean)
    }

    consome_token(TK_RES_MAIN); // main
    bloco_cmd();
    consome_token(TK_EOF); // EOF
}

// (int | char | boolean) Var {, Var} ;
void parser::dec_var()
{
    // (int | char | boolean)
    if      (token_lido.tipo == TK_RES_INT)  consome_token(TK_RES_INT);     // int
    else if (token_lido.tipo == TK_RES_CHAR) consome_token(TK_RES_CHAR);    // char
    else                                     consome_token(TK_RES_BOOLEAN); // boolean

    var();

    // {, Var}
    while(token_lido.tipo == TK_OP_COMMA)
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

    if      (token_lido.tipo == TK_OP_PLUS)  consome_token(TK_OP_PLUS);  // +
    else if (token_lido.tipo == TK_OP_MINUS) consome_token(TK_OP_MINUS); // -

    consome_token(TK_CONST);         // CONST
    consome_token(TK_END_STATEMENT); // ;
}

// ID [:= [+|-] CONST | "[" CONST "]" ]
void parser::var()
{
    consome_token(TK_ID); // ID

    if (token_lido.tipo == TK_OP_ATTRIB) // := [+|-] CONST
    {
        consome_token(TK_OP_ATTRIB); // :=

        if      (token_lido.tipo == TK_OP_PLUS)  consome_token(TK_OP_PLUS);  // +
        else if (token_lido.tipo == TK_OP_MINUS) consome_token(TK_OP_MINUS); // -

        consome_token(TK_CONST); // CONST
    }
    else if (token_lido.tipo == TK_BRA_O_SQR) // "[" CONST "]"
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
    while (token_lido.tipo != TK_BRA_C_CUR) cmd_t();

    consome_token(TK_BRA_C_CUR); // }
}

// ID [ "[" Exp "]" ] := Exp
// readln "(" ID [ "[" Exp "]" ] ")"
// (write | writeln) "(" Exp {, Exp} ")"
void parser::cmd_s()
{
    if (token_lido.tipo == TK_ID) // ID [ "[" Exp "]" ] := Exp
    {
        consome_token(TK_ID); // ID

        // [ "[" Exp "]" ]
        if (token_lido.tipo == TK_BRA_O_SQR)
        {
            consome_token(TK_BRA_O_SQR); // [
            exp();
            consome_token(TK_BRA_C_SQR); // ]
        }

        consome_token(TK_OP_ATTRIB); // :=
        exp();
    }
    else if (token_lido.tipo == TK_RES_READLN) // readln "(" ID [ "[" Exp "]" ] ")"
    {
        consome_token(TK_RES_READLN); // readln
        consome_token(TK_BRA_O_PAR);  // (
        consome_token(TK_ID);         // ID

        // [ "[" Exp "]" ]
        if (token_lido.tipo == TK_BRA_O_SQR)
        {
            consome_token(TK_BRA_O_SQR); // [
            exp();
            consome_token(TK_BRA_C_SQR); // ]
        }

        consome_token(TK_BRA_C_PAR); // )

    }
    else // (write | writeln) "(" Exp {, Exp} ")"
    {
        if (token_lido.tipo == TK_RES_WRITE) consome_token(TK_RES_WRITE);   // write
        else                                 consome_token(TK_RES_WRITELN); // writeln

        consome_token(TK_BRA_O_PAR); // (
        exp();

        // {, Exp}
        while (token_lido.tipo == TK_OP_COMMA) // ,
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
    if (token_lido.tipo != TK_END_STATEMENT)
    {
        cmd_s();

        // {, CmdS}
        while (token_lido.tipo == TK_OP_COMMA)
        {
            consome_token(TK_OP_COMMA); // ,
            cmd_s();
        }
    }
    consome_token(TK_END_STATEMENT); // ;

    exp();
    consome_token(TK_END_STATEMENT); // ;

    // [CmdS {, CmdS}]
    if(token_lido.tipo != TK_BRA_C_PAR)
    {
        cmd_s();

        // {, CmdS}
        while (token_lido.tipo == TK_OP_COMMA)
        {
            consome_token(TK_OP_COMMA); // ,
            cmd_s();
        }
    }

    consome_token(TK_BRA_C_PAR); // )

    // (CmdT | BlocoCmd)
    if (token_lido.tipo == TK_BRA_O_CUR) bloco_cmd();
    else                                 cmd_t();
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
    if (token_lido.tipo == TK_BRA_O_CUR) bloco_cmd();
    else                                 cmd_t();

    // [else (CmdT | BlocoCmd)]
    if (token_lido.tipo == TK_RES_ELSE)
    {
        consome_token(TK_RES_ELSE); // else

        // (CmdT | BlocoCmd)
        if (token_lido.tipo == TK_BRA_O_CUR) bloco_cmd();
        else                                 cmd_t();
    }

}

// [CmdS] ; | CmdFor | CmdIf
void parser::cmd_t()
{
    if      (token_lido.tipo == TK_END_STATEMENT) consome_token(TK_END_STATEMENT); // ;
    else if (token_lido.tipo == TK_RES_FOR)       cmd_for();                       // for
    else if (token_lido.tipo == TK_RES_IF)        cmd_if();                        // if
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
           (token_lido.tipo == TK_OP_EQ) || // =
           (token_lido.tipo == TK_OP_NE) || // <>
           (token_lido.tipo == TK_OP_GT) || // >
           (token_lido.tipo == TK_OP_LT) || // <
           (token_lido.tipo == TK_OP_GE) || // >=
           (token_lido.tipo == TK_OP_LE)    // <=
       )
    {
        consome_token(token_lido.tipo); // (=|<>|>|<|>=|<=)
        soma();
    }
}

// [+|-] Termo {(+|-|or) Termo}
void parser::soma()
{
    // [+|-]
    if      (token_lido.tipo == TK_OP_PLUS)  consome_token(TK_OP_PLUS);  // +
    else if (token_lido.tipo == TK_OP_MINUS) consome_token(TK_OP_MINUS); // -

    termo();

    // {(+|-|or) Termo}
    while (
            (token_lido.tipo == TK_OP_PLUS)  || // +
            (token_lido.tipo == TK_OP_MINUS) || // -
            (token_lido.tipo == TK_RES_OR)      // or
          )
    {
        consome_token(token_lido.tipo); // (+|-|or)
        termo();
    }
}

// Fator {(*|/|%|and) Fator}
void parser::termo()
{
    fator();

    // {(*|/|%|and) Fator}
    while (
            (token_lido.tipo == TK_OP_MUL)     || // *
            (token_lido.tipo == TK_OP_SLASH)   || // /
            (token_lido.tipo == TK_OP_PERCENT) || // %
            (token_lido.tipo == TK_RES_AND)       // and
          )
    {
        consome_token(token_lido.tipo); // (*|/|%|and)
        fator();
    }
}

// not Fator | "(" Exp ")" | ID [ "[" Exp "]" ] | CONST
void parser::fator()
{
    if (token_lido.tipo == TK_RES_NOT) // not Fator
    {
        consome_token(TK_RES_NOT); // not
        fator();
    }
    else if (token_lido.tipo == TK_BRA_O_PAR) // "(" Exp ")"
    {
        consome_token(TK_BRA_O_PAR); // (
        exp();
        consome_token(TK_BRA_C_PAR); // )
    }
    else if (token_lido.tipo == TK_ID) // ID [ "[" Exp "]" ]
    {
        consome_token(TK_ID); // ID

        // [ "[" Exp "]" ]
        if(token_lido.tipo == TK_BRA_O_SQR)
        {
            consome_token(TK_BRA_O_SQR); // [
            exp();
            consome_token(TK_BRA_C_SQR); // ]
        }
    }
    else consome_token(TK_CONST); // CONST
}
