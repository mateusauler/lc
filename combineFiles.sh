#!/bin/bash

filename=lc.cpp
tmpfile=$filename.tmp

srcdir=src

maincpp=$srcdir/main.cpp

headers=$(printf "main.h excessoes.h tabela_hash.h tabela_simbolos.h lexer.h parser.h" | sed -E "s/(\s|^)/\1$srcdir\//g")
code=$(ls $srcdir/*.cpp | grep -v $maincpp)

includes=$(grep -h -E "#include <.+>" $maincpp $headers $code | sort -u)

cat $headers > $tmpfile
sed -E -i "/#include (<.+>|\".+\")/d" $tmpfile

cpp -C $tmpfile > $filename

firstline=$(grep -v -m 1 -E "^(#|\\s*$)" $tmpfile)
let linecount=$(grep -n "$firstline" $filename | cut -d':' -f1)-1

tail -n +$linecount $filename > $tmpfile

sed -E -i "/^#.*/d" $tmpfile

cat $code >> $tmpfile

cat $maincpp >> $tmpfile

sed -E -i "/#include (\".+\"|<.+>)/d" $tmpfile
sed -E -i "/using namespace std;/d" $tmpfile

printf "%b" "$includes\n\n" > $filename
printf "%b" "using namespace std;\n\n" >> $filename

cat $tmpfile >> $filename

rm $tmpfile
