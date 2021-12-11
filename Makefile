COMPILER 	= gcc
DEBUG 		= -g
FLAGS 		= -pedantic -std=c99 -Wall
LINK_FLAGS 	= $(FLAGS) -fsanitize=address
BINARY 		= imdb
QUERIES 	= query1.csv query2.csv query3.csv
OBJECTS 	= main.o imdbADT.o

all: $(OBJECTS)
	$(COMPILER) $(LINK_FLAGS) $(OBJECTS) -o $(BINARY)

debugger: FLAGS += $(DEBUG)
debugger: all

main.o: main.c imdbADT/imdbADT.h
	$(COMPILER) $(FLAGS) -c main.c

imdbADT.o: imdbADT/imdbADT.c imdbADT/imdbADT.h
	$(COMPILER) $(FLAGS) -c imdbADT/imdbADT.c
