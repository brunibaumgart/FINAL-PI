COMPILER 	= gcc
DEBUG 		= -g
FLAGS 		= -pedantic -std=c99 -Wall
LINK_FLAGS 	= $(FLAGS) -fsanitize=address
BINARY 		= imdb
QUERIES 	= query1.csv query2.csv query3.csv
OBJECTS 	= main.o imdbADT.o

all: $(OBJECTS)
	$(COMPILER) $(LINK_FLAGS) $(OBJECTS) -o $(BINARY)

debug: FLAGS += $(DEBUG)
debug: all

main.o: main.c imdb/imdbADT.h
	$(COMPILER) $(FLAGS) -c main.c

imdbADT.o: imdb/imdbADT.c imdb/imdbADT.h
	$(COMPILER) $(FLAGS) -c imdb/imdbADT.c

cleanAll: cleanQueries cleanBinary cleanObjects

cleanBinary:
	rm -rf $(BINARY)

cleanQueries:
	rm -rf $(QUERIES)

cleanObjects:
	rm -rf $(OBJECTS)