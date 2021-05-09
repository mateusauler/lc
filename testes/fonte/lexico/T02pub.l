/* Exemplo de Programa na linguagem L */

int n;
char nome[40];
final MAXITER='aa';

main {
/* Programa Principal */

  write("Digite seu nome: ");
  readln(nome);
  for (n:=1; n<=MAXITER; n:=n+1)
    writeln("Ola' ",nome);
}
