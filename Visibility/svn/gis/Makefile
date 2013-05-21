
PLATFORM = $(shell uname)
SOURCE = src
BUILD = build

MKDIR = mkdir -p

#######################
## Compilation flags ##
#######################
# Debugging
#CFLAGS = -g3 -pg
# Release
CFLAGS = -O3 -DNDEBUG# -pg

CFLAGS+= -I$(SOURCE)/$(COMMON_DIR)
CFLAGS+= -Wall -Wstrict-prototypes

ifeq ($(PLATFORM),Darwin)
## Mac OS X
CFLAGS += -m64 -isystem/usr/local/include
LDFLAGS+= -lc -L/usr/local/lib -framework AGL -framework OpenGL -framework GLUT
LDFLAGS+= -framework Foundation
else
## Linux
LDFLAGS+= -pthread -lGL -lglut
endif


##################
## Source Files ##
##################

# Common
COMMON_DIR = common
COMMON_BLD_DIR = $(BUILD)/$(COMMON_DIR)
COMMON_BLD_DIR:= $(filter-out $(wildcard $(COMMON_BLD_DIR)), $(COMMON_BLD_DIR))
COMMON_SRCS = $(addprefix $(COMMON_DIR)/, 	gridpoint.c \
						datagrid.c \
						rtimer.c \
						rbbst.c \
						pqheap.c \
						visevent.c \
						)
# Brute Alg
BRUTE_DIR = inmem_brute
BRUTE_BLD_DIR = $(BUILD)/$(BRUTE_DIR)
BRUTE_BLD_DIR:= $(filter-out $(wildcard $(BRUTE_BLD_DIR)), $(BRUTE_BLD_DIR))
BRUTE_SRCS = $(addprefix $(BRUTE_DIR)/,	vector.c \
					runthreads.c \
					vis.c \
					)
BRUTE_TARGETS = $(BRUTE_DIR)/bvshed
# Radial2 Alg
RAD2_DIR = inmem_radial2
RAD2_BLD_DIR = $(BUILD)/$(RAD2_DIR)
RAD2_BLD_DIR:= $(filter-out $(wildcard $(RAD2_BLD_DIR)), $(RAD2_BLD_DIR))
RAD2_SRCS  = $(addprefix $(RAD2_DIR)/, radial2.c)
RAD2_TARGETS = $(RAD2_DIR)/main
# IORadial2 Alg
IORAD2_DIR = io_radial2
IORAD2_BLD_DIR = $(BUILD)/$(IORAD2_DIR)
IORAD2_BLD_DIR:= $(filter-out $(wildcard $(IORAD2_BLD_DIR)), $(IORAD2_BLD_DIR))
IORAD2_SRCS  = $(addprefix $(IORAD2_DIR)/, radial2.c)
IORAD2_TARGETS = $(IORAD2_DIR)/main

# All
ALL_SRCS = $(COMMON_SRCS) $(BRUTE_SRCS) $(RAD2_SRCS)
ALL_TARGETS = $(BRUTE_TARGETS) $(RAD2_TARGETS)

# Add build prefix
COMMON_OBJS:= $(addprefix $(BUILD)/,$(COMMON_SRCS:.c=.o))
BRUTE_OBJS := $(addprefix $(BUILD)/,$(BRUTE_SRCS:.c=.o) \
			            $(addsuffix .o,$(BRUTE_TARGETS)))
RAD2_OBJS  := $(addprefix $(BUILD)/,$(RAD2_SRCS:.c=.o) \
			            $(addsuffix .o,$(RAD2_TARGETS)))
IORAD2_OBJS  := $(addprefix $(BUILD)/,$(IORAD2_SRCS:.c=.o) \
			            $(addsuffix .o,$(IORAD2_TARGETS)))
ALL_OBJS:= $(COMMON_OBJS) $(BRUTE_OBJS) $(RAD2_OBJS) $(IORAD2_OBJS)
# Add source prefix
COMMON_SRCS:= $(addprefix $(SOURCE)/,$(COMMON_SRCS))
BRUTE_SRCS := $(addprefix $(SOURCE)/,$(BRUTE_SRCS))
RAD2_SRCS  := $(addprefix $(SOURCE)/,$(RAD2_SRCS))
IORAD2_SRCS  := $(addprefix $(SOURCE)/,$(IORAD2_SRCS))
# Add build prefix for targets
BRUTE_TARGETS := $(addprefix $(BUILD)/,$(BRUTE_TARGETS))
RAD2_TARGETS  := $(addprefix $(BUILD)/,$(RAD2_TARGETS))
IORAD2_TARGETS  := $(addprefix $(BUILD)/,$(IORAD2_TARGETS))


###################
## Build targets ##
###################

# Default target: all 
.PHONY : all common brute rad2 iorad2
all:	common brute rad2 iorad2
common:	$(COMMON_OBJS)
brute:	$(BRUTE_TARGETS)
rad2:	$(RAD2_TARGETS)
iorad2:	$(IORAD2_TARGETS)

# Main executables
$(BRUTE_TARGETS):	$(COMMON_OBJS) $(BRUTE_OBJS)
$(RAD2_TARGETS):	$(COMMON_OBJS) $(RAD2_OBJS)
$(IORAD2_TARGETS):	$(COMMON_OBJS) $(IORAD2_OBJS)

# Require build directories
$(COMMON_OBJS):	$(COMMON_BLD_DIR)
$(BRUTE_OBJS):	$(BRUTE_BLD_DIR)
$(RAD2_OBJS):	$(RAD2_BLD_DIR)
$(IORAD2_OBJS):	$(IORAD2_BLD_DIR)

# Require common objects
$(BRUTE_OBJS):	$(COMMON_OBJS)
$(RAD2_OBJS):	$(COMMON_OBJS)
$(IORAD2_OBJS):	$(COMMON_OBJS)

# Directory creation
$(COMMON_BLD_DIR) $(BRUTE_BLD_DIR) $(RAD2_BLD_DIR) $(IORAD2_BLD_DIR):
	$(MKDIR) $@

# Object compilation and source dependencies
$(ALL_SRCS):	%.c	:	%.h
$(ALL_OBJS):	$(BUILD)/%.o : 	$(SOURCE)/%.c
	$(CC) $(CFLAGS) -c $< -o $@
$(ALL_TARGETS):	$(BUILD)/%   :	$(BUILD)/%.o

# Clean target
.PHONY : clean
clean:
	$(RM) $(ALL_OBJS) $(ALL_TARGETS)
	$(RM) -r $(BUILD)
