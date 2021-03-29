#include "source_iterator.h"

source_iterator::source_iterator()
{
    stringstream ss;
    string linha;

    while (getline(cin, linha))
        ss << linha << endl;

    ss << (char)EOF;
    fonte = ss.str();
    char_ptr = 0;
    from_file = false;
}

source_iterator::source_iterator(char* filename)
{
    f = fopen(filename, "r");
    from_file = true;
}

source_iterator::~source_iterator()
{
    if (from_file)
        fclose(f);
}

char source_iterator::proximo_char()
{
    if (from_file)
        return fgetc(f);

    return fonte[++char_ptr - 1];
}

void source_iterator::seekInput(int count)
{
    if (from_file)
        fseek(f, count, SEEK_CUR);

    char_ptr += count;
}
