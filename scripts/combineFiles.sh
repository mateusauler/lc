#!/bin/bash

filename=lc.cpp
tmpfile=$filename.tmp
srcdir=src
maincpp=$srcdir/main.cpp

headers="enums.h excessoes.h tabela_hash.h tabela_simbolos.h lexer.h parser.h"
headers=$(printf "$headers" | sed -E "s/(\s+|^)/\1$srcdir\//g")
code=$(ls $srcdir/*.cpp | grep -v $maincpp)

includes=$(grep -h -E "#include <.+>" $maincpp $headers $code | sort -u)

function manda_arquivo()
{
    nome=$(echo $1 | sed "s/$srcdir\///")
    printf "%b" "\n/*\n * ----------------------------------------\n" >> $2
    printf "%b" " * $nome\n" >> $2
    printf "%b" " * ----------------------------------------\n */\n" >> $2
    cat $1 >> $2
}

printf "%b" "$includes\n" > $filename

[ -f $tmpfile ] && rm $tmpfile
for f in $headers ; do
    manda_arquivo $f $tmpfile
done

sed -i "/#pragma once/d" $tmpfile

for f in $code ; do
    manda_arquivo $f $tmpfile
done
manda_arquivo $maincpp $tmpfile

sed -E -i "/#include (\".+\"|<.+>)/d" $tmpfile

cat $tmpfile >> $filename

rm $tmpfile
