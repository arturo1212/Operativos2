.PHONY:all
.PHONY:clean

all: UsoDisco
UsoDisco: UsoDiscofunc.o UsoDisco.o
	gcc -pthread -o UsoDisco UsoDisco.o UsoDiscofunc.o

UsoDiscofunc.o: UsoDiscofunc.c UsoDiscofunc.h
	gcc -pthread -c UsoDiscofunc.c

UsoDisco.o: UsoDisco.c UsoDiscofunc.h
	gcc -pthread -c UsoDisco.c

clean:
	rm UsoDisco UsoDisco.o UsoDiscofunc.o