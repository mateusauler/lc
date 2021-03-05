					  /*    TAB          LF          CR */
#define VALIDCHAR(c) ((c == 0x9  || c == 0xA || c == 0xD) || \
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

	TK_OP_ATTRIB,    // :=

	TK_OP_LT,        // <
	TK_OP_GT,        // >
	TK_OP_LE,        // <=
	TK_OP_GE,        // >=
	TK_OP_EQ,        // =

	TK_OP_PLUS,      // +
	TK_OP_MINUS,     // -
	TK_OP_MUL,       // *
	TK_OP_SLASH,     // /
	TK_OP_PERCENT,   // %

	TK_BRA_O_PAR,    // (
	TK_BRA_C_PAR,    // )
	TK_BRA_O_SQR,    // [
	TK_BRA_C_SQR,    // ]
	TK_BRA_O_CUR,    // {
	TK_BRA_C_CUR,    // }

	TK_END_STATEMENT // ;
} token_type_t;

typedef enum {
	CONST_INT,
	CONST_CHAR,
	CONST_HEX,
	CONST_STR,
	CONST_BOOL
} const_type_t;

int main(int argc, char* argv[])
{

	return 0;
}
