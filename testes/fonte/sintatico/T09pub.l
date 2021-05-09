/* Exemplo de Programa na linguagem L */

Int n;
char nome[40];
final MAXITER=10;

main {
/* Programa Principal */

  write("Digite seu nome: ");
  readln(nome);
  for (n:=1; n<=MAXITER; n:=n+1)
    writeln("Ola' ",nome);
}
