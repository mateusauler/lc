int array[50];
int inicio, fim, meio, ordenado, atual, i, j;
char tmp;

main
{
    /* Inicializa o array com valores aleatorios pre-definidos */
    array[0]  := 10775;
    array[1]  := 5569;
    array[2]  := -16160;
    array[3]  := -29362;
    array[4]  := -21118;
    array[5]  := -26786;
    array[6]  := 26049;
    array[7]  := -9292;
    array[8]  := 14501;
    array[9]  := -6664;
    array[10] := -7431;
    array[11] := 7539;
    array[12] := -12914;
    array[13] := -6050;
    array[14] := -21899;
    array[15] := -11175;
    array[16] := 8044;
    array[17] := -19066;
    array[18] := 24322;
    array[19] := 26783;
    array[20] := -11092;
    array[21] := 22509;
    array[22] := 27178;
    array[23] := -5583;
    array[24] := -29030;
    array[25] := -17943;
    array[26] := 11634;
    array[27] := -28511;
    array[28] := -11234;
    array[29] := 28082;
    array[30] := 10187;
    array[31] := 32310;
    array[32] := 883;
    array[33] := 26795;
    array[34] := -29820;
    array[35] := 12533;
    array[36] := -32758;
    array[37] := 28997;
    array[38] := -29526;
    array[39] := 14511;
    array[40] := -10435;
    array[41] := -4189;
    array[42] := -10718;
    array[43] := 9419;
    array[44] := 22530;
    array[45] := 151;
    array[46] := 31013;
    array[47] := -2194;
    array[48] := 13853;
    array[49] := 22567;

    ordenado := 0;
    for (i := 1; i < 50; i := i + 1)
    {
        inicio := 0;
        fim    := ordenado + 1;
        atual  := array[i];

        for (;inicio < fim;)
        {
            meio := inicio + (fim - inicio)/2;

            if (array[meio] <= atual) then
                inicio := meio + 1;
            else
                fim := meio;
        }

        for (j := ordenado; j >= inicio; j := j - 1)
            array[j + 1] := array[j];

        array[inicio] := atual;

        ordenado := ordenado + 1;
    }

    for (i := 0; i < 49; i := i + 1)
        write(array[i], ", ");

    writeln(array[49]);

    readln(tmp);
}