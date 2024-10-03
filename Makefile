CC = gcc
CFLAGS = -Wall -pthread

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

benchmark_scenario.o: benchmark_scenario.c
	$(CC) $(CFLAGS) -c benchmark_scenario.c

benchmark_scenario: benchmark_scenario.o
	$(CC) $(CFLAGS) -o benchmark_scenario benchmark_scenario.o

dent.so: $(OBJ)
	$(CC) $(CFLAGS) -fPIC -shared -o dent.so dent.c main.c

.PHONY:test
test: test_dent
	./test_dent

.PHONY: clean
clean:
	rm -f $(OBJ) static-dent dent test_dent dent.so