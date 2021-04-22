#include "parser.h"
#include "excessoes.h"

/* Converte um `tipo_constante_t` para um `tipo_dados_t`.
 * `t_const` Tipo de constante a ser convertido.
 * `aceita_null` Se deve aceitar constante nula ou jogar um erro.
 */
static tipo_dados_t converte_tipo_constante(tipo_constante_t t_const, bool aceita_null = true)
{
    switch (t_const)
    {
        case CONST_INT:
            return TP_INT;
            break;

        case CONST_CHAR:
        case CONST_HEX:
            return TP_CHAR;
            break;

        case CONST_BOOL:
            return TP_BOOL;
            break;

        default:
            if (aceita_null)
                return TP_NULL;
            throw tipo_incompativel();
    }
}

void parser::exec_parser()
{
    token_lido = proximo_token();
    prog();
}

void parser::consome_token(tipo_token_t token_esperado)
{
    if (token_lido.tipo_token == token_esperado)
    {
        if (token_lido.tipo_token != TK_EOF)
            token_lido = proximo_token();
    }
    else if (token_lido.tipo_token == TK_EOF) throw eof_inesperado();
    else                                      throw token_invalido(token_lido.lex);
}

void parser::prog()
{
    // {DecVar|DecConst} main BlocoCmd EOF

    // {DecVar|DecConst}
    while (token_lido.tipo_token != TK_RES_MAIN)
    {
        if (token_lido.tipo_token == TK_RES_FINAL) dec_const(); // final
        else                                       dec_var();   // (int | char | boolean)
    }

    consome_token(TK_RES_MAIN); // main
    bloco_cmd();
    consome_token(TK_EOF); // EOF
}

void parser::dec_var()
{
    // (int | char | boolean) Var {, Var} ;
    tipo_dados_t tipo;

    // (int | char | boolean)
    switch (token_lido.tipo_token)
    {
        case TK_RES_INT: // int
            consome_token(TK_RES_INT);
            tipo = TP_INT;
            break;

        case TK_RES_CHAR: // char
            consome_token(TK_RES_CHAR);
            tipo = TP_CHAR;
            break;

        default: // boolean
            consome_token(TK_RES_BOOLEAN);
            tipo = TP_BOOL;
            break;
    }

    var(tipo);

    // {, Var}
    while(token_lido.tipo_token == TK_OP_VIRGULA)
    {
        consome_token(TK_OP_VIRGULA); // ,
        var(tipo);
    }

    consome_token(TK_FIM_DECL); // ;
}

void parser::dec_const()
{
    // final ID = [+|-] CONST ;

    consome_token(TK_RES_FINAL); // final

    registro_tabela_simbolos *rt = token_lido.simbolo;
    std::string lex = token_lido.lex;

    consome_token(TK_ID); // ID

    if (rt->classe != CL_NULL) throw id_ja_declarado(lex);
    rt->classe = CL_CONST;

    consome_token(TK_OP_EQ); // =

    if      (token_lido.tipo_token == TK_OP_MAIS)  consome_token(TK_OP_MAIS);  // +
    else if (token_lido.tipo_token == TK_OP_MENOS) consome_token(TK_OP_MENOS); // -

    tipo_constante_t tipo_constante = token_lido.tipo_constante;

    consome_token(TK_CONST); // CONST

    rt->tipo = converte_tipo_constante(tipo_constante, false);

    consome_token(TK_FIM_DECL); // ;
}

void parser::var(tipo_dados_t tipo)
{
    // ID [:= [+|-] CONST | "[" CONST "]" ]

    registro_tabela_simbolos *rt = token_lido.simbolo;
    std::string lex = token_lido.lex;

    tipo_constante_t tipo_constante;
    tipo_dados_t tipo_convertido;

    consome_token(TK_ID); // ID

    if(rt->classe != CL_NULL) throw id_ja_declarado(lex);
    rt->classe = CL_VAR;
    rt->tipo = tipo;

    switch (token_lido.tipo_token)
    {
        case TK_OP_ATRIB: // := [+|-] CONST
            consome_token(TK_OP_ATRIB); // :=

            if      (token_lido.tipo_token == TK_OP_MAIS)  consome_token(TK_OP_MAIS);  // +
            else if (token_lido.tipo_token == TK_OP_MENOS) consome_token(TK_OP_MENOS); // -

            tipo_constante = token_lido.tipo_constante;

            consome_token(TK_CONST); // CONST

            tipo_convertido = converte_tipo_constante(tipo_constante, false);
            if (tipo_convertido != tipo) throw tipo_incompativel();

            break;

        case TK_GRU_O_COL: // "[" CONST "]"
            consome_token(TK_GRU_O_COL); // [
            consome_token(TK_CONST);     // CONST
            consome_token(TK_GRU_F_COL); // ]
            break;

        default:
            break;
    }
}

