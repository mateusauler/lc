#include "tabela_simbolos.h"
#include "parser.h"
#include "excessoes.h"
#include "main.h"

                        /*    TAB          LF          CR */
#define CHAR_VALIDO(c) ((c == 0x9  || c == 0xA || c == 0xD || c == EOF) || \
                       (c >= ' '  && c <= '"') || \
                       (c >= '\'' && c <= '[') || \
                       (c >= 'a'  && c <= '{') || \
                       (c == '$'  || c == '%') || \
                       (c == ']'  || c == '_') || \
                       (c == '}'))

                                 /* TAB */
#define CHAR_VALIDO_CONST(c) ((c == 0x9) || \
                             (c >= ' ' && c <= '"') || \
                             (c >= '(' && c <= '[') || \
                             (c >= 'a' && c <= '{') || \
                             (c == '$' || c == '%' || c == ']' || c == '_' || c == '}'))

#define IS_CHAR(c) ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
#define IS_DIGIT(c) (c >= '0' && c <= '9')

void imprimir_erro(string lex, tipo_erro_t terro);

FILE *f;
int num_linha = 1;
list<token_t> *registro_lexico;
tabela_simbolos *tbl_simbolos;

int
main(int argc, char* argv[])
{
    if (argc != 2)
        return 1;

    f = fopen(argv[1], "r");

    tbl_simbolos = new tabela_simbolos(128);
    registro_lexico = new list<token_t>();

    tbl_simbolos->inserir(TK_RES_FINAL,   "final");
    tbl_simbolos->inserir(TK_RES_INT,     "int");
    tbl_simbolos->inserir(TK_RES_CHAR,    "char");
    tbl_simbolos->inserir(TK_RES_BOOLEAN, "boolean");
    tbl_simbolos->inserir(TK_RES_IF,      "if");
    tbl_simbolos->inserir(TK_RES_ELSE,    "else");
    tbl_simbolos->inserir(TK_RES_THEN,    "then");
    tbl_simbolos->inserir(TK_RES_FOR,     "for");
    tbl_simbolos->inserir(TK_RES_AND,     "and");
    tbl_simbolos->inserir(TK_RES_OR,      "or");
    tbl_simbolos->inserir(TK_RES_NOT,     "not");
    tbl_simbolos->inserir(TK_CONST,       "FALSE");
    tbl_simbolos->inserir(TK_CONST,       "TRUE");
    tbl_simbolos->inserir(TK_RES_WRITE,   "write");
    tbl_simbolos->inserir(TK_RES_WRITELN, "writeln");
    tbl_simbolos->inserir(TK_RES_READLN,  "readln");
    tbl_simbolos->inserir(TK_RES_MAIN,    "main");

    try
    {
        parser *p = new parser();
        p->execParser();
    }
    catch (excProgramaFonte e)
    {
        imprimir_erro(e.l, e.terro);

        fclose(f);
        return -1;
    }

    cout << num_linha << " linhas compiladas." << endl;

    fclose(f);

    return 0;
}

