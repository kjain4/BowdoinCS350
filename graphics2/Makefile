LDLIBS =
GLDLIBS = -framework AGL -framework OpenGL -framework GLUT -framework Foundation
LDFLAGS  = $(LDLIBS) $(GLDLIBS) -lm

CC = gcc -O3 -Wall $(INCLUDEPATH)


PROGS = view1 view2 view3 view4 cube

default: $(PROGS)

view1: view1.o
	$(CC) -o $@ view1.o $(LDFLAGS)

view2: view2.o
	$(CC) -o $@ view2.o $(LDFLAGS)

view3: view3.o
	$(CC) -o $@ view3.o $(LDFLAGS)

view4: view4.o
	$(CC) -o $@ view4.o $(LDFLAGS)

cube: 	cube.o
	$(CC) -o $@ cube.o $(LDFLAGS)


clean::	
	rm view?.o
	rm view?


%.o:: %.c
	$(CC) -c $(INCLUDEPATH)  $< -o $@