void parser::bloco_cmd()
{
    // "{" {CmdT} "}"

    consome_token(TK_GRU_A_CHA); // {

    // {CmdT}
    while (token_lido.tipo_token != TK_GRU_F_CHA) cmd_t();

    consome_token(TK_GRU_F_CHA); // }
}

void parser::cmd_s()
{
    // ID [ "[" Exp "]" ] := Exp
    // readln "(" ID [ "[" Exp "]" ] ")"
    // (write | writeln) "(" Exp {, Exp} ")"

    registro_tabela_simbolos *rt;
    std::string lex;

    switch (token_lido.tipo_token)
    {
        case TK_ID: // ID [ "[" Exp "]" ] := Exp
            rt = token_lido.simbolo;
            lex = token_lido.lex;

            consome_token(TK_ID); // ID

            if (rt->classe == CL_NULL) throw id_nao_declarado(lex);

            // [ "[" Exp "]" ]
            if (token_lido.tipo_token == TK_GRU_O_COL)
            {
                consome_token(TK_GRU_O_COL); // [
                exp();
                consome_token(TK_GRU_F_COL); // ]
            }

            consome_token(TK_OP_ATRIB); // :=
            exp();
            break;

        case TK_RES_READLN: // readln "(" ID [ "[" Exp "]" ] ")"
            consome_token(TK_RES_READLN); // readln
            consome_token(TK_GRU_A_PAR);  // (
            consome_token(TK_ID);         // ID

            // [ "[" Exp "]" ]
            if (token_lido.tipo_token == TK_GRU_O_COL)
            {
                consome_token(TK_GRU_O_COL); // [
                exp();
                consome_token(TK_GRU_F_COL); // ]
            }

            consome_token(TK_GRU_F_PAR); // )
            break;

        default: // (write | writeln) "(" Exp {, Exp} ")"
            if (token_lido.tipo_token == TK_RES_WRITE) consome_token(TK_RES_WRITE);   // write
            else                                       consome_token(TK_RES_WRITELN); // writeln

            consome_token(TK_GRU_A_PAR); // (
            exp();

            // {, Exp}
            while (token_lido.tipo_token == TK_OP_VIRGULA) // ,
            {
                consome_token(TK_OP_VIRGULA);
                exp();
            }

            consome_token(TK_GRU_F_PAR); // )
            break;
    }
}

void parser::cmd_for()
{
    // for "(" [Cmd {, Cmd}] ; Exp ; [Cmd {, Cmd}] ")" (CmdT | BlocoCmd)

    consome_token(TK_RES_FOR);   // for
    consome_token(TK_GRU_A_PAR); // (

    // [Cmd {, Cmd}]
    if (token_lido.tipo_token != TK_FIM_DECL)
    {
        cmd();

        // {, Cmd}
        while (token_lido.tipo_token == TK_OP_VIRGULA)
        {
            consome_token(TK_OP_VIRGULA); // ,
            cmd();
        }
    }
    consome_token(TK_FIM_DECL); // ;

    exp();
    consome_token(TK_FIM_DECL); // ;

    // [Cmd {, Cmd}]
    if(token_lido.tipo_token != TK_GRU_F_PAR)
    {
        cmd();

        // {, Cmd}
        while (token_lido.tipo_token == TK_OP_VIRGULA)
        {
            consome_token(TK_OP_VIRGULA); // ,
            cmd();
        }
    }

    consome_token(TK_GRU_F_PAR); // )

    // (CmdT | BlocoCmd)
    if (token_lido.tipo_token == TK_GRU_A_CHA) bloco_cmd();
    else                                       cmd_t();
}

