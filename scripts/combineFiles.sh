#!/bin/bash

filename=lc.cpp
tmpfile=$filename.tmp
srcdir=src
maincpp=$srcdir/main.cpp

headers="main.h excessoes.h tabela_hash.h tabela_simbolos.h lexer.h parser.h"
headers=$(printf "$headers" | sed -E "s/(\s+|^)/\1$srcdir\//g")
code=$(ls $srcdir/*.cpp | grep -v $maincpp)

includes=$(grep -h -E "#include <.+>" $maincpp $headers $code | sort -u)

printf "%b" "$includes\n" > $filename

cat $headers > $tmpfile
sed -i "/#pragma once/d" $tmpfile

cat $code $maincpp >> $tmpfile

sed -E -i "/#include (\".+\"|<.+>)/d" $tmpfile

cat $tmpfile >> $filename

rm $tmpfile
