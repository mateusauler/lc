SRCDIR=asm
DESTDIR=exe

SRC != cd ${SRCDIR} ; ls *.asm
OBJ  = ${SRC:%.asm=${DESTDIR}/%.EXE}

all: ${DESTDIR} ${OBJ}

${OBJ}: ${DESTDIR}/%.EXE:${SRCDIR}/%.asm
	jwasm -mz -q -Fo $@ $<

${DESTDIR}:
	mkdir -p ${DESTDIR}

.PHONY: all
