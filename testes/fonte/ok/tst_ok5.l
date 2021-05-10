int a, v[6];
final f = 7;
char str1[15], str2[15], tmp;

main
{
    str1 := "str1";
    str2 := "str2";

    v[1 * 0]   := 10;
    v[1 / 1]   := 11;
    v[2 % 4]   := 24;
    v[1 + 2]   := 12;
    v[-6 + 10] := -64;
    v[10 - 5]  := 105;

    a := 2 + 2 * 3 + 2 * 4;
    writeln(a); /* 16 */

    writeln(f % 4); /* 3 */

    writeln(f / 4); /* 1 */

    /* 10 11 24 12 -64 105 */
    writeln(v[0 * 1], " ", v[1 * 1], " ", v[18 % 16], " ", v[3], " ", v[4], " ", v[5]);

    if ((a > v[0]) or FALSE) then
    {
        writeln("Linha 29 (deve imprimir)");
    }

    if ((a >= v[2]) and TRUE) then
    {
        writeln("Linha 34 (nao deve imprimir)");
    }
    else
    {
        writeln("Linha 38 (deve imprimir)");
    }

    if ((f >= a) or (f <= v[3])) then
        writeln("Linha 42 (deve imprimir)");
    else
        writeln("Linha 43 (nao deve imprimir)");

    if (("abcd" = "abc") or ("abc" = "abcD") or ("abd" = "abc")) then
        writeln("Linha 47 (nao deve imprimir)");
    else if ((str1 = str2) or (str1 = "abc") or ("abc" = str2)) then
        writeln("Linha 49 (nao deve imprimir)");
    else if (("abc" = "abc") and (str1 = "str1") and ("str2" = str2)) then
        writeln("Linha 51 (deve imprimir)");

    writeln("Valor de str1: '", str1, "'");
    write("Valor de str2: '", str2, "'");

    readln(tmp);
}