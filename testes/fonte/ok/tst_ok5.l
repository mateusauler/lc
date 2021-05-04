/**/
/***/
/* */
/* **/

/* inteiros */
int i_a;
int i_b, i_c;
int i_d :=4, i_e[8], i_f, i_g:= - 2;

/* caracteres */
char c_a;
char c_b, c_c;
char c_d :='4', c_e[32], c_f, c_g:= 023h, c_h:=00Fh;

/* boleanos */
boolean b_a;
boolean b_b, b_c;
boolean b_d :=TRUE, b_e[32], b_f, b_g:= FALSE;

/* constantes */
final f_a = 0;
final f_b = -2;
final f_c = TRUE;
final f_d = 'f';
final f_e = 0C0h;

/* mais inteiros */
int i, j;

/* main */
main
{
    /* este programa nao faz sentido */

    i_a := -5;
    i_b := -2*i_a-1;
    i_e[4-2] := i_b%3;

    readln(i_c);
    readln(i_e[0]);

    for (i := 0, j := 0; i < i_b; i := i + 1)
        for (j := 0; j < i_b; j := j + 1)
        {
            writeln("i: ", i, " j: ", j);
            writeln("");
        }

    if (2 + 3 > i_a) then
        if (i_a < 0) then
            i_a := 0;
        else
        {
            i_a := i_b;
            i_b := i_b + 1;
        }

    b_a := not f_c or (7 * 6 - 5 > i_c % 150) and (not FALSE);
}