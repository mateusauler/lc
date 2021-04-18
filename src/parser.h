#pragma once

#include "lexer.h"

class parser : public lexer
{

public:

    using lexer::lexer;

    // Executa o parsing no arquivo passado para o construtor
    void exec_parser();

private:

    token_t token_lido; // Ultimo token gerado pelo analisador lexico

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

    // Comando terminado
    void cmd_t();

    // Expressao
    void exp();

    // Lista de somas
    void soma();

    // Termo de somas (lista de multiplicacoes)
    void termo();

    // Fator de multiplicacoes
    void fator();

};
