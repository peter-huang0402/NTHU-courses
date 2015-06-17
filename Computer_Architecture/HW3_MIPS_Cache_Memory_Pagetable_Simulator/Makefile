# Makefile for CMP simulator

CC=gcc
CFLAGS=-c -Wall
SOURCES=MIPS_Cache_Memory_Pagetable_Simulator.c
OBJECTS=$(SOURCES:.c=.o)
EXE=CMP

all: $(SOURCES) $(EXE)

$(EXE): $(OBJECTS)
	$(CC)  $(OBJECTS) -o $@

%.o:  %.c 
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.rpt
	rm -rf *.o $(EXE)	
	rm -rf *.bin
