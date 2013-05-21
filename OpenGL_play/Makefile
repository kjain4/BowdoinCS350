LDLIBS =
GLDLIBS = -framework AGL -framework OpenGL -framework GLUT -framework Foundation
LDFLAGS  = $(LDLIBS) $(GLDLIBS) -lm

CC = gcc -O3 -Wall

PROGS = flower

default: $(PROGS)

flower: flower.o
	$(CC) -o $@ flower.o   $(LDFLAGS)

clean::	
	rm flower.o
	rm flower

%.o:: %.c
	$(CC) -c  $< -o $@
