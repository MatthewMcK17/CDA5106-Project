CC=gcc
TARGET=sim_cache
DEPS=simulator.h

all: simulator.c $(DEPS)
	$(CC) simulator.c -o $(TARGET)
clean:
	rm -f $(TARGET)
	
