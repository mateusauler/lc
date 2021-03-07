#ifndef TIPOS_H
#define TIPOS_H

#include <iostream>
#include <sstream>
#include <list>

using namespace std;

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
	ST_COMMENT_NEWLINE,     // Leu \r e pode ler um \n (dentro do comentario, deve contabilizar nova linha)

	ST_NEWLINE,             // Leu \r e pode ler um \n (deve incrementar o contador de linha)

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

	TK_RES_FINAL,     // final
	TK_RES_INT,       // int
	TK_RES_CHAR,      // char
	TK_RES_BOOLEAN,   // boolean

	TK_RES_IF,        // if
	TK_RES_ELSE,      // else
	TK_RES_THEN,      // then

	TK_RES_WHILE,     // while
	TK_RES_FOR,       // for

	TK_RES_AND,       // and
	TK_RES_OR,        // or
	TK_RES_NOT,       // not

	TK_RES_FALSE,     // FALSE
	TK_RES_TRUE,      // TRUE

	TK_RES_WRITE,     // write
	TK_RES_WRITELN,   // writeln
	TK_RES_READLN,    // readln
	TK_RES_MAIN,      // main

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

string nome_tipo_token(token_type_t tipo);
string nome_tipo_constante(const_type_t tipo);

typedef struct registro_tabela_simbolos registro_tabela_simbolos;
struct registro_tabela_simbolos {
    token_type_t tipo_token;
	
    string lexema;
    
    registro_tabela_simbolos() { }
    
    registro_tabela_simbolos(token_type_t t, string l) {
        this->tipo_token = t;
        this->lexema = l;
    }

	static string imprimir_registro_ts(registro_tabela_simbolos *rts)
	{
		if (rts == NULL)
			return "";

		return imprimir_registro_ts(*rts);
	}

	static string imprimir_registro_ts(registro_tabela_simbolos rts)
	{
		stringstream stream;

		stream << nome_tipo_token(rts.tipo_token) << " (" << rts.lexema << ")";

		return stream.str();
	}
};

typedef struct {
	token_type_t tipo;
	string lex;
	registro_tabela_simbolos* simbolo;
	const_type_t tipo_constante;
	int tam_constante;
} token_t;

#endif
