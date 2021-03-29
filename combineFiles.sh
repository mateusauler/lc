#!/bin/bash

filename=lc.cpp
tmpfile=$filename.tmp

srcdir=src

mainh=$srcdir/main.h
maincpp=$srcdir/main.cpp

headers=$(ls $srcdir/*.h | grep -v $mainh)
code=$(ls $srcdir/*.cpp | grep -v $maincpp)

includes=$(grep -h -E "#include <.+>" $mainh $maincpp $headers $code | sort -u)

cat $mainh > $tmpfile

cat $headers >> $tmpfile
sed -E -i "/#include \".+\"/d" $tmpfile
sed -E -i "/#include <.+>/d" $tmpfile

cpp -C $tmpfile > $filename

tail -n +57 $filename > $tmpfile

cat $code >> $tmpfile

cat $maincpp >> $tmpfile

sed -E -i "/#include \".+\"/d" $tmpfile
sed -E -i "/#include <.+>/d" $tmpfile
sed -E -i "/using namespace std;/d" $tmpfile

echo "#define VERDE" > $filename
printf "%b" "$includes\n\n" >> $filename
printf "%b" "using namespace std;\n\n" >> $filename

cat $tmpfile >> $filename

rm $tmpfile
