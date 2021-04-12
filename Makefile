SRCDIR  = src
DESTDIR = out
TARGET  = lc

WARNINGS = -Wall
CFLAGS   = -g -Og ${WARNINGS}
LDFLAGS  =

CC = c++

SRC != cd ${SRCDIR} ; ls *.cpp
OBJ  = ${SRC:%.cpp=${DESTDIR}/%.o}

all: ${DESTDIR} ${TARGET}

${OBJ}: ${DESTDIR}/%.o:${SRCDIR}/%.cpp ${SRCDIR}/*.h
	${CC} -c -o $@ $< ${CFLAGS}

${TARGET}: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}

${DESTDIR}:
	mkdir -p ${DESTDIR}

test: all
	scripts/test.sh

clean:
	rm -rf ${DESTDIR}/* ${TARGET} lc.cpp

combine: all
	scripts/combineFiles.sh

.PHONY: all clean test combine
