char m[5], n[5], o[6];
int i := 1;

main
{
    m[i * i * 2] := 's';
    m := "abcd";
    n := m;
    o := n;
    m := "abcde";
}