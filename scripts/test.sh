#!/bin/bash

testes="ok lexico sintatico semantico"
dirfonte=testes/fonte
diresperado=testes/esperado/compilador
dirresultado=testes/resultado

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
            destino=$(echo "$dirresultado/$f" | sed "s/\.l$/.asm/g")
            s1=$(./lc $filename $destino)
            s2=$(./lc < $filename)

            if [ "$s1" = "$s2" ] ; then
                fileresultado=$(echo "$diresperado/$d/$f" | sed "s/\.l/.se/")
                if [ -f $fileresultado ] ; then
                    esperado=$(cat $fileresultado)
                    if [ "$s1" != "$esperado" ] ; then
                        cat_erro "\n$filename:"
                        cat_erro "Saida inesperada."
                        cat_erro "Esperado:"
                        cat_erro "$esperado"
                        cat_erro "Gerado:"
                        cat_erro "$s1\n"
                    fi
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

make combined > /dev/null

run_tests

if [ ! -z "$erros" ] ; then
    cat_erro "Arquivos combinados:"
    printf "%b" "$erros"
    exit 1
fi
