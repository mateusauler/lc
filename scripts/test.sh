#!/bin/bash

testes="ok lexico sintatico"
dirfonte=testes/fonte
diresperado=testes/esperado/compilador

erros=""

function cat_erro()
{
    erros="${erros}$1\n"
}

function run_tests()
{
    for d in $testes; do
        for f in $(ls $dirfonte/$d); do
            filename="$dirfonte/$d/$f"
            s1=$(./lc $filename)
            s2=$(./lc < $filename)

            if [ "$s1" = "$s2" ] ; then
                fileresultado=$(echo "$diresperado/$d/$f" | sed "s/\.l/.se/")
                esperado=$(cat $fileresultado)
                if [ "$s1" != "$esperado" ] ; then
                    cat_erro "\n$filename:"
                    cat_erro "Saida inesperada."
                    cat_erro "Esperado:"
                    cat_erro "$esperado"
                    cat_erro "Gerado:"
                    cat_erro "$s1\n"
                fi
            else
                cat_erro "$filename:"
                cat_erro "Saidas incompativeis."
                cat_erro "$s1"
                cat_erro "$s2"
            fi
        done
    done
}

run_tests

if [ ! -z "$erros" ] ; then
    printf "%b" "$erros"
    exit 1
fi

c++ lc.cpp -o lc

run_tests

if [ ! -z "$erros" ] ; then
    cat_erro "Arquivos combinados:"
    printf "%b" "$erros"
    exit 1
fi
