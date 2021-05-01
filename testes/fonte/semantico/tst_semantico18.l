int a, b[4096];
char c;
boolean d;

main
{
    a := 2 + 3 % 4 * 5;
    c := 041h;
    c := 'a';
    d := (2 <= 3) and ((2 = 3) or (2 = 2));

    b[2 * 3 + b[0]] := 15 * b[0];
    b[TRUE] := 6;
}