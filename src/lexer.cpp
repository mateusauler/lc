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

// Caractere valido dentro de constante do tipo char
#define CHAR_VALIDO_CONST(c) ((c == '\t') || \
                             (c >= ' ' && c <= '"') || \
                             (c >= '(' && c <= '[') || \
                             (c >= 'a' && c <= '{') || \
                             (c == '$' || c == '%' || c == ']' || c == '_' || c == '}'))

#define IS_CHAR(c) ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
#define IS_DIGIT(c) (c >= '0' && c <= '9')

static void parse_valor(token_t& t)
{
    switch (t.tipo_constante)
    {
        case CONST_BOOL:
            t.valor_const = new bool(t.lex == "TRUE");
            break;

        case CONST_HEX:
            t.valor_const = new char(std::stoi(t.lex.substr(1, 2), 0, 16));
            break;

        case CONST_CHAR:
            t.valor_const = new char(t.lex[1]);
            break;

        case CONST_INT:
            t.valor_const = new int(std::atoi(t.lex.c_str()));
            break;

        case CONST_STR:
            t.valor_const = new std::string(t.lex.substr(1, t.tam_constante - 1) + '$');
            break;

        default:
            break;
    }
}

lexer::lexer(FILE *_f) : f(_f)
{
    // Inicializa a tabela de simbolos com as palavras reservadas
    tbl_simbolos.inserir(TK_RES_FINAL,   "final");
    tbl_simbolos.inserir(TK_RES_INT,     "int");
    tbl_simbolos.inserir(TK_RES_CHAR,    "char");
    tbl_simbolos.inserir(TK_RES_BOOLEAN, "boolean");
    tbl_simbolos.inserir(TK_RES_IF,      "if");
    tbl_simbolos.inserir(TK_RES_ELSE,    "else");
    tbl_simbolos.inserir(TK_RES_THEN,    "then");
    tbl_simbolos.inserir(TK_RES_FOR,     "for");
    tbl_simbolos.inserir(TK_RES_AND,     "and");
    tbl_simbolos.inserir(TK_RES_OR,      "or");
    tbl_simbolos.inserir(TK_RES_NOT,     "not");
    tbl_simbolos.inserir(TK_CONST,       "FALSE");
    tbl_simbolos.inserir(TK_CONST,       "TRUE");
    tbl_simbolos.inserir(TK_RES_WRITE,   "write");
    tbl_simbolos.inserir(TK_RES_WRITELN, "writeln");
    tbl_simbolos.inserir(TK_RES_READLN,  "readln");
    tbl_simbolos.inserir(TK_RES_MAIN,    "main");
}

int lexer::get_linha() const
{
    return num_linha;
}

