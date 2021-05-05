#pragma once

#include "lexer.h"

class parser : public lexer
{

public:

    parser(FILE *fonte, FILE *saida) : lexer(fonte), arq_saida(saida) {}

    // Executa o parsing no arquivo passado para o construtor
    void exec_parser();

private:

    // Arquivo de saida
    FILE *arq_saida;

    // Verifica se o token lido tem o tipo esperado e le o proximo
    void consome_token(tipo_token_t token_esperado);

    // Simbolo inicial da gramatica
    void prog();

    // Declaracao de variaveis
    void dec_var();

    // Declaracao de constantes
    void dec_const();

    // Lista de variaveis declaradas
    void var(tipo_dados_t tipo);

    // Bloco de comandos
    void bloco_cmd();

    // Comando simples
    void cmd_s();

    // Comando `for`
    void cmd_for();

    // Comando `if`
    void cmd_if();

    // Comando sem terminacao
    void cmd();

    // Comando terminado
    void cmd_t();

    // Expressao
    void exp(tipo_dados_t &tipo, int &tamanho);

    // Lista de somas
    void soma(tipo_dados_t &tipo, int &tamanho);

    // Termo de somas (lista de multiplicacoes)
    void termo(tipo_dados_t &tipo, int &tamanho);

    // Fator de multiplicacoes
    void fator(tipo_dados_t &tipo, int &tamanho);

};