token_t
proximo_token()
{
    state_t estado = ST_START;

    char c;
    bool backtrack = false;

    token_t tok;
    tok.simbolo = nullptr;

    const_type_t tipo_const = CONST_NULL;
    stringstream *stream_lexema = new stringstream();

    do
    {
        c = fgetc(f);

        if (c == EOF && estado != ST_START)
            throw excProgramaFonte(ERR_EOF_INESPERADO);

        if (!CHAR_VALIDO(c))
            throw excProgramaFonte(ERR_CHAR_INVALIDO);

        switch (estado)
        {
            case ST_START:
                stream_lexema->str("");

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

                        default:
                            break;
                    }
                }
                break;

            case ST_ID_UNDERSCORE:
                if (IS_CHAR(c)
                 || IS_DIGIT(c))
                    estado = ST_ID_NAME;
                else if (c != '_')
                    throw excProgramaFonte(stream_lexema->str(), ERR_LEX_NAO_IDENTIFICADO);

                break;

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

                    tok.simbolo = tbl_simbolos->pesquisar(stream_lexema->str());

                    if (tok.simbolo == NULL)
                        tok.simbolo = tbl_simbolos->inserir(tok.tipo, stream_lexema->str());
                    else
                        tok.tipo = tok.simbolo->tipo_token;

                    if (tok.tipo == TK_CONST) // boleano
                        tipo_const = CONST_BOOL;
                }
                break;

            case ST_CONST_HEX_START:
                if (IS_DIGIT(c))
                    estado = ST_CONST_HEX_NUM1;
                else if (c >= 'A' && c <= 'F')
                    estado = ST_CONST_HEX_ALPHA1;
                else
                {
                    tok.tipo = TK_CONST;
                    tipo_const = CONST_INT;

                    estado = ST_END;
                    backtrack = true;
                }
                break;

            case ST_CONST_HEX_ALPHA1:
                if (IS_DIGIT(c)
                || (c >= 'A' && c <= 'F'))
                    estado = ST_CONST_HEX_ALPHA2;
                else
                {
                    *stream_lexema << c;
                    throw excProgramaFonte(stream_lexema->str(), ERR_LEX_NAO_IDENTIFICADO);
                }

                break;

            case ST_CONST_HEX_NUM1:
                if (IS_DIGIT(c))
                    estado = ST_CONST_HEX_NUM2;
                else if (c >= 'A' && c <= 'F')
                    estado = ST_CONST_HEX_ALPHA2;
                else
                {
                    tok.tipo = TK_CONST;
                    tipo_const = CONST_INT;

                    estado = ST_END;
                    backtrack = true;
                }
                break;

            case ST_CONST_HEX_ALPHA2:
                if (c == 'h')
                {
                    tok.tipo = TK_CONST;
                    tipo_const = CONST_HEX;

                    estado = ST_END;
                }
                else
                {
                    *stream_lexema << c;
                    throw excProgramaFonte(stream_lexema->str(), ERR_LEX_NAO_IDENTIFICADO);
                }

                break;

            case ST_CONST_HEX_NUM2:
                if (IS_DIGIT(c))
                    estado = ST_CONST_NUM;
                else if (c == 'h')
                {
                    tok.tipo = TK_CONST;
                    tipo_const = CONST_HEX;

                    estado = ST_END;
                }
                else
                {
                    tok.tipo = TK_CONST;
                    tipo_const = CONST_INT;

                    estado = ST_END;
                    backtrack = true;
                }
                break;

            case ST_CONST_NUM:
                if (!IS_DIGIT(c))
                {
                    tok.tipo = TK_CONST;
                    tipo_const = CONST_INT;

                    estado = ST_END;
                    backtrack = true;
                }
                break;

            case ST_CONST_CHAR_START:
                if (CHAR_VALIDO_CONST(c))
                    estado = ST_CONST_CHAR_INTERNAL;
                else throw excProgramaFonte(ERR_CHAR_INVALIDO);

                break;

            case ST_CONST_CHAR_INTERNAL:
                if (c == '\'')
                {
                    tok.tipo = TK_CONST;
                    tipo_const = CONST_CHAR;

                    estado = ST_END;
                }
                else
                {
                    *stream_lexema << c;
                    throw excProgramaFonte(stream_lexema->str(), ERR_LEX_NAO_IDENTIFICADO);
                }
                break;

            case ST_CONST_STR_INTERNAL:
                switch (c)
                {
                    case '"':
                        tok.tipo = TK_CONST;
                        tipo_const = CONST_STR;
                        estado = ST_END;
                        break;

                    case '$':
                        *stream_lexema << c;
                    case '\n':
                    case '\r':
                        throw excProgramaFonte(stream_lexema->str(), ERR_LEX_NAO_IDENTIFICADO);

                    default:
                        break;
                }
                break;

            case ST_OP_SLASH:
                if (c == '*')
                    estado = ST_COMMENT;
                else
                {
                    tok.tipo = TK_OP_SLASH;

                    backtrack = true;
                    estado = ST_END;
                }
                break;

            case ST_COMMENT:
				if (c == '*')
					estado = ST_COMMENT_END;
                break;

            case ST_COMMENT_END:
                switch (c)
                {
                    case '/':
                        estado = ST_START;
                        break;

                    case '*':
                        break;

                    default:
                        estado = ST_COMMENT;
                        break;
                }
                break;

            case ST_OP_ATTRIB_START:
                if(c == '=')
                {
                    tok.tipo = TK_OP_ATTRIB;
                    estado = ST_END;
                }
                else
                {
                    *stream_lexema << c;
                    throw excProgramaFonte(stream_lexema->str(), ERR_LEX_NAO_IDENTIFICADO);
                }
                break;

            case ST_OP_LT:
                switch (c)
                {
                    case '=':
                        tok.tipo = TK_OP_LE;
                        break;

                    case '>':
                        tok.tipo = TK_OP_NE;
                        break;

                    default:
                        tok.tipo = TK_OP_LT;
                        backtrack = true;
                        break;
                }

                estado = ST_END;

                break;

            case ST_OP_GT:
                switch (c)
                {
                    case '=':
                        tok.tipo = TK_OP_GE;
                        break;

                    default:
                        tok.tipo = TK_OP_GT;
                        backtrack = true;
                        break;
                }

                estado = ST_END;

                break;

            default: // switch (estado)
                break;
        } // switch (estado)

		if (c == '\n')
			num_linha++;

        if (!backtrack)
            *stream_lexema << c;

    } while (estado != ST_END && c != EOF);

    if (backtrack)
        fseek(f, -1, SEEK_CUR);

    tok.lex = stream_lexema->str();
    int lex_len = tok.lex.length();

    if (tok.tipo == TK_ERRO)
        tipo_const = CONST_NULL;

    tok.tipo_constante = tipo_const;
    tok.tam_constante = 0;

    switch (tok.tipo)
    {
        case TK_ID:
            if (lex_len > 32)
                throw excProgramaFonte(stream_lexema->str(), ERR_LEX_NAO_IDENTIFICADO);
            break;

        case TK_CONST:
            switch (tipo_const)
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
            break; // case TK_CONST

        default:
            break;
    } // switch (tok.tipo)

    registro_lexico->push_back(tok);

    return tok;
}

void
imprimir_erro(string lex, tipo_erro_t erro)
{
    stringstream mensagem;
    switch (erro)
    {
        case ERR_OK:
            break;

        case ERR_CHAR_INVALIDO:
            mensagem << "caractere invalido.";
            break;

        case ERR_LEX_NAO_IDENTIFICADO:
            mensagem << "lexema nao identificado [" << lex << "].";
            break;

        case ERR_TOKEN_NAO_ESPERADO:
            mensagem << "token nao esperado [" << lex << "].";
            break;

        case ERR_EOF_INESPERADO:
            mensagem << "fim de arquivo nao esperado.";
            break;
    }

    cerr << num_linha << endl;
    cerr << mensagem.str() << endl;
}