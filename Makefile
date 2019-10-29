all: ppm2pgm

ppm2pgm: to_grey.o
	gcc -o ppm2pgm -g ppm2pgm.c -std=c99 $^

to_grey.o:
	nasm -f elf64 to_grey.asm

clean:
	rm -f *.o
	rm -f ppm2pgm


