CC = gcc -Wall -ansi -pedantic -I../f123inc

PGMNAME= ../f123app/samprg3b

SOURCE= ../f123app/samprg3b.c

OBJECTS= ../f123obj/samprg3b.o

LIB=../f123lib/fips123.a

FLAGS=

OTHERS=

all: setup $(PGMNAME)

setup:

$(OBJECTS): $(SOURCE)
	cd ../f123obj; $(CC)  -c $(SOURCE)

$(PGMNAME): $(OBJECTS)
	$(CC) $(OBJECTS) $(LIB) -o $(PGMNAME)
 
