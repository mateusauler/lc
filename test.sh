#!/bin/bash

for d in $(ls exemplos); do
    printf "$d:\n\n"
    for f in $(ls exemplos/$d); do
        s1=$(./lc exemplos/$d/$f)
        s2=$(./lc < exemplos/$d/$f)

        printf "$f:\n"
        if [ "$s1" = "$s2" ] ; then
            printf "%b" "$s1"
        else
            printf "Saidas incompativeis.\n"
            printf "%b" "$s1\n"
            printf "%b" "$s2\n"
        fi
        printf "\n\n"
    done
    printf "\n"
done
