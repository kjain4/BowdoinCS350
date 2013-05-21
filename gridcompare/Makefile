
LDLIBS =
LDFLAGS  = $(LDLIBS)  -lm 

CC = gcc  -O3 -Wall   #-arch ppc64  #-arch x86-64 

PROGS = gridcompare

default: $(PROGS)

gridcompare: gridcompare.o grid.o
		$(CC) $(LDFLAGS) -o $@  gridcompare.o grid.o



%.o:%.c
	$(CC) -c  $< -o $@



clean::	
	rm gridcompare
	rm *.o


