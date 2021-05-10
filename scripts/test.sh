#!/bin/bash

testes="ok lexico sintatico semantico"

dir_base=$(pwd)

dir_tst=$dir_base/testes

dir_entrada=$dir_tst/entradas

dir_fonte=$dir_tst/fonte
dir_esperado=$dir_tst/esperado/compilador
dir_res_esperado=$dir_tst/esperado/programa

dir_result=$dir_tst/resultado

dir_asm=$dir_result/asm
dir_exe=$dir_result/exe
dir_saida=$dir_result/saida

erros=""

mkdir -p $dir_asm $dir_saida

function cat_erro()
{
    erros="${erros}$1\n"
}

function run_tests()
{
    for d in $testes; do
        for f in $(ls $dir_fonte/$d); do
            filename="$dir_fonte/$d/$f"
            destino=$(echo "$dir_asm/$f" | sed "s/\.l$/.asm/g")
            s1=$(./lc $filename $destino)
            s2=$(./lc < $filename)

            if [ "$s1" = "$s2" ] ; then
                file_resultado=$(echo "$dir_esperado/$d/$f" | sed "s/\.l/.se/")
                if [ -f $file_resultado ] ; then
                    esperado=$(cat $file_resultado)
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
    cat_erro "Arquivos combinados."
    printf "%b" "$erros"
    exit 1
fi

if [ ! -z "$(command -v dosbox)" ] && [ ! -z "$(command -v jwasm)" ] ; then
	make -C $dir_result --no-print-directory

    cp $dir_entrada/*.e $dir_exe

	cd $dir_exe
	cmd=$(ls *.EXE | sed -E "s/(^|\s+)(.+)(\.EXE)/\2\3 < \2.E > \2.T\n/g")
    echo "$cmd" > progs.bat
    echo "exit" >> progs.bat
	dosbox -c "mount c ." -c "c:" -c progs.bat > /dev/null

    rm *.e progs.bat

	mv *.T $dir_saida
	cd $dir_saida

    for f in $(ls *.T); do
        f_esperado=$(echo $f | sed -E "s/\.T/\.se/g")

        if [ -f "$dir_res_esperado/$f_esperado" ] ; then
            conteudo_esperado=$(cat $dir_res_esperado/$f_esperado)
            conteudo_resultado=$(cat $f)

            if [ "$conteudo_esperado" != "$conteudo_resultado" ] ; then
                cat_erro "\n$f:"
                cat_erro "Resultado final inesperado"
                cat_erro "Esperado:"
                cat_erro "$conteudo_esperado"
                cat_erro "Gerado:"
                cat_erro "$conteudo_resultado\n"
            fi
        fi
    done

    cd $dir_base

    if [ ! -z "$erros" ] ; then
        printf "%b" "$erros"
        exit 1
    fi
fi
