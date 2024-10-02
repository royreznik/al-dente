CC = gcc
CFLAGS = -Wall -pthread

OBJ = main.o dent.o

all: dent

dent: $(OBJ)
	$(CC) $(CFLAGS) -o dent $(OBJ)

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
	rm -f $(OBJ) dent test_dent