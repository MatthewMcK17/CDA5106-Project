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
typedef struct Vector Vector;

typedef void (*Push)(ArrayList *array_list, uint addr);
typedef void (*Delete)(ArrayList *array_list, int index);
typedef void (*Trim)(ArrayList *array_list);
typedef void (*Resize)(ArrayList *array_list);
typedef void (*Clear)(ArrayList *array_list);

struct Vector {
    ArrayList *list;
    Push push;
    Delete delete;
    Trim trim;
    Resize resize;
    Clear clear;
};
struct ArrayList {
    uint *ar;
    int size;
    int cap;
};

struct Block {
    unsigned int addr;
    unsigned int tag;
    int valid;
    char dirty;
    int replacementCount;
};

struct Address {
    uint addr;
    int index;
    uint tag;
};

void printInput();
void lru(char, uint);
void printFile(FILE *);
void usage();
void init();
void free_everything();
void init_vectors();

Address calc_addressing(uint, int);

void trim(ArrayList *);
void append(ArrayList *, uint);
void resize(ArrayList *);
uint getIndex(int);

void delete(ArrayList *, int);
void clear(ArrayList *);