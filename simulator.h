#include <stdlib.h>

#define DEBUG 1
#define MAX 0xFFFFFFFF
#define CALCULATE_MASK(x) (MAX >> (x >> 2) << (x >> 2));

typedef enum Replacement {LRU, FIFO, foroptimal} Replacement;
typedef enum Inclusion {noninclusive, inclusive} Inclusion;
typedef unsigned int uint;
typedef struct Block Block;
typedef struct Cache Cache;
typedef struct Set Set;

struct Block {
    Block *next;
    Block *prev;
    uint tag;
    int dirty;
};

struct Set {
    Block *head;
    Block *tail;
    int size;
};

struct Cache {
    Set **cache;
};

void printInput();
void fifo(char, uint);
void lru(char, uint);
void printFile(FILE *trace_file_open);
void usage();
void init();
Block *find_block(Set *, uint);
void append_block(Set *, uint);
void move_to_tail(Set *,Block *);
void remove_head(Set *);
void free_everything();
