char m[5], n[5], o[6];
boolean b;

main
{
    b := (2 > 3) or (TRUE and FALSE);

    m := "abc";
    n := "abc";
    if (m = n) then
        writeln("Igual");

    if (m > n) then
        writeln("Maior");
}