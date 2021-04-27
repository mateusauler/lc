SRCDIR  = src
DESTDIR = out
TARGET  = lc

WARNINGS = -Wall
CFLAGS   = -g -Og ${WARNINGS}
LDFLAGS  =

CC = c++

SRC  != cd ${SRCDIR} ; ls *.cpp 2> /dev/null
OBJ   = ${SRC:%.cpp=${DESTDIR}/%.o}
HEAD != ls ${SRCDIR}/*.h 2> /dev/null

SRCFILES != ls ${SRCDIR}/* | grep -E "\.(cpp|h)$$"

all: ${DESTDIR} ${TARGET}

${OBJ}: ${DESTDIR}/%.o:${SRCDIR}/%.cpp ${HEAD}
	${CC} -c -o $@ $< ${CFLAGS}

${TARGET}: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}

${DESTDIR}:
	mkdir -p ${DESTDIR}

${TARGET}.cpp: ${SRCFILES}
	scripts/combineFiles.sh

combine: ${TARGET}.cpp

combined: combine
	${CC} -o ${TARGET} ${TARGET}.cpp ${CFLAGS} ${LDFLAGS}

test: all combine
	scripts/test.sh

clean:
	rm -rf ${DESTDIR}/* ${TARGET} ${TARGET}.cpp vgcore.*

.PHONY: all clean test combine combined
