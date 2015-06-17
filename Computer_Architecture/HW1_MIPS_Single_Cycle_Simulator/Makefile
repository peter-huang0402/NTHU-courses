# Makefile for single cycle simulator

CC=gcc
CFLAGS=-c -Wall
SOURCES=MIPS_Single_Cycle_Simulator.c
OBJECTS=$(SOURCES:.c=.o)
EXE=single_cycle
INCLUDE_FILES=single_cycle.h

all: $(SOURCES) $(EXE)

$(EXE): $(OBJECTS)
	$(CC)  $(OBJECTS) -o $@

%.o:  %.c  ${INCLUDE_FILES}
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.rpt
	rm -rf *.o $(EXE)	
	rm -rf *.bin
