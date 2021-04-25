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

        case CONST_CHAR:
        case CONST_HEX:
            return TP_CHAR;

        case CONST_BOOL:
            return TP_BOOL;

        case CONST_STR:
            return TP_STRING;

        default:
            if (aceita_null)
                return TP_NULL;
            throw tipo_incompativel();
    }
}

static operador_t converte_operacao(tipo_token_t token, bool aceita_null = true)
{
    switch(token)
    {
        case TK_OP_LT:       // <
            return OP_LT;

        case TK_OP_GT:       // >
            return OP_GT;

        case TK_OP_LE:       // <=
            return OP_LE;

        case TK_OP_GE:       // >=
            return OP_GE;

        case TK_OP_EQ:       // =
            return OP_EQ;

        case TK_OP_NE:       // <>
            return OP_NE;

        case TK_OP_MAIS:     // +
            return OP_ADD;

        case TK_OP_MENOS:    // -
            return OP_SUB;

        case TK_OP_MUL:      // *
            return OP_MUL;

        case TK_OP_BARRA:    // /
            return OP_DIV;

        case TK_OP_PORCENTO: // %
            return OP_MOD;

        case TK_RES_AND:     // and
            return OP_AND;

        case TK_RES_OR:      // or
            return OP_OR;

        case TK_RES_NOT:     // not
            return OP_NOT;

        default:
            if(aceita_null)
                return OP_NULL;
            throw tipo_incompativel();

    }
}

