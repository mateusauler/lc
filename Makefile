SRCDIR = src
DESTDIR = out
TARGET = lc

WARNINGS = -Wall #-Wno-unused-variable -Wno-unused-but-set-variable

CFLAGS   = -g -O0 ${WARNINGS}
LDFLAGS  =

CC = g++

SRC = main.cpp
OBJ = ${SRC:%.cpp=${DESTDIR}/%.o}

all: ${DESTDIR} ${TARGET}

${OBJ}: ${DESTDIR}/%.o : ${SRCDIR}/%.cpp
	${CC} -c -o $@ $< ${CFLAGS}

${TARGET}: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}

${DESTDIR}:
	mkdir -p ${DESTDIR}

clean:
	rm -rf ${DESTDIR}/ ${TARGET}

.PHONY: all clean
