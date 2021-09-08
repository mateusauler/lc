SRCDIR=asm
DESTDIR=exe

SRC != cd ${SRCDIR} ; ls *.asm
OBJ  = ${SRC:%.asm=${DESTDIR}/%}

all: ${DESTDIR} ${OBJ}

${OBJ}: ${DESTDIR}/%:${SRCDIR}/%.asm
	@echo "$<:"
	nasm $< -g -w-zeroing -f elf64 -o $@.o
	ld $@.o -o $@
	rm $@.o
	@echo

${DESTDIR}:
	mkdir -p ${DESTDIR}

.PHONY: all
