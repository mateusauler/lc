SRCDIR = src
DESTDIR = out
TARGET = lc

WARNINGS = -Wall

CFLAGS   = -g -O0 ${WARNINGS}
LDFLAGS  =

CC = c++

SRC = main.cpp tabela_hash.cpp tabela_simbolos.cpp parser.cpp source_iterator.cpp
OBJ = ${SRC:%.cpp=${DESTDIR}/%.o}

all: ${DESTDIR} ${TARGET}

${OBJ}: ${DESTDIR}/%.o:${SRCDIR}/%.cpp ${SRCDIR}/*.h
	${CC} -c -o $@ $< ${CFLAGS}

${TARGET}: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}

${DESTDIR}:
	mkdir -p ${DESTDIR}

clean:
	rm -rf ${DESTDIR}/* ${TARGET}

.PHONY: all clean
