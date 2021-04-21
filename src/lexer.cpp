#include <sstream>

#include "lexer.h"
#include "excessoes.h"

// Caractere valido
#define CHAR_VALIDO(c) ((c == '\t' || c == '\n' || c == '\r' || c == EOF) || \
                       (c >= ' '   && c <= '"') || \
                       (c >= '\''  && c <= '[') || \
                       (c >= 'a'   && c <= '{') || \
                       (c == '$'   || c == '%') || \
                       (c == ']'   || c == '_') || \
                       (c == '}'))

#define E_CHAR(c)  ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
#define E_DIGITO(c) (c >= '0' && c <= '9')

token_t::~token_t()
{
    switch (tipo_constante)
    {
        case CONST_BOOL:
            delete (bool*)valor_const;
            break;

        case CONST_HEX:
        case CONST_CHAR:
            delete (char*)valor_const;
            break;

        case CONST_INT:
            delete (int*)valor_const;
            break;

        case CONST_STR:
            delete (std::string*)valor_const;
            break;

        default:
            break;
    }
}

int lexer::get_linha() const
{
    return num_linha;
}

token_t lexer::proximo_token()
{
    estado_t estado = ES_INICIO;

    char c;
    bool backtrack = false;

    token_t tok;

    do
    {
        c = fgetc(arq_fonte);

        // O unico estado que aceita EOF e o estado inicial
        if (c == EOF && estado != ES_INICIO)
            throw eof_inesperado();

        // Caractere invalido
        if (!CHAR_VALIDO(c))
            throw char_invalido();

        switch (estado)
        {
            // Simbolo inicial
            // Ignora espacos e novas linhas (a contabilizacao de linhas e feita depois)
            // Qualquer caractere que nao e especificado explicitamente, e considerado erro
            case ES_INICIO:
                // Limpa o lexema
                tok.lex = "";

                if (E_CHAR(c))
                    estado = ES_ID_NOME;
                else if (c > '0' && E_DIGITO(c))
                    estado = ES_CONST_NUM;
                else
                {
                    switch(c)
                    {
                        case '_':
                            estado = ES_ID_UNDERLINE;
                            break;

                        case '0':
                            estado = ES_CONST_HEX_INICIO;
                            break;

                        case '\'':
                            estado = ES_CONST_CHAR_INICIO;
                            break;

                        case '"':
                            estado = ES_CONST_STR_INTERNO;
                            break;

                        case '/':
                            estado = ES_OP_BARRA;
                            break;

                        case ':':
                            estado = ES_OP_ATRIB_INICIO;
                            break;

                        case '<':
                            estado = ES_OP_LT;
                            break;

                        case '>':
                            estado = ES_OP_GT;
                            break;

                        case '{':
                            tok.tipo_token = TK_GRU_A_CHA;
                            estado = ES_FIM;
                            break;

                        case '}':
                            tok.tipo_token = TK_GRU_F_CHA;
                            estado = ES_FIM;
                            break;

                        case '[':
                            tok.tipo_token = TK_GRU_O_COL;
                            estado = ES_FIM;
                            break;

                        case ']':
                            tok.tipo_token = TK_GRU_F_COL;
                            estado = ES_FIM;
                            break;

                        case '(':
                            tok.tipo_token = TK_GRU_A_PAR;
                            estado = ES_FIM;
                            break;

                        case ')':
                            tok.tipo_token = TK_GRU_F_PAR;
                            estado = ES_FIM;
                            break;

                        case '%':
                            tok.tipo_token = TK_OP_PORCENTO;
                            estado = ES_FIM;
                            break;

                        case '*':
                            tok.tipo_token = TK_OP_MUL;
                            estado = ES_FIM;
                            break;

                        case '+':
                            tok.tipo_token = TK_OP_MAIS;
                            estado = ES_FIM;
                            break;

                        case '-':
                            tok.tipo_token = TK_OP_MENOS;
                            estado = ES_FIM;
                            break;

                        case ',':
                            tok.tipo_token = TK_OP_VIRGULA;
                            estado = ES_FIM;
                            break;

                        case ';':
                            tok.tipo_token = TK_FIM_DECL;
                            estado = ES_FIM;
                            break;

                        case '=':
                            tok.tipo_token = TK_OP_EQ;
                            estado = ES_FIM;
                            break;

                        case EOF:
                            tok.tipo_token = TK_EOF;
                            estado = ES_FIM;
                            break;

                        case ' ':
                        case '\r':
                        case '\n':
                        case '\t':
                            break;

                        default:
                            throw lex_nao_identificado(tok.lex + c);
                    }
                }
                break;

            // {_}+
            case ES_ID_UNDERLINE:
                if (E_CHAR(c)
                 || E_DIGITO(c))
                    estado = ES_ID_NOME;
                else if (c != '_') // Somente {_}+ e invalido
                    throw lex_nao_identificado(tok.lex);

                break;

            // {_}(LETRA | DIGITO){LETRA | DIGITO | _}
            case ES_ID_NOME:
                if (E_CHAR(c)
                 || E_DIGITO(c)
                 || (c == '_'))
                    estado = ES_ID_NOME;
                else
                {
                    tok.tipo_token = TK_ID;
                    estado = ES_FIM;
                    backtrack = true;
                }
                break;

            // 0
            case ES_CONST_HEX_INICIO:
                if (E_DIGITO(c)) // 0(0-9)
                    estado = ES_CONST_HEX_NUM1;
                else if (c >= 'A' && c <= 'F') // 0(A-F)
                    estado = ES_CONST_HEX_ALPHA1;
                else // 0
                {
                    tok.tipo_token = TK_CONST;
                    tok.tipo_constante = CONST_INT;

                    estado = ES_FIM;
                    backtrack = true;
                }
                break;

            // 0(A-F)
            case ES_CONST_HEX_ALPHA1:
                if (E_DIGITO(c)
                || (c >= 'A' && c <= 'F')) // 0(A-F)(A-F | 0-9)
                    estado = ES_CONST_HEX_ALPHA2;
                else
                    throw lex_nao_identificado(tok.lex);

                break;

            // 0(0-9)
            case ES_CONST_HEX_NUM1:
                if (E_DIGITO(c)) // 0(0-9)(0-9)
                    estado = ES_CONST_HEX_NUM2;
                else if (c >= 'A' && c <= 'F') // 0(0-9)(A-F)
                    estado = ES_CONST_HEX_ALPHA2;
                else
                {
                    tok.tipo_token = TK_CONST;
                    tok.tipo_constante = CONST_INT;

                    estado = ES_FIM;
                    backtrack = true;
                }
                break;

            // 0 ((A-F)(A-F | 0-9) | (A-F | 0-9)(A-F))
            case ES_CONST_HEX_ALPHA2:
                if (c == 'h') // 0 ((A-F)(A-F | 0-9) | (A-F | 0-9)(A-F)) h
                {
                    tok.tipo_token = TK_CONST;
                    tok.tipo_constante = CONST_HEX;

                    estado = ES_FIM;
                }
                else
                    throw lex_nao_identificado(tok.lex);

                break;

            // 0(0-9)(0-9)
            case ES_CONST_HEX_NUM2:
                if (E_DIGITO(c)) // 0(0-9)(0-9)(0-9)
                    estado = ES_CONST_NUM;
                else if (c == 'h') // 0(0-9)(0-9)h
                {
                    tok.tipo_token = TK_CONST;
                    tok.tipo_constante = CONST_HEX;

                    estado = ES_FIM;
                }
                else // 0(0-9)(0-9)
                {
                    tok.tipo_token = TK_CONST;
                    tok.tipo_constante = CONST_INT;

                    estado = ES_FIM;
                    backtrack = true;
                }
                break;

            // {0-9}+
            case ES_CONST_NUM:
                if (!E_DIGITO(c))
                {
                    tok.tipo_token = TK_CONST;
                    tok.tipo_constante = CONST_INT;

                    estado = ES_FIM;
                    backtrack = true;
                }
                break;

            // '
            case ES_CONST_CHAR_INICIO:
                switch (c)
                {
                    case '\n':
                    case '\r':
                    case '\'':
                        throw char_invalido();

                    default:
                        estado = ES_CONST_CHAR_INTERNO;
                        break;
                }
                break;

            // '(caractere)
            case ES_CONST_CHAR_INTERNO:
                if (c == '\'') // '(caractere)'
                {
                    tok.tipo_token = TK_CONST;
                    tok.tipo_constante = CONST_CHAR;

                    estado = ES_FIM;
                }
                else throw lex_nao_identificado(tok.lex);
                break;

            // "{caractere}
            case ES_CONST_STR_INTERNO:
                switch (c)
                {
                    case '"': // "{caractere}"
                        tok.tipo_token = TK_CONST;
                        tok.tipo_constante = CONST_STR;
                        estado = ES_FIM;
                        break;

                    case '$':
                    case '\n':
                    case '\r':
                        throw lex_nao_identificado(tok.lex);

                    default:
                        break;
                }
                break;

            // /
            case ES_OP_BARRA:
                if (c == '*') // /*
                    estado = ES_COMENTARIO;
                else // /
                {
                    tok.tipo_token = TK_OP_BARRA;

                    backtrack = true;
                    estado = ES_FIM;
                }
                break;

            // /*{qualquer caractere}
            case ES_COMENTARIO:
				if (c == '*')
					estado = ES_COMENTARIO_FIM;
                break;

            // /*{qualquer caractere}{*}+
            case ES_COMENTARIO_FIM:
                switch (c)
                {
                    case '/': // Fim do comentario
                        estado = ES_INICIO;
                        break;

                    case '*': // Ainda pode finalizar o comentario
                        break;

                    default: // Comentario continua
                        estado = ES_COMENTARIO;
                        break;
                }
                break;

            // :
            case ES_OP_ATRIB_INICIO:
                if(c == '=') // :=
                {
                    tok.tipo_token = TK_OP_ATRIB;
                    estado = ES_FIM;
                }
                else throw lex_nao_identificado(tok.lex);
                break;

            // <
            case ES_OP_LT:
                switch (c)
                {
                    case '=': // <=
                        tok.tipo_token = TK_OP_LE;
                        break;

                    case '>': // <>
                        tok.tipo_token = TK_OP_NE;
                        break;

                    default: // <
                        tok.tipo_token = TK_OP_LT;
                        backtrack = true;
                        break;
                }

                estado = ES_FIM;

                break;

            // >
            case ES_OP_GT:
                switch (c)
                {
                    case '=': // >=
                        tok.tipo_token = TK_OP_GE;
                        break;

                    default: // >
                        tok.tipo_token = TK_OP_GT;
                        backtrack = true;
                        break;
                }

                estado = ES_FIM;

                break;

            default:
                break;
        } // switch (estado)

        // Caso nao tenha lido um caractere que nao pertence ao lexema atual
        if (!backtrack)
        {
            if (c == '\n')
                num_linha++;

            tok.lex += c;
        }

    } while (estado != ES_FIM);

    if (backtrack)
        fseek(arq_fonte, -1, SEEK_CUR);

    int lex_len = tok.lex.length();

    if (tok.tipo_token == TK_ID)
    {
        // Se o lexema tiver mais de 32 caracteres, ignora a partir do 33
        if (lex_len > 32)
            tok.lex.erase(32, tok.lex.length() - 32);

        tok.simbolo = tbl_simbolos.buscar(tok.lex);

        if (tok.simbolo == nullptr) // Caso seja um ID novo
            tok.simbolo = tbl_simbolos.inserir(tok.tipo_token, tok.lex);
        else
        {
            // Pode ser uma palavra reservada
            tok.tipo_token = tok.simbolo->tipo_token;

            // Se estiver na tabela de simbolos e for constante, e boleano
            if (tok.tipo_token == TK_CONST)
                tok.tipo_constante = CONST_BOOL;
        }
    }

    if (tok.tipo_token == TK_CONST)
    {
        switch (tok.tipo_constante)
        {
            case CONST_NULL:
                tok.tam_constante = 0;
                break;

            case CONST_INT:
                tok.tam_constante = 2;
                break;

            case CONST_CHAR:
            case CONST_HEX:
                tok.tam_constante = 1;
                break;

            case CONST_STR:
                tok.tam_constante = lex_len - 1;
                break;

            case CONST_BOOL:
                tok.tam_constante = 1;
                break;
        }
    }

    // Calcula e preenche o valor de um token do tipo constante
    switch (tok.tipo_constante)
    {
        case CONST_BOOL:
            tok.valor_const = new bool(tok.lex == "TRUE");
            break;

        case CONST_HEX:
            tok.valor_const = new char(std::stoi(tok.lex.substr(1, 2), 0, 16));
            break;

        case CONST_CHAR:
            tok.valor_const = new char(tok.lex[1]);
            break;

        case CONST_INT:
            tok.valor_const = new int(std::atoi(tok.lex.c_str()));
            break;

        case CONST_STR:
            tok.valor_const = new std::string(tok.lex.substr(1, tok.tam_constante - 1) + '$');
            break;

        default:
            break;
    }

    return tok;
}
