LDLIBS =
LDFLAGS  = $(LDLIBS)  -lm 

CXX = gcc  #-arch ppc64  #-arch x86-64 

CXXFLAGS += -O3 -DNDEBUG # -g
CXXFLAGS += -Wall   #-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE 


%.o:%.cc
	$(CXX) -c $(CXXFLAGS) $< -o $@

%.o:%.c
	$(CXX) -c $(CXXFLAGS) $< -o $@


PROGS = multiviewshed

OBJ =  	main.o inmemdistribute.o event.o radial.o rbbst.o \
	rtimer.o  status_structure.o grid.o event_quicksort.o


multiviewshed: $(OBJ)
	$(CXX) $(OBJ) $(CXXFLAGS) $(LDFLAGS) -o $@

default: $(PROGS)

clean::	
	rm *.o
	rm multiviewshed



