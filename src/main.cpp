#include "hash_tbl.cpp"
#include "tipos.h"

using namespace std;

					  /*    TAB          LF          CR */
#define VALIDCHAR(c) ((c == 0x9  || c == 0xA || c == 0xD || c == EOF) || \
					  (c >= ' '  && c <= '"') || \
					  (c >= '\'' && c <= '[') || \
					  (c >= 'a'  && c <= '{') || \
					  (c == '$'  || c == '%') || \
					  (c == ']'  || c == '_') || \
					  (c == '}'))

						    /* TAB */
#define VALIDSTRCHAR(c) ((c == 0x9) || \
                         (c >= '(' && c <= '[') || \
                         (c >= 'a' && c <= '{') || \
                         (c == ' ' || c == '!' || c == '%' || c == ']' || c == '_' || c == '}'))

						     /* TAB */
#define PRINTABLECHAR(c) ((c == 0x9) || \
						  (c >= ' ' && c <= '"') || \
                          (c >= '(' && c <= '[') || \
                          (c >= 'a' && c <= '{') || \
                          (c == '$' || c == '%' || c == ']' || c == '_' || c == '}'))

token_t next_token();

FILE *f;
int num_linha = 1;

int
main(int argc, char* argv[])
{
	if (argc != 2)
		return 1;

	f = fopen(argv[1], "r");

	token_t tok;

	do
	{
		int old_linha = num_linha;
		tok = next_token();
		string tkname = "ERRO";

		switch (tok.tipo)
		{
			case TK_ID:
				tkname = "ID";
				break;

			case TK_CONST:
				tkname = "CONST";
				break;

			case TK_OP_ATTRIB:
				tkname = "OP_ATTRIB";
				break;

			case TK_OP_LT:
				tkname = "OP_LT";
				break;

			case TK_OP_GT:
				tkname = "OP_GT";
				break;

			case TK_OP_LE:
				tkname = "OP_LE";
				break;

			case TK_OP_GE:
				tkname = "OP_GE";
				break;

			case TK_OP_EQ:
				tkname = "OP_EQ";
				break;

			case TK_OP_NE:
				tkname = "OP_NE";
				break;

			case TK_OP_PLUS:
				tkname = "OP_PLUS";
				break;

			case TK_OP_MINUS:
				tkname = "OP_MINUS";
				break;

			case TK_OP_MUL:
				tkname = "OP_MUL";
				break;

			case TK_OP_SLASH:
				tkname = "OP_SLASH";
				break;

			case TK_OP_PERCENT:
				tkname = "OP_PERCENT";
				break;

			case TK_BRA_O_PAR:
				tkname = "BRA_O_PAR";
				break;

			case TK_BRA_C_PAR:
				tkname = "BRA_C_PAR";
				break;

			case TK_BRA_O_SQR:
				tkname = "BRA_O_SQR";
				break;

			case TK_BRA_C_SQR:
				tkname = "BRA_C_SQR";
				break;

			case TK_BRA_O_CUR:
				tkname = "BRA_O_CUR";
				break;

			case TK_BRA_C_CUR:
				tkname = "BRA_C_CUR";
				break;

			case TK_OP_COMMA:
				tkname = "OP_COMMA";
				break;

			case TK_END_STATEMENT:
				tkname = "END_STATEMENT";
				break;

			case TK_EOF:
				tkname = "EOF";
				break;
		}

		if (old_linha != num_linha)
			cout << endl << num_linha << '\t';

		cout << tkname << ": \"" << tok.lex << "\"\t";

	} while (tok.tipo != TK_EOF);

	cout << endl;

	fclose(f);

	return 0;
}

token_t
next_token()
{
	state_t estado = ST_START;

	char c;
	bool backtrack = false;

	token_t tok;
	const_type_t tipo_const = CONST_NULL;
	stringstream *stream_lexema = new stringstream();

	do
	{
		c = fgetc(f);

		if (c == EOF && estado != ST_START)
		{
			// TODO: erro
		}

		if (!VALIDCHAR(c))
		{
			// TODO: erro
		}

		switch (estado)
		{
			case ST_START:
				stream_lexema->str("");

				if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
					estado = ST_ID_NAME;
				else if (c >= '0' && c <= '9')
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

						case '\r':
							estado = ST_NEWLINE;
							break;

						case '\n':
							num_linha++;
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
				if ((c >= 'a' && c <= 'z') ||
					(c >= 'A' && c <= 'Z') ||
					(c >= '0' && c <= '9'))
					estado = ST_ID_NAME;
				else if (c == '_')
					estado = ST_ID_UNDERSCORE;
				else
				{
					// TODO: Erro lexema invalido
				}
				break;

			case ST_ID_NAME:
				if ((c >= 'a' && c <= 'z') ||
					(c >= 'A' && c <= 'Z') ||
					(c >= '0' && c <= '9') ||
					(c == '_'))
					estado = ST_ID_NAME;
				else
				{
					tok.tipo = TK_ID;
					estado = ST_END;
					backtrack = true;
				}
				break;

			case ST_CONST_HEX_START:
				if (c >= '0' && c <= '9')
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
				if ((c >= '0' && c <= '9') || 
				    (c >= 'A' && c <= 'F'))
					estado = ST_CONST_HEX_ALPHA2;
				else
				{
					// TODO: erro
				}

				break;

			case ST_CONST_HEX_NUM1:
				if (c >= '0' && c <= '9')
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
					// TODO: erro
				}
					 
				break;

			case ST_CONST_HEX_NUM2:
				if (c >= '0' && c <= '9')
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
				if (!(c >= '0' && c <= '9'))
				{
					tok.tipo = TK_CONST;
					tipo_const = CONST_INT;
					
					estado = ST_END;
					backtrack = true;
				}
				break;

			case ST_CONST_CHAR_START:
				if (PRINTABLECHAR(c))
					estado = ST_CONST_CHAR_INTERNAL;
				else
				{
					// TODO: erro
				}
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
					// TODO: erro
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

					case '\n':
					case '\r':
					case '$':
						// TODO: erro
						break;

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
				switch(c)
				{
					case '*':
						estado = ST_COMMENT_END;
						break;

					case '\n':
						num_linha++;
						break;

					case '\r':
						estado = ST_COMMENT_NEWLINE;
						break;
				}
				break;
			
			case ST_COMMENT_END:
				switch (c)
				{
					case '/':
						estado = ST_START;
						break;

					case '\r':
						estado = ST_COMMENT_NEWLINE;
						break;

					case '\n':
						num_linha++;
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
			        //TODO: erro
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

			case ST_NEWLINE:
				backtrack = (c != '\n');
				num_linha++;
				estado = ST_START;
				break;

			case ST_COMMENT_NEWLINE:
				backtrack = (c != '\n');
				num_linha++;
				estado = ST_COMMENT;
				break;

			default: // switch (estado)
				break;
		} // switch (estado)

		if (!backtrack)
			*stream_lexema << c;

	} while (estado != ST_END && c != EOF);

	if (backtrack)
		fseek(f, -1, SEEK_CUR);

	tok.lex = stream_lexema->str();
	int lex_len = tok.lex.length();

	switch (tok.tipo)
	{	
		case TK_ID:
			// TODO: Guardar endereco na tabela de simbolos
			if (lex_len > 32)
			{
				// TODO: Erro
			}
			break;

		case TK_CONST:
			tok.tipo_constante = tipo_const;

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
			tok.simbolo = NULL;
			tok.tam_constante = 0;
			tok.tipo_constante = CONST_NULL;
			break;
	} // switch (tok.tipo)

	return tok;
}
