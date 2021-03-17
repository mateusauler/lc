#!/bin/bash

filename=lc.cpp
tmpfile=$filename.tmp

srcdir=src

mainh=$srcdir/main.h
maincpp=$srcdir/main.cpp

headers=$(ls $srcdir/*.h | grep -v $mainh)
code=$(ls $srcdir/*.cpp | grep -v $maincpp)

includes=$(grep -h -E "#include <.+>" $mainh $maincpp $includes $code | sort -u)

cat $mainh >> $tmpfile

cat $headers >> $tmpfile
cat $code >> $tmpfile

cat $maincpp >> $tmpfile

sed -E -i "/#include \".+\"/d" $tmpfile
sed -E -i "/#include <.+>/d" $tmpfile
sed -E -i "/using namespace std;/d" $tmpfile

printf "%b" "$includes\n\n" > $filename
printf "%b" "using namespace std;\n\n" >> $filename

cat $tmpfile >> $filename

rm $tmpfile
