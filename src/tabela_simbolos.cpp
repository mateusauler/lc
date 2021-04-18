#include "tabela_simbolos.h"

tabela_simbolos::tabela_simbolos()
{
    // Inicializa a tabela de simbolos com as palavras reservadas
    inserir(TK_RES_FINAL,   "final");
    inserir(TK_RES_INT,     "int");
    inserir(TK_RES_CHAR,    "char");
    inserir(TK_RES_BOOLEAN, "boolean");
    inserir(TK_RES_IF,      "if");
    inserir(TK_RES_ELSE,    "else");
    inserir(TK_RES_THEN,    "then");
    inserir(TK_RES_FOR,     "for");
    inserir(TK_RES_AND,     "and");
    inserir(TK_RES_OR,      "or");
    inserir(TK_RES_NOT,     "not");
    inserir(TK_CONST,       "FALSE");
    inserir(TK_CONST,       "TRUE");
    inserir(TK_RES_WRITE,   "write");
    inserir(TK_RES_WRITELN, "writeln");
    inserir(TK_RES_READLN,  "readln");
    inserir(TK_RES_MAIN,    "main");
}

// Insere um registro na tabela
// Retorna um ponteiro para o registro inserido
registro_tabela_simbolos* tabela_simbolos::inserir(token_type_t tipo_token, std::string lexema)
{
    registro_tabela_simbolos* obj = new registro_tabela_simbolos(tipo_token, lexema);

    hash_bkt<registro_tabela_simbolos>* retorno = tabela_hash::inserir(lexema, obj);

    return (registro_tabela_simbolos*)retorno->elemento;
}

// Busca um elemento com a chave "lexema" na tabela
// Retorna um ponteiro para o registro encontrado ou NULL, caso nao tenha sido encontrado
registro_tabela_simbolos* tabela_simbolos::buscar(std::string lexema)
{
    hash_bkt<registro_tabela_simbolos>* retorno = tabela_hash::buscar(lexema);

    if (retorno == NULL) return NULL;

    return (registro_tabela_simbolos*)retorno->elemento;
}
