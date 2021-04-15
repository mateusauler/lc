int i;

main
{
    for (i := 0; i < 10; i := i+1)
        writeln(i);
    
    for (; i < 100; i := i+1)
        writeln(i);

    for (i := 0; i < 10; i := i+1, writeln(i))
    {
        write("i: ");
    }

    for (i := 0; i < 10; i := i+1)
}