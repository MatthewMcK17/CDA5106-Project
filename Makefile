CC=gcc
TARGET=sim
DEPS=simulator.h

all: simulator.c $(DEPS)
	$(CC) simulator.c -o $(TARGET) -lm
clean:
	rm -f $(TARGET)
	
