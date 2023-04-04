CC=gcc
TARGET=sim_cache
DEPS=simulator.h

all: simulator.c $(DEPS)
	$(CC) simulator.c -o $(TARGET) -lm
clean:
	rm -f $(TARGET)
	
