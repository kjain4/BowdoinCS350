
PLATFORM = $(shell uname)

# PYTHON
CFLAGS  = -pthread -fwrapv -fPIC
CFLAGS += -Wall -Wstrict-prototypes
LDFLAGS = -lreadline

ifeq ($(PLATFORM),Darwin)
## Mac OS X
CFLAGS += -m64 -isystem/usr/local/include
LDFLAGS+= -lc -L/usr/local/lib -framework AGL -framework OpenGL -framework GLUT
LDFLAGS+= -framework Foundation
else
## Linux
LDFLAGS+= -pthread -lGL -lglut
endif

# Debugging
#CFLAGS+= -g3 -pg
# Release
CFLAGS+= -O3 -DNDEBUG# -pg

# Vars
SRCS = rtimer.c vector.c datagrid.c runthreads.c flow.c graphics.c vis.c rbbst.c
OBJS = $(SRCS:.c=.o)

PRGM = fishgis
MAIN = shell
CMDS = $(MAIN) stats flowdir flowaccu bvshed svshed
CMDS+= trials display2d display3d
CMD_MAIN = $(addprefix $(PRGM)-,$(MAIN))
CMD_EXES = $(addprefix $(PRGM)-,$(CMDS))
CMD_SRCS = $(addsuffix .c,$(CMDS_EXES))

LIBRARY = lib$(PRGM).a

TESTS = graphics_test

# Default target: build all the 'scripts'
.PHONY : all
all:	$(PRGM)

static:	$(LIBRARY)

$(PRGM): $(CMD_MAIN) $(CMD_EXES)
	ln -sf $< $@

$(CMD_EXES): $(OBJS)

$(OBJS): %.o: %.h

test:	$(TESTS)

$(TESTS):	$(OBJS)


$(LIBRARY):	$(LIBRARY)($(OBJS))



# Clean target
.PHONY : clean
clean:
	$(RM) $(PRGM) $(CMD_EXES) $(TESTS) $(OBJS) $(LIBRARY)
