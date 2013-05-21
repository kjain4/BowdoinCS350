LDLIBS = 
PLATFORM = $(shell uname)
GLDLIBS =
ifeq ($(PLATFORM), Darwin)
GLDLIBS = -framework AGL -framework OpenGL -framework GLUT -framework Foundation
else
GLDLIBS = -I/usr/include/GL/ -L/usr/lib -L/usr/X11R6/lib64 -lglut -lGLU -lGL -lX11 -lm -lXext -lXi
endif
LDFLAGS  = $(LDLIBS) $(GLDLIBS) -lm

CC = gcc
CFLAGS = -O3 -Wall -DNDEBUG
#CFLAGS = -g3
CC+= $(CFLAGS)


PROGS = oneVis multVis

SINGLE_O_FILES = Single_Main.o compareDouble.o Grid.o Horizon.o Points.o Visibility.o rtimer.o
MULT_O_FILES = Mult_Main.o compareDouble.o Grid.o Horizon.o Points.o Visibility.o rtimer.o

default: $(PROGS)

oneVis: $(SINGLE_O_FILES)
	$(CC) $(LDFLAGS)  $(SINGLE_O_FILES) -o $@

multVis: $(MULT_O_FILES)
	$(CC) $(LDFLAGS)  $(MULT_O_FILES) -o $@

Single_Main.o: Single_Main.c Grid.h Horizon.h Points.h Visibility.h rtimer.h
	$(CC) -c $< -o $@

Mult_Main.o: Mult_Main.c Grid.h Horizon.h Points.h Visibility.h rtimer.h
	$(CC) -c $< -o $@

compareDouble.o: compareDouble.c compareDouble.h
	$(CC) -c $< -o $@

Grid.o: Grid.c Grid.h Points.h
	$(CC) -c $< -o $@

Horizon.o: Horizon.c Horizon.h Points.h
	$(CC) -c $< -o $@

Points.o: Points.c Points.h
	$(CC) -c $< -o $@

Visibility.o: Visibility.c Visibility.h compareDouble.h Points.h Horizon.h
	$(CC) -c $< -o $@

rtimer.o: rtimer.c rtimer.h
	$(CC) -c $< -o $@

clean:
	$(RM) *.o oneVis multVis
