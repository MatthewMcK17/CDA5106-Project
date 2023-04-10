#include <stdlib.h>

#define DEBUG 0
#define MAX 0xFFFFFFFF
#define CALCULATE_MASK(x) (MAX >> (x >> 2) << (x >> 2));
#define DEFAULT_CAP 1024

typedef enum Replacement {LRU, FIFO, OPTIMAL} Replacement;
typedef enum Inclusion {noninclusive, inclusive} Inclusion;
typedef unsigned int uint;
typedef struct Block Block;
typedef struct ArrayList ArrayList;
typedef struct Address Address;

struct ArrayList {
    uint *ar;
    int size;
    int cap;
};

struct Block {
    unsigned int addr;
    unsigned int tag;
    char dirty;
    int replacementCount;
};

struct Address {
    int index;
    uint tag;
};

void printInput();
void fifo(char, uint);
void lru(char, uint);
void printFile(FILE *);
void usage();
void init();
void free_everything();

Address calc_addressing(uint, int);

void trim();
void append(uint);
void resize();
uint getIndex(int);

void invalidateCacheL1(unsigned int addr);
void l1Cache(char operation,unsigned int addr);
void l2Cache(char operation,unsigned int addr);