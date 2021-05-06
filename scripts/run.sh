#!/bin/bash

dir_base=$(pwd)

dir_tst=$dir_base/testes

dir_fonte=$dir_tst/fonte/ok
dir_result=$dir_tst/resultado

dir_asm=$dir_result/asm
dir_exe=$dir_result/exe

mkdir -p $dir_asm

for f in $(ls $dir_fonte); do
    printf "%s: " "$f"
    destino=$(echo "$dir_asm/$f" | sed "s/\.l$/.asm/g")
    $dir_base/lc $dir_fonte/$f $destino
done

if [ ! -z "$(command -v dosbox)" ] && [ ! -z "$(command -v jwasm)" ] ; then
	make -C $dir_result --no-print-directory

	cd $dir_exe
	cmd=$(ls *.EXE | sed -E "s/(^|\\s+)(.+)(\.EXE)/ -c \\2\\3/g")
	dosbox -c "mount c ." -c "c:" $cmd > /dev/null

	cd $dir_base
fi