static int byte_tipo( tipo_dados_t tipo )
{
    switch(tipo)
    {
        case TP_INT:
            return 2;

        case TP_CHAR:
        case TP_BOOL:
            return 1;

        default:
            return 0;

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
            //Ação 1
            tipo = TP_INT;
            break;

        case TK_RES_CHAR: // char
            consome_token(TK_RES_CHAR);
            //Ação 2
            tipo = TP_CHAR;
            break;

        default: // boolean
            consome_token(TK_RES_BOOLEAN);
            //Ação 3
            tipo = TP_BOOL;
            break;
    }

    //Ação 4
    var(tipo);

    // {, Var}
    while(token_lido.tipo_token == TK_OP_VIRGULA)
    {
        consome_token(TK_OP_VIRGULA); // ,
        //Ação 4
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

    //Ação 5
    if (rt->classe != CL_NULL) throw id_ja_declarado(lex);
    rt->classe = CL_CONST;

    consome_token(TK_OP_EQ); // =

    if      (token_lido.tipo_token == TK_OP_MAIS)  consome_token(TK_OP_MAIS);  // +
    else if (token_lido.tipo_token == TK_OP_MENOS) consome_token(TK_OP_MENOS); // -

    tipo_constante_t tipo_constante = token_lido.tipo_constante;

    consome_token(TK_CONST); // CONST

    //Ação 6
    rt->tipo = converte_tipo_constante(tipo_constante, false);
    if(rt->tipo == TP_STRING) throw tipo_incompativel();

    consome_token(TK_FIM_DECL); // ;
}

void parser::var(tipo_dados_t tipo)
{
    // ID [:= [+|-] CONST | "[" CONST "]" ]

    registro_tabela_simbolos *rt = token_lido.simbolo;
    std::string lex = token_lido.lex;

    tipo_constante_t tipo_constante;
    tipo_dados_t tipo_convertido;

    void *valor_array;

    consome_token(TK_ID); // ID

    //Ação 7
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

            //Ação 8
            tipo_convertido = converte_tipo_constante(tipo_constante, false);
            if (tipo_convertido != tipo) throw tipo_incompativel();

            break;

        case TK_GRU_A_COL: // "[" CONST "]"
            consome_token(TK_GRU_A_COL); // [

            tipo_convertido = converte_tipo_constante(token_lido.tipo_constante);
            valor_array = token_lido.valor_const;

            consome_token(TK_CONST);     // CONST

            //Ação 9
            if(tipo_convertido != TP_INT) throw tipo_incompativel();
            if( *(int*)valor_array * byte_tipo(tipo) > 8192) throw tam_vet_excede_max();
            rt->tam = *(int*)valor_array;

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

    tipo_dados_t tipo_exp;
    int tamanho_exp;

    int tamanho;

    switch (token_lido.tipo_token)
    {
        case TK_ID: // ID [ "[" Exp "]" ] := Exp
            rt = token_lido.simbolo;
            lex = token_lido.lex;

            consome_token(TK_ID); // ID

            // if(!rt) throw std::exception();

            //Ação 10
            if (rt->classe == CL_NULL) throw id_nao_declarado(lex);
            if (rt->classe == CL_CONST) throw classe_id_incompativel(lex);

            tamanho = rt->tam;

            // [ "[" Exp "]" ]
            if (token_lido.tipo_token == TK_GRU_A_COL)
            {
                consome_token(TK_GRU_A_COL); // [
                exp(tipo_exp, tamanho_exp);

                //Regra 11
                if(tipo_exp != TP_INT) throw tipo_incompativel();
                if(tamanho == 0) throw tipo_incompativel();
                tamanho = 0;

                consome_token(TK_GRU_F_COL); // ]
            }

            consome_token(TK_OP_ATRIB); // :=
            exp(tipo_exp, tamanho_exp);

            //Regra 12
            if(tipo_exp != rt->tipo)
            {
                if(rt->tipo != TP_CHAR && tipo_exp != TP_STRING) throw tipo_incompativel();
                if(tamanho < tamanho_exp) throw tipo_incompativel();
            }
            if(tamanho > 0 ) throw tipo_incompativel();


            break;

        case TK_RES_READLN: // readln "(" ID [ "[" Exp "]" ] ")"
            consome_token(TK_RES_READLN); // readln
            consome_token(TK_GRU_A_PAR);  // (

            rt = token_lido.simbolo;

            consome_token(TK_ID);         // ID

            //Regra 13
            if (rt->classe == CL_NULL) throw id_nao_declarado(lex);
            if (rt->classe == CL_CONST) throw classe_id_incompativel(lex);

            // [ "[" Exp "]" ]
            if (token_lido.tipo_token == TK_GRU_A_COL)
            {
                consome_token(TK_GRU_A_COL); // [
                exp(tipo_exp, tamanho_exp);

                //Regra 14
                if(tipo_exp != TP_INT) throw tipo_incompativel();

                consome_token(TK_GRU_F_COL); // ]
            }

            consome_token(TK_GRU_F_PAR); // )
            break;

        default: // (write | writeln) "(" Exp {, Exp} ")"
            if (token_lido.tipo_token == TK_RES_WRITE) consome_token(TK_RES_WRITE);   // write
            else                                       consome_token(TK_RES_WRITELN); // writeln

            consome_token(TK_GRU_A_PAR); // (
            exp(tipo_exp, tamanho_exp);

            // {, Exp}
            while (token_lido.tipo_token == TK_OP_VIRGULA) // ,
            {
                consome_token(TK_OP_VIRGULA);
                exp(tipo_exp, tamanho_exp);
            }

            consome_token(TK_GRU_F_PAR); // )
            break;
    }
}

void parser::cmd_for()
{
    // for "(" [Cmd {, Cmd}] ; Exp ; [Cmd {, Cmd}] ")" (CmdT | BlocoCmd)

    tipo_dados_t tipo_exp;
    int tamanho_exp;

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

    exp(tipo_exp, tamanho_exp);

    //Regra 15
    if(tipo_exp != TP_BOOL) throw tipo_incompativel();

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

    tipo_dados_t tipo_exp;
    int tamanho_exp;

    consome_token(TK_RES_IF);    // if
    consome_token(TK_GRU_A_PAR); // (

    exp(tipo_exp, tamanho_exp);

    //Regra 16
    if(tipo_exp != TP_BOOL) throw tipo_incompativel();

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

void parser::exp(tipo_dados_t &tipo, int &tamanho)
{
    // Soma [(=|<>|>|<|>=|<=) Soma]

    tipo_dados_t tipo_soma;
    int tamanho_soma;

    tipo_token_t token_operador;
    operador_t operador;

    //Regra 17
    soma(tipo, tamanho);


    // [(=|<>|>|<|>=|<=) Soma]
    switch (token_lido.tipo_token)
    {
        case TK_OP_EQ: // =
        case TK_OP_NE: // <>
        case TK_OP_GT: // >
        case TK_OP_LT: // <
        case TK_OP_GE: // >=
        case TK_OP_LE: // <=

            token_operador = token_lido.tipo_token;

            consome_token(token_lido.tipo_token); // (=|<>|>|<|>=|<=)
            soma(tipo_soma, tamanho_soma);

            operador = converte_operacao(token_operador, false);

            //Regra 18
            if(tipo != tipo_soma)
            {
                if ( (tipo == TP_CHAR && tipo_soma == TP_STRING) ||
                     (tipo == TP_STRING && tipo_soma == TP_CHAR) )
                {
                    if(tamanho == 0 || tamanho_soma == 0) throw tipo_incompativel();
                    if(operador != OP_EQ && operador != OP_NE) throw tipo_incompativel();
                }
                else throw tipo_incompativel();
            }
            if(operador == OP_EQ || operador == OP_NE)
            {
                if  (tamanho != tamanho_soma && (tamanho == 0 || tamanho_soma == 0) ) throw tipo_incompativel();
            }
            else if(tamanho > 0 || tamanho_soma > 0) throw tipo_incompativel();

            tipo = TP_BOOL;

        break;

        default:
            break;
    }
}

void parser::soma(tipo_dados_t &tipo, int &tamanho)
{
    // [+|-] Termo {(+|-|or) Termo}

    tipo_dados_t tipo_termo;
    int tamanho_termo;

    tipo_token_t token_operador;
    operador_t operador;

    // [+|-]
    if      (token_lido.tipo_token == TK_OP_MAIS)  consome_token(TK_OP_MAIS);  // +
    else if (token_lido.tipo_token == TK_OP_MENOS) consome_token(TK_OP_MENOS); // -

    //Regra 19
    termo(tipo, tamanho);

    // {(+|-|or) Termo}
    while (true)
    {
        switch (token_lido.tipo_token)
        {
            case TK_OP_MAIS:  // +
            case TK_OP_MENOS: // -
            case TK_RES_OR:   // or

                token_operador = token_lido.tipo_token;

                consome_token(token_lido.tipo_token); // (+|-|or)
                termo(tipo_termo, tamanho_termo);

                //Regra 20
                if(tipo != tipo_termo) throw tipo_incompativel();
                if(tamanho_termo != 0 || tamanho != 0) throw tipo_incompativel();
                operador = converte_operacao(token_operador, false);
                if(operador == OP_OR && tipo != TP_BOOL) throw tipo_incompativel();
                else if(tipo != TP_INT) throw tipo_incompativel();


                break;

            default:
                return;
        }
    }
}

void parser::termo(tipo_dados_t &tipo, int &tamanho)
{
    // Fator {(*|/|%|and) Fator}

    tipo_dados_t tipo_fator;
    int tamanho_fator;

    tipo_token_t token_operador;
    operador_t operador;

    //Regra 21
    fator(tipo, tamanho);

    // {(*|/|%|and) Fator}
    while (true)
    {
        switch (token_lido.tipo_token)
        {
            case TK_OP_MUL:      // *
            case TK_OP_BARRA:    // /
            case TK_OP_PORCENTO: // %
            case TK_RES_AND:     // and

                token_operador = token_lido.tipo_token;

                consome_token(token_lido.tipo_token); // (*|/|%|and)
                fator(tipo_fator, tamanho_fator);

                //Regra 22
                if(tipo != tipo_fator) throw tipo_incompativel();
                if(tamanho_fator != 0 || tamanho != 0) throw tipo_incompativel();
                operador = converte_operacao(token_operador, false);
                if(operador == OP_AND && tipo != TP_BOOL) throw tipo_incompativel();
                else if(tipo != TP_INT) throw tipo_incompativel();

                break;

            default:
                return;
        }
    }
}

void parser::fator(tipo_dados_t &tipo, int &tamanho)
{
    // not Fator | "(" Exp ")" | ID [ "[" Exp "]" ] | CONST

    tipo_dados_t tipo_exp;
    int tamanho_exp;

    tipo_dados_t tipo_fator;
    int tamanho_fator;

    tipo_constante_t tipo_constante;

    registro_tabela_simbolos *rt = token_lido.simbolo;
    std::string lex = token_lido.lex;


    switch (token_lido.tipo_token)
    {
        case TK_RES_NOT: // not Fator
            consome_token(TK_RES_NOT); // not

            //Regra 23
            tipo = TP_BOOL;
            tamanho = 0;

            fator(tipo_fator, tamanho_fator);

            //Regra 24
            if(tipo_fator != TP_BOOL) throw tipo_incompativel();


            break;

        case TK_GRU_A_PAR: // "(" Exp ")"
            consome_token(TK_GRU_A_PAR); // (
            exp(tipo_exp, tamanho_exp);

            //Regra 25
            tipo = tipo_exp;
            tamanho = tamanho_exp;


            consome_token(TK_GRU_F_PAR); // )
            break;

        case TK_ID: // ID [ "[" Exp "]" ]


            consome_token(TK_ID); // ID

            //Regra 26
            if (rt->classe != CL_NULL)
            {
                tipo = rt->tipo;
                tamanho = rt->tam;
            }
            else throw id_nao_declarado(lex);



            // [ "[" Exp "]" ]
            if(token_lido.tipo_token == TK_GRU_A_COL)
            {
                consome_token(TK_GRU_A_COL); // [
                exp(tipo_exp, tamanho_exp);

                //Regra 27
                if(rt->tam == 0) throw tipo_incompativel();
                else tamanho = 0;

                consome_token(TK_GRU_F_COL); // ]
            }
            break;

        default: // CONST

            //Regra 28
            tipo_constante = token_lido.tipo_constante;
            tipo = converte_tipo_constante(tipo_constante, false);
            tamanho = token_lido.tam_constante;

            consome_token(TK_CONST);
            break;
    }
}
