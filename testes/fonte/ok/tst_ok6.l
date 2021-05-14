int i;
char c;

main
{
	i := - 12;
	writeln("-12 = ", i);

	i := 1 + 2;
	writeln("1 + 2 = ", i);

	i := 5 - 3;
	writeln("5 - 3 = ", i);

	i := 2 * 5;
	writeln("2 * 5 = ", i);

	i := -2 * 5;
	writeln("-2 * 5 = ", i);

	i := 8 / 4;
	writeln("8 / 4 = ", i);

	i := 7 % 4;
	writeln("7 % 4 = ", i);

	i := 1 + 2*3 - 4 / 2 + 5 % 2;
	writeln("1 + 2*3 - 4 / 2 + 5 % 2 = ", i);

	readln(c);
}
