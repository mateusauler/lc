SRCDIR = src
DESTDIR = out
TARGET = lc

WARNINGS = -Wall

CFLAGS   = -g -Og ${WARNINGS}
LDFLAGS  =

CC = c++

SRC = main.cpp tabela_hash.cpp tabela_simbolos.cpp excessoes.cpp lexer.cpp parser.cpp
OBJ = ${SRC:%.cpp=${DESTDIR}/%.o}

all: ${DESTDIR} ${TARGET}

${OBJ}: ${DESTDIR}/%.o:${SRCDIR}/%.cpp ${SRCDIR}/*.h
	${CC} -c -o $@ $< ${CFLAGS}

${TARGET}: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}

${DESTDIR}:
	mkdir -p ${DESTDIR}

test: all
	./test.sh

clean:
	rm -rf ${DESTDIR}/* ${TARGET}

.PHONY: all clean test