void parser::cmd_if()
{
    // if "(" Exp ")" then (CmdT | BlocoCmd) [else (CmdT | BlocoCmd)]

    consome_token(TK_RES_IF);    // if
    consome_token(TK_GRU_A_PAR); // (

    exp();

    consome_token(TK_GRU_F_PAR); // )

    consome_token(TK_RES_THEN);  // then

    // (CmdT | BlocoCmd)
    if (token_lido.tipo_token == TK_GRU_A_CHA) bloco_cmd();
    else                                       cmd_t();

    // [else (CmdT | BlocoCmd)]
    if (token_lido.tipo_token == TK_RES_ELSE)
    {
        consome_token(TK_RES_ELSE); // else

        // (CmdT | BlocoCmd)
        if (token_lido.tipo_token == TK_GRU_A_CHA) bloco_cmd();
        else                                       cmd_t();
    }

}

void parser::cmd_t()
{
    // [CmdS] ; | CmdFor | CmdIf

    switch (token_lido.tipo_token)
    {
        case TK_RES_FOR: // for
            cmd_for();
            break;

        case TK_RES_IF: // if
            cmd_if();
            break;

        default:
            if (token_lido.tipo_token != TK_FIM_DECL) cmd_s(); // [CmdS]
            consome_token(TK_FIM_DECL); // ;
            break;
    }
}

void parser::cmd()
{
    // CmdS | CmdFor | CmdIf

    switch (token_lido.tipo_token)
    {
        case TK_RES_FOR: // for
            cmd_for();
            break;

        case TK_RES_IF: // if
            cmd_if();
            break;

        default:
            cmd_s();
    }
}

void parser::exp()
{
    // Soma [(=|<>|>|<|>=|<=) Soma]

    soma();

    // [(=|<>|>|<|>=|<=) Soma]
    switch (token_lido.tipo_token)
    {
        case TK_OP_EQ: // =
        case TK_OP_NE: // <>
        case TK_OP_GT: // >
        case TK_OP_LT: // <
        case TK_OP_GE: // >=
        case TK_OP_LE: // <=
            consome_token(token_lido.tipo_token); // (=|<>|>|<|>=|<=)
            soma();
        break;

        default:
            break;
    }
}

void parser::soma()
{
    // [+|-] Termo {(+|-|or) Termo}

    // [+|-]
    if      (token_lido.tipo_token == TK_OP_MAIS)  consome_token(TK_OP_MAIS);  // +
    else if (token_lido.tipo_token == TK_OP_MENOS) consome_token(TK_OP_MENOS); // -

    termo();

    // {(+|-|or) Termo}
    while (true)
    {
        switch (token_lido.tipo_token)
        {
            case TK_OP_MAIS:  // +
            case TK_OP_MENOS: // -
            case TK_RES_OR:   // or
                consome_token(token_lido.tipo_token); // (+|-|or)
                termo();
                break;

            default:
                return;
        }
    }
}

void parser::termo()
{
    // Fator {(*|/|%|and) Fator}

    fator();

    // {(*|/|%|and) Fator}
    while (true)
    {
        switch (token_lido.tipo_token)
        {
            case TK_OP_MUL:      // *
            case TK_OP_BARRA:    // /
            case TK_OP_PORCENTO: // %
            case TK_RES_AND:     // and
                consome_token(token_lido.tipo_token); // (*|/|%|and)
                fator();
                break;

            default:
                return;
        }
    }
}

void parser::fator()
{
    // not Fator | "(" Exp ")" | ID [ "[" Exp "]" ] | CONST

    switch (token_lido.tipo_token)
    {
        case TK_RES_NOT: // not Fator
            consome_token(TK_RES_NOT); // not
            fator();
            break;

        case TK_GRU_A_PAR: // "(" Exp ")"
            consome_token(TK_GRU_A_PAR); // (
            exp();
            consome_token(TK_GRU_F_PAR); // )
            break;

        case TK_ID: // ID [ "[" Exp "]" ]
            consome_token(TK_ID); // ID

            // [ "[" Exp "]" ]
            if(token_lido.tipo_token == TK_GRU_O_COL)
            {
                consome_token(TK_GRU_O_COL); // [
                exp();
                consome_token(TK_GRU_F_COL); // ]
            }
            break;

        default: // CONST
            consome_token(TK_CONST);
            break;
    }
}
