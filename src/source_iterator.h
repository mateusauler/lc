#ifndef SOURCE_ITERATOR_H
#define SOURCE_ITERATOR_H

#include "main.h"

class source_iterator
{
public:
    source_iterator();
    source_iterator(char* filename);
    ~source_iterator();

    char proximo_char();
    void seekInput(int count);

private:
    FILE *f;
    bool from_file;
    string fonte;
    int char_ptr;

};

#endif
