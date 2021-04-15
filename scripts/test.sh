#!/bin/bash

testes="ok lexico sintatico"
dirfonte=testes/fonte
diresperado=testes/esperado/compilador

erros=""

function cat_erro()
{
    erros="${erros}$1"
}

for d in $testes; do
    for f in $(ls $dirfonte/$d); do
        filename="$dirfonte/$d/$f"
        s1=$(./lc $filename)
        s2=$(./lc < $filename)

        if [ "$s1" = "$s2" ] ; then
            fileresultado=$(echo "$diresperado/$d/$f" | sed "s/\.l/.se/")
            esperado=$(cat $fileresultado)
            if [ "$s1" != "$esperado" ] ; then
                cat_erro "\n$filename:\n"
                cat_erro "Saida inesperada.\n"
                cat_erro "Esperado:\n"
                cat_erro "$esperado\n"
                cat_erro "Gerado:\n"
                cat_erro "$s1\n\n"
            fi
        else
            cat_erro "$filename:\n"
            cat_erro "Saidas incompativeis.\n"
            cat_erro "$s1\n"
            cat_erro "$s2\n"
        fi
    done
done

if [ -z "$erros" ] ; then
    exit 0
fi

printf "%b" "$erros"
exit 1