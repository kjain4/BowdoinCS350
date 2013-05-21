LDLIBS =
GLDLIBS = -framework AGL -framework OpenGL -framework GLUT -framework Foundation
LDFLAGS  = $(LDLIBS) $(GLDLIBS) -lm

CC = gcc -O3 -Wall -m64

PROGS = gis render flowdir flowaccu

GIS_O_FILES = Main.o 
RENDER_O_FILES = Render.o B_Map.o Elev_type.o rtimer.o
FLOWDIR_O_FILES = B_Map.o FD.o Elev_type.o rtimer.o
FLOWACU_O_FILES = B_Map.o FA.o Elev_type.o rtimer.o

default: $(PROGS)

gis: Main.o 
	$(CC) $(LDFLAGS)  $(GIS_O_FILES) -o $@

render: Render.o B_Map.o rtimer.o
	$(CC) $(LDFLAGS) $(RENDER_O_FILES)  -o $@

flowdir: FD.o B_Map.o Elev_type.o rtimer.o
	$(CC) $(LDFLAGS) $(FLOWDIR_O_FILES)  -o $@

flowaccu: FA.o B_Map.o Elev_type.o rtimer.o
	$(CC) $(LDFLAGS) $(FLOWACU_O_FILES)  -o $@

Main.o: Main.c rtimer.o
	$(CC) -c $< -o $@

Render.o: Render.c B_Map.o rtimer.o
	$(CC) -c $< -o $@

B_Map.o: B_Map.c B_Map.h Elev_type.o
	$(CC) -c $< -o $@

Elev_type.o: Elev_type.h
	$(CC) -c $< -o $@

FD.o: FD.c FD.h Elev_type.o rtimer.o
	$(CC) -c $< -o $@

FA.o: FA.c FA.h Elev_type.o rtimer.o
	$(CC) -c $< -o $@

rtimer.o: rtimer.c rtimer.h
	$(CC) -c $< -o $@

clean:
	$(RM) *.o gis render flowdir flowaccu