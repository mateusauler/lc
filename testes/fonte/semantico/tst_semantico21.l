char m[20], n[20];
boolean b;
int i;

main
{
    b := TRUE;

    readln(m);
    readln(n);

    for (i := 0; i < 20; i := i+1)
    {
        if (m[i] <> n[i]) then
        {
            b := FALSE;
            i := 5;
        }

        if ((m[i] = '$') or (n[i] = '$')) then
            i := 5;
    }

    if (b) then
    {
        writeln("Igual");

        for (i := 0; i < 20 or m[i] = '$' ; i := i+1)
            write(m[i]);
    }
    else
    {
        writeln("Diferente");

        for (i := 0; i < 20 or m[i] = '$' ; i := i+1)
            write(m[i]);

        writeln("");

        for (i := 0; i < 20 or n[i] = '$' ; i := i+1)
            write(n[i]);

        writeln("");
    }
}