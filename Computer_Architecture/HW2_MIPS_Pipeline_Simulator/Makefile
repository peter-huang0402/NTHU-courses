# Makefile for pipeline simulator

CC=gcc
CFLAGS=-c -Wall
SOURCES=MIPS_Pipeline_Simulator.c
OBJECTS=$(SOURCES:.c=.o)
EXE=pipeline
INCLUDE_FILES=pipeline.h

all: $(SOURCES) $(EXE)

$(EXE): $(OBJECTS)
	$(CC)  $(OBJECTS) -o $@

%.o:  %.c  ${INCLUDE_FILES}
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.rpt
	rm -rf *.o $(EXE)	
	rm -rf *.bin