// Le o proximo token do arquivo fonte
token_t lexer::proximo_token()
{
    state_t estado = ST_START;

    char c;
    bool backtrack = false;

    token_t tok;

    do
    {
        c = fgetc(f);

        // O unico estado que aceita EOF e o estado inicial
        if (c == EOF && estado != ST_START)
            throw eof_inesperado();

        // Caractere invalido
        if (!CHAR_VALIDO(c))
            throw char_invalido();

        switch (estado)
        {
            // Simbolo inicial
            // Ignora espacos e novas linhas (a contabilizacao de linhas e feita depois)
            // Qualquer caractere que nao e especificado explicitamente, e considerado erro
            case ST_START:
                // Limpa o lexema
                tok.lex = "";

                if (IS_CHAR(c))
                    estado = ST_ID_NAME;
                else if (c > '0' && IS_DIGIT(c))
                    estado = ST_CONST_NUM;
                else
                {
                    switch(c)
                    {
                        case '_':
                            estado = ST_ID_UNDERSCORE;
                            break;

                        case '0':
                            estado = ST_CONST_HEX_START;
                            break;

                        case '\'':
                            estado = ST_CONST_CHAR_START;
                            break;

                        case '"':
                            estado = ST_CONST_STR_INTERNAL;
                            break;

                        case '/':
                            estado = ST_OP_SLASH;
                            break;

                        case ':':
                            estado = ST_OP_ATTRIB_START;
                            break;

                        case '<':
                            estado = ST_OP_LT;
                            break;

                        case '>':
                            estado = ST_OP_GT;
                            break;

                        case '{':
                            tok.tipo = TK_BRA_O_CUR;
                            estado = ST_END;
                            break;

                        case '}':
                            tok.tipo = TK_BRA_C_CUR;
                            estado = ST_END;
                            break;

                        case '[':
                            tok.tipo = TK_BRA_O_SQR;
                            estado = ST_END;
                            break;

                        case ']':
                            tok.tipo = TK_BRA_C_SQR;
                            estado = ST_END;
                            break;

                        case '(':
                            tok.tipo = TK_BRA_O_PAR;
                            estado = ST_END;
                            break;

                        case ')':
                            tok.tipo = TK_BRA_C_PAR;
                            estado = ST_END;
                            break;

                        case '%':
                            tok.tipo = TK_OP_PERCENT;
                            estado = ST_END;
                            break;

                        case '*':
                            tok.tipo = TK_OP_MUL;
                            estado = ST_END;
                            break;

                        case '+':
                            tok.tipo = TK_OP_PLUS;
                            estado = ST_END;
                            break;

                        case '-':
                            tok.tipo = TK_OP_MINUS;
                            estado = ST_END;
                            break;

                        case ',':
                            tok.tipo = TK_OP_COMMA;
                            estado = ST_END;
                            break;

                        case ';':
                            tok.tipo = TK_END_STATEMENT;
                            estado = ST_END;
                            break;

                        case '=':
                            tok.tipo = TK_OP_EQ;
                            estado = ST_END;
                            break;

                        case EOF:
                            tok.tipo = TK_EOF;
                            estado = ST_END;
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
            case ST_ID_UNDERSCORE:
                if (IS_CHAR(c)
                 || IS_DIGIT(c))
                    estado = ST_ID_NAME;
                else if (c != '_') // Somente {_}+ e invalido
                    throw lex_nao_identificado(tok.lex);

                break;

            // {_}(LETRA | DIGITO){LETRA | DIGITO | _}
            case ST_ID_NAME:
                if (IS_CHAR(c)
                 || IS_DIGIT(c)
                 || (c == '_'))
                    estado = ST_ID_NAME;
                else
                {
                    tok.tipo = TK_ID;
                    estado = ST_END;
                    backtrack = true;
                }
                break;

            // 0
            case ST_CONST_HEX_START:
                if (IS_DIGIT(c)) // 0(0-9)
                    estado = ST_CONST_HEX_NUM1;
                else if (c >= 'A' && c <= 'F') // 0(A-F)
                    estado = ST_CONST_HEX_ALPHA1;
                else // 0
                {
                    tok.tipo = TK_CONST;
                    tok.tipo_constante = CONST_INT;

                    estado = ST_END;
                    backtrack = true;
                }
                break;

            // 0(A-F)
            case ST_CONST_HEX_ALPHA1:
                if (IS_DIGIT(c)
                || (c >= 'A' && c <= 'F')) // 0(A-F)(A-F | 0-9)
                    estado = ST_CONST_HEX_ALPHA2;
                else
                    throw lex_nao_identificado(tok.lex);

                break;

            // 0(0-9)
            case ST_CONST_HEX_NUM1:
                if (IS_DIGIT(c)) // 0(0-9)(0-9)
                    estado = ST_CONST_HEX_NUM2;
                else if (c >= 'A' && c <= 'F') // 0(0-9)(A-F)
                    estado = ST_CONST_HEX_ALPHA2;
                else
                {
                    tok.tipo = TK_CONST;
                    tok.tipo_constante = CONST_INT;

                    estado = ST_END;
                    backtrack = true;
                }
                break;

            // 0(A-F)(A-F | 0-9)
            case ST_CONST_HEX_ALPHA2:
                if (c == 'h') // 0(A-F)(A-F | 0-9)h
                {
                    tok.tipo = TK_CONST;
                    tok.tipo_constante = CONST_HEX;

                    estado = ST_END;
                }
                else
                    throw lex_nao_identificado(tok.lex);

                break;

            // 0(0-9)(0-9)
            case ST_CONST_HEX_NUM2:
                if (IS_DIGIT(c)) // 0(0-9)(0-9)(0-9)
                    estado = ST_CONST_NUM;
                else if (c == 'h') // 0(0-9)(0-9)h
                {
                    tok.tipo = TK_CONST;
                    tok.tipo_constante = CONST_HEX;

                    estado = ST_END;
                }
                else // 0(0-9)(0-9)
                {
                    tok.tipo = TK_CONST;
                    tok.tipo_constante = CONST_INT;

                    estado = ST_END;
                    backtrack = true;
                }
                break;

            // {0-9}+
            case ST_CONST_NUM:
                if (!IS_DIGIT(c))
                {
                    tok.tipo = TK_CONST;
                    tok.tipo_constante = CONST_INT;

                    estado = ST_END;
                    backtrack = true;
                }
                break;

            // '
            case ST_CONST_CHAR_START:
                if (CHAR_VALIDO_CONST(c))
                    estado = ST_CONST_CHAR_INTERNAL;
                else throw char_invalido();
                break;

            // '(caractere)
            case ST_CONST_CHAR_INTERNAL:
                if (c == '\'') // '(caractere)'
                {
                    tok.tipo = TK_CONST;
                    tok.tipo_constante = CONST_CHAR;

                    estado = ST_END;
                }
                else throw lex_nao_identificado(tok.lex);
                break;

            // "{caractere}
            case ST_CONST_STR_INTERNAL:
                switch (c)
                {
                    case '"': // "{caractere}"
                        tok.tipo = TK_CONST;
                        tok.tipo_constante = CONST_STR;
                        estado = ST_END;
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
            case ST_OP_SLASH:
                if (c == '*') // /*
                    estado = ST_COMMENT;
                else // /
                {
                    tok.tipo = TK_OP_SLASH;

                    backtrack = true;
                    estado = ST_END;
                }
                break;

            // /*{qualquer caractere}
            case ST_COMMENT:
				if (c == '*')
					estado = ST_COMMENT_END;
                break;

            // /*{qualquer caractere}{*}+
            case ST_COMMENT_END:
                switch (c)
                {
                    case '/': // Fim do comentario
                        estado = ST_START;
                        break;

                    case '*': // Ainda pode finalizar o comentario
                        break;

                    default: // Comentario continua
                        estado = ST_COMMENT;
                        break;
                }
                break;

            // :
            case ST_OP_ATTRIB_START:
                if(c == '=') // :=
                {
                    tok.tipo = TK_OP_ATTRIB;
                    estado = ST_END;
                }
                else throw lex_nao_identificado(tok.lex);
                break;

            // <
            case ST_OP_LT:
                switch (c)
                {
                    case '=': // <=
                        tok.tipo = TK_OP_LE;
                        break;

                    case '>': // <>
                        tok.tipo = TK_OP_NE;
                        break;

                    default: // <
                        tok.tipo = TK_OP_LT;
                        backtrack = true;
                        break;
                }

                estado = ST_END;

                break;

            // >
            case ST_OP_GT:
                switch (c)
                {
                    case '=': // >=
                        tok.tipo = TK_OP_GE;
                        break;

                    default: // >
                        tok.tipo = TK_OP_GT;
                        backtrack = true;
                        break;
                }

                estado = ST_END;

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

    } while (estado != ST_END);

    if (backtrack)
        fseek(f, -1, SEEK_CUR);

    int lex_len = tok.lex.length();

    if (tok.tipo == TK_ID)
    {
        // Se o lexema tiver mais de 32 caracteres, ignora a partir do 33
        if (lex_len > 32)
            tok.lex.erase(32, tok.lex.length() - 32);

        tok.simbolo = tbl_simbolos.buscar(tok.lex);

        if (tok.simbolo == nullptr) // Caso seja um ID novo
            tok.simbolo = tbl_simbolos.inserir(tok.tipo, tok.lex);
        else
        {
            // Pode ser uma palavra reservada
            tok.tipo = tok.simbolo->tipo_token;

            // Se estiver na tabela de simbolos e seja constante, e boleano
            if (tok.tipo == TK_CONST)
                tok.tipo_constante = CONST_BOOL;
        }
    }

    if (tok.tipo == TK_CONST)
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
                tok.tam_constante = 1;
                break;

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

    parse_valor(tok);

    return tok;
}
