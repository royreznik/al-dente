CC = gcc
CFLAGS = -Wall -pthread -pg

OBJ = main.o dent.o

all: dent static-dent

dent: $(OBJ)
	$(CC) $(CFLAGS) -o dent $(OBJ)

static-dent: $(OBj)
	$(CC) $(CFLAGS) -static -o static-dent $(OBJ)

main.o: main.c dent.h
	$(CC) $(CFLAGS) -c main.c

dent.o: dent.c dent.h
	$(CC) $(CFLAGS) -c dent.c

test_dent: test_dent.c dent
	$(CC) -Wall -o test_dent test_dent.c

.PHONY:test
test: test_dent
	./test_dent

.PHONY: clean
clean:
	rm -f $(OBJ) static-dent dent test_dent