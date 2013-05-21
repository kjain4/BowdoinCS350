LDLIBS = 
PLATFORM = $(shell uname)
GLDLIBS =
ifeq ($(PLATFORM), Darwin)
GLDLIBS = -framework AGL -framework OpenGL -framework GLUT -framework Foundation
else
GLDLIBS = -I/usr/include/GL/ -L/usr/lib -L/usr/X11R6/lib64 -lglut -lGLU -lGL -lX11 -lm -lXext -lXi
endif
LDFLAGS  = $(LDLIBS) $(GLDLIBS) -lm

CC = gcc -O3 -Wall -DNDEBUG

PROGS = oneVis multVis

SINGLE_O_FILES = Single_Main.o Grid.o Horizon.o Points.o Visibility.o rtimer.o
MULT_O_FILES = Mult_Main.o Grid.o Horizon.o Points.o Visibility.o rtimer.o

default: $(PROGS)

oneVis: $(SINGLE_O_FILES)
	$(CC) $(LDFLAGS)  $(SINGLE_O_FILES) -o $@

multVis: $(MULT_O_FILES)
	$(CC) $(LDFLAGS)  $(MULT_O_FILES) -o $@\

Single_Main.o: Single_Main.c Grid.o Horizon.o Points.o Visibility.o rtimer.o
	$(CC) -c $< -o $@

Mult_Main.o: Mult_Main.c Grid.o Horizon.o Points.o Visibility.o rtimer.o
	$(CC) -c $< -o $@

Grid.o: Grid.c Grid.h Points.o
	$(CC) -c $< -o $@

Horizon.o: Horizon.c Horizon.h Points.o
	$(CC) -c $< -o $@

Points.o: Points.c Points.h
	$(CC) -c $< -o $@

Visibility.o: Visibility.c Visibility.h Points.o Horizon.o
	$(CC) -c $< -o $@

rtimer.o: rtimer.c rtimer.h
	$(CC) -c $< -o $@

clean:
	$(RM) *.o oneVis multVis
