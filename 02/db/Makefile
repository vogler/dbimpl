# Generic GNUMakefile

# Just a snippet to stop executing under other make(1) commands
# that won't understand these lines
ifneq (,)
This makefile requires GNU Make.
endif

PROGRAM = BufferManager
C_FILES := $(wildcard buffer/*.cpp)
OBJS := $(patsubst %.cpp, %.o, $(C_FILES))
CC = g++
CFLAGS = -pthread -g -Werror
LDFLAGS =

all: $(PROGRAM)

$(PROGRAM): .depend $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $(PROGRAM)

depend: .depend

.depend: cmd = g++ -MM -MF depend $(var); cat depend >> .depend;
.depend:
	@echo "Generating dependencies..."
	@$(foreach var, $(C_FILES), $(cmd))
	@rm -f depend

-include .depend

# These are the pattern matching rules. In addition to the automatic
# variables used here, the variable $* that matches whatever % stands for
# can be useful in special cases.
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

%: %.cpp
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f .depend *.o

.PHONY: clean depend
