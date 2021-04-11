#!/bin/bash

testes="ok lexico sintatico"

for d in $testes; do
    printf "$d:\n\n"
    for f in $(ls exemplos/$d); do
        filename="exemplos/$d/$f"
        s1=$(./lc $filename)
        s2=$(./lc < $filename)

        printf "$f:\n"

        if [ "$s1" = "$s2" ] ; then
            if [ $(command -v valgrind) ] ; then
                valgrind --leak-check=full -q ./lc $filename
            else
                printf "%b\n" "$s1"
            fi
        else
            printf "Saidas incompativeis.\n"
            printf "%b" "$s1\n"
            printf "%b" "$s2\n"
        fi
        printf "\n"
    done
    printf "\n"
done
