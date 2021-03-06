#include <string>
#include <iostream>
#include <sstream>

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

typedef enum {
	ST_START, // Inicio
	ST_END,   // Fim, deve retornar o token

	ST_ID_UNDERSCORE,       // Somente leu _
	ST_ID_NAME,             // Esta lendo o nome do identificador (max 32 char)

	ST_CONST_HEX_START,     // Leu 0
	ST_CONST_HEX_ALPHA1,    // Leu um [A-F]
	ST_CONST_HEX_ALPHA2,    // Leu dois [A-F]
	ST_CONST_HEX_NUM1,      // Leu 0 seguido de um digito
	ST_CONST_HEX_NUM2,      // Leu 0 seguido de dois digitos
	ST_CONST_NUM,           // Esta lendo constante numerica (nao hexa)

	ST_CONST_CHAR_START,    // Leu '
	ST_CONST_CHAR_INTERNAL, // Leu ' e um caractere imprimivel

	ST_CONST_STR_INTERNAL,  // Leu " e esta lendo caracteres de string

	ST_COMMENT,             // Leu /* e esta lendo comentario
	ST_COMMENT_END,         // Leu * e pode ler /, terminando o comentario

	ST_OP_SLASH,            // Leu /
	ST_OP_ATTRIB_START,     // Leu :
	ST_OP_LT,               // Leu <
	ST_OP_GT,               // Leu >
} state_t;

typedef enum {
	TK_ID,           // Identificador
	TK_CONST,        // Constante

	TK_OP_ATTRIB,     // :=

	TK_OP_LT,         // <
	TK_OP_GT,         // >
	TK_OP_LE,         // <=
	TK_OP_GE,         // >=
	TK_OP_EQ,         // =
	TK_OP_NE,         // <>

	TK_OP_PLUS,       // +
	TK_OP_MINUS,      // -
	TK_OP_MUL,        // *
	TK_OP_SLASH,      // /
	TK_OP_PERCENT,    // %
	TK_OP_COMMA,      // ,

	TK_BRA_O_PAR,     // (
	TK_BRA_C_PAR,     // )
	TK_BRA_O_SQR,     // [
	TK_BRA_C_SQR,     // ]
	TK_BRA_O_CUR,     // {
	TK_BRA_C_CUR,     // }

	TK_END_STATEMENT, // ;
	TK_EOF            // EOF
} token_type_t;

typedef enum {
	CONST_NULL,
	CONST_INT,
	CONST_CHAR,
	CONST_HEX,
	CONST_STR,
	CONST_BOOL
} const_type_t;

typedef struct {
	token_type_t tipo;
	string lex;
	void* simbolo;
	const_type_t tipo_constante;
	int tam_constante;
} token_t;

token_t next_token();

FILE *f;
int num_linha = 1;

int main(int argc, char* argv[])
{
	if (argc != 2)
		return 1;

	f = fopen(argv[1], "r");

	token_t tok;

	do
	{
		tok = next_token();
		string tkname = "ERRO";

		switch (tok.tipo)
		{
			case TK_ID:
				tkname = "TK_ID";
				break;

			case TK_CONST:
				tkname = "TK_CONST";
				break;

			case TK_OP_ATTRIB:
				tkname = "TK_OP_ATTRIB";
				break;

			case TK_OP_LT:
				tkname = "TK_OP_LT";
				break;

			case TK_OP_GT:
				tkname = "TK_OP_GT";
				break;

			case TK_OP_LE:
				tkname = "TK_OP_LE";
				break;

			case TK_OP_GE:
				tkname = "TK_OP_GE";
				break;

			case TK_OP_EQ:
				tkname = "TK_OP_EQ";
				break;

			case TK_OP_NE:
				tkname = "TK_OP_NE";
				break;

			case TK_OP_PLUS:
				tkname = "TK_OP_PLUS";
				break;

			case TK_OP_MINUS:
				tkname = "TK_OP_MINUS";
				break;

			case TK_OP_MUL:
				tkname = "TK_OP_MUL";
				break;

			case TK_OP_SLASH:
				tkname = "TK_OP_SLASH";
				break;

			case TK_OP_PERCENT:
				tkname = "TK_OP_PERCENT";
				break;

			case TK_BRA_O_PAR:
				tkname = "TK_BRA_O_PAR";
				break;

			case TK_BRA_C_PAR:
				tkname = "TK_BRA_C_PAR";
				break;

			case TK_BRA_O_SQR:
				tkname = "TK_BRA_O_SQR";
				break;

			case TK_BRA_C_SQR:
				tkname = "TK_BRA_C_SQR";
				break;

			case TK_BRA_O_CUR:
				tkname = "TK_BRA_O_CUR";
				break;

			case TK_BRA_C_CUR:
				tkname = "TK_BRA_C_CUR";
				break;

			case TK_OP_COMMA:
				tkname = "TK_OP_COMMA";
				break;

			case TK_END_STATEMENT:
				tkname = "TK_END_STATEMENT";
				break;

			case TK_EOF:
				tkname = "TK_EOF";
				break;
		}

		cout << tkname << endl;

	} while (tok.tipo != TK_EOF/* && tok.type != TK_ERR*/);

	fclose(f);

	return 0;
}

token_t next_token()
{
	state_t estado = ST_START;

	char c;
	int lex_len = 0;
	bool backtrack = false, cr = false;

	token_t tok;
	const_type_t tipo_const = CONST_NULL;
	stringstream *stream_lexema = new stringstream();

	do
	{
		c = fgetc(f);
		lex_len++;

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
				lex_len = 1;
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
				if(c == '"')
				{
				    tok.tipo = TK_CONST;
					tipo_const = CONST_STR;

				    estado = ST_END;
				}
				else if(c == '\n' || c == '\r' || c == '$')
				{
				    //TODO: erro
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
				if (c == '/')
					estado = ST_START;
				else
					estado = ST_COMMENT;
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

			default: // switch (estado)
				break;
		}

		if (!backtrack)
		{
			// Caso o caractere anterior seja um CR ou o caractere atual seja um LF, isso e uma quebra de linha
			if (cr || c == 0xA)
				num_linha++;
		
			// CR
			cr = c == 0xD;

			if (!cr && c != 0xA)
				*stream_lexema << c;
		}

	} while (estado != ST_END && c != EOF);

	if (backtrack)
	{
		lex_len--;
		fseek(f, -1, SEEK_CUR);
	}

	tok.lex = stream_lexema->str();

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
