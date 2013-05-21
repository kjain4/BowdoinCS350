LDLIBS =
GLDLIBS = -framework AGL -framework OpenGL -framework GLUT -framework Foundation
LDFLAGS  = $(LDLIBS) $(GLDLIBS) -lm

CC = gcc -O3 -Wall

PROGS = simple1 simple2 simple3 simple4

default: $(PROGS)

simple1: simple1.o
	$(CC) -o $@ simple1.o   $(LDFLAGS)

simple2: simple2.o
	$(CC) -o $@ simple2.o  $(LDFLAGS)

simple3: simple3.o
	$(CC) -o $@ simple3.o  $(LDFLAGS)

simple4:simple4.o
	$(CC) -o $@ simple4.o  $(LDFLAGS)

clean::	
	rm simple?.o
	rm simple?



%.o:: %.c
	$(CC) -c  $< -o $@
