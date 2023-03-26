#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "simulator.h"

int block_size;
int l1_size;
int l1_assoc;
int l2_size;
int l2_assoc;
int l1_num_sets;
int l2_num_sets = 0;
Replacement replacement_policy;
Inclusion inclusion_property;
char *ifp;

Cache *l1 = NULL;
Cache *l2 = NULL;

uint mask = 0;

int countRead = 0;
int countWrite = 0;

int readHit = 0;
int readMiss = 0;
int writeHit = 0;
int writeMiss = 0;

int totalCount = 0;

int main(int argc, char *argv[]) {
    FILE *trace_file;
    if (argc != 9) {
        usage();
    }

    block_size = atoi(argv[1]);
    l1_size = atoi(argv[2]);
    l1_assoc = atoi(argv[3]);
    l2_size = atoi(argv[4]);
    l2_assoc = atoi(argv[5]);
    replacement_policy = atoi(argv[6]);
    inclusion_property = atoi(argv[7]);
    ifp = argv[8];
    trace_file = fopen(argv[8], "r");
    printInput();

    init(l2_size);
    mask = CALCULATE_MASK(block_size);

    printFile(trace_file);

    fclose(trace_file);
    //free_everything();
}

void usage() {
    char *usage_statement = "Usage: ./sim <BLOCKSIZE> <L1_SIZE> <L1_ASSOC> <L2_SIZE> <L2_ASSOC> <REPLACEMENT_POLICY> <INCLUSION_PROPERTY> <trace_file> \n" \
                            "   <BLOCKSIZE> - Block size in bytes. Same block size for all caches in the memory hierarchy. (positive integer)\n" \
                            "   <L1_SIZE> - L1 cache size in bytes. (positive integer) \n" \
                            "   <L1_ASSOC> - L1 set-associativity. 1 is direct-mapped. (positive integer)\n" \
                            "   <L2_SIZE> - L2 cache size in bytes.  0 signifies that there is no L2 cache. (positive integer)\n" \
                            "   <L2_ASSOC> - L2 set-associativity. 1 is direct-mapped. (positive integer)\n" \
                            "   <REPLACEMENT_POLICY> -  0 for LRU, 1 for FIFO, 2 foroptimal.(positive integer)\n" \
                            "   <INCLUSION_PROPERTY> - 0 for non-inclusive, 1 for inclusive. (positive integer)\n" \
                            "   <trace_file> - is the filename of the input trace (string)";
    printf("%s\n", usage_statement);
}

void free_set(Set *set) {
    Block *tmp = NULL;


}

void free_everything() {
    for (int i = 0; i < l1_num_sets; i++) {
        free_set(l1->cache[i]);
    }

    free(l1);

    for (int i = 0; i < l2_num_sets; i++) {
        free_set(l2->cache[i]);
    }

    free(l2);
}

void init(int l2_size) {
    l1 = malloc(sizeof(Cache));
    l1_num_sets = l1_size / (l1_assoc  * block_size);

    if (l2_size) {
        l2_num_sets = l2_size / (l2_assoc * block_size);
        l2 = malloc(sizeof(Cache));
        l2->cache = malloc(sizeof(Set *) * l2_num_sets);

        for (int i = 0; i < l2_num_sets; i++) {
            l2->cache[i] = malloc(sizeof(Set));
            l2->cache[i]->head = NULL;
            l2->cache[i]->tail = NULL;
            l2->cache[i]->size = 0;
        }
    }

    l1->cache = malloc(sizeof(Set *) * l1_num_sets);
    for (int i = 0; i < l1_num_sets; i++) {
            l1->cache[i] = malloc(sizeof(Set));
            l1->cache[i]->head = NULL;
            l1->cache[i]->tail = NULL;
            l1->cache[i]->size = 0;
    }
}

static inline char *convertReplacement(Replacement replacement_policy){
    if (replacement_policy > 2){
        char * emptyChar = "Not Valid";
        return emptyChar;
    }
    char * const replacementStrings[] = {"LRU", "FIFO", "foroptimal"};
    return replacementStrings[replacement_policy];
}

static inline char *convertInclusion(Inclusion inclusion_property){
    if (inclusion_property > 1){
        char * emptyChar = "Not Valid";
        return emptyChar;
    }
    char * const inclusionStrings[] = {"non-inclusive", "inclusive"};
    return inclusionStrings[inclusion_property];
}

int calc_index(uint addr, int lvl) {
    int offset_size = log2(block_size);
    int index_size;

    if (lvl == 1)
        index_size = log2(l1_num_sets);
    else if (lvl == 2)
        index_size = log2(l2_num_sets);

    addr >>= offset_size;
    return (addr & (int)(pow(2, index_size) - 1));
}

int calc_tag(uint addr, int lvl) {
    int offset_size = log2(block_size);
    int index_size;

    if (lvl == 1)
        index_size = log2(l1_num_sets);
    else if (lvl == 2)
        index_size = log2(l2_num_sets);

    addr >>= index_size;
    return (addr & (int)(pow(2, 32 - index_size - offset_size) - 1));
}


void printInput() {
    printf("===== Simulator configuration =====\n");
    printf("BLOCKSIZE: %d\n", block_size);
    printf("L1_SIZE: %d\n", l1_size);
    printf("L1_ASSOC: %d\n", l1_assoc);
    printf("L2_SIZE: %d\n", l2_size);
    printf("L2_ASSOC: %d\n", l2_assoc);
    printf("REPLACEMENT POLICY: %s\n", convertReplacement(replacement_policy));
    printf("INCLUSION POLICY: %s\n", convertInclusion(inclusion_property));
    printf("trace_file: %s\n", ifp);
    printf("------------------------------------\n");
}

// ONLY HANDLES L1 CACHE
void lru(char op, uint addr) {
    int index = calc_index(addr, 1), tag = calc_tag(addr, 1);
    Block *block = find_block(l1->cache[index], tag);

    if (l2 != NULL) {
        index = calc_index(addr, 2), tag = calc_tag(addr, 2);
        block = find_block(l2->cache[index], tag);
    }

    if (op == 'r') {
#if DEBUG
        printf("L1 read : %x (tag %x, index %d)\n", addr, tag, index);
#endif
        countRead++;
    } else if (op == 'w') {
#if DEBUG
        printf("L1 write : %x (tag %x, index %d)\n", addr, tag, index);
#endif
        countWrite++;
    }

    if (block != NULL) {
#if DEBUG
        printf("L1 hit\n");
#endif
        if (op == 'r')
            readHit++;
        else if (op == 'w')
            writeHit++;
        move_to_tail(l1->cache[index], block);
    } else {
#if DEBUG
        printf("L1 miss\n");
#endif
        if (op == 'r')
            readMiss++;
        else if (op == 'w')
            writeMiss++;

        if (l1->cache[index]->size == l1_assoc);
            remove_head(l1->cache[index]);
        append_block(l1->cache[index], tag);
    }
    totalCount++;
}

// ONLY HANDLES L1 CACHE
void fifo(char op, uint addr) {
    int index = calc_index(addr, 1), tag = calc_tag(addr, 1);
    Block *block = find_block(l1->cache[index], tag);

    if (l2 != NULL) {
        index = calc_index(addr, 2), tag = calc_tag(addr, 2);
        block = find_block(l2->cache[index], tag);
    }

    if (op == 'r') {
#if DEBUG
        printf("L1 read : %x (tag %x, index %d)\n", addr, tag, index);
#endif
        countRead++;
    } else if (op == 'w') {
#if DEBUG
        printf("L1 write : %x (tag %x, index %d)\n", addr, tag, index);
#endif
        countWrite++;
    }

    if (block != NULL) {
#if DEBUG
        printf("L1 hit\n");
#endif
        if (op == 'r')
            readHit++;
        else if (op == 'w')
            writeHit++;
    } else {
#if DEBUG
        printf("L1 miss\n");
#endif
        if (op == 'r')
            readMiss++;
        else if (op == 'w')
            writeMiss++;
        
        if (l1->cache[index]->size == l1_assoc)
            remove_head(l1->cache[index]);
        append_block(l1->cache[index], tag);
    }
    totalCount++;
}

void move_to_tail(Set *set, Block *block) {
    if (set->tail == block)
        return;

    // Fixes neighbor nodes
    if (block->prev != NULL)
        block->prev->next = block->next;
    if (block->next != NULL)
        block->next->prev = block->prev;
    
    // Moves node to tail
    if (set->tail != NULL) {
        set->tail->next = block;
        block->prev = set->tail;
        set->tail = block;
        set->tail->next = NULL;
    } else if (set->head == set->tail) {
        set->head = block;
        set->tail = block;
    }
}

void remove_head(Set *set) {
    Block *tmp = set->head;

    if (tmp == NULL)
        return;

    set->head = tmp->next;
    set->size--;
    free(tmp);

    if (set->head == NULL)
        return;

    set->head->prev = NULL;
}

void append_block(Set *set, uint tag) {
    Block *tmp = malloc(sizeof(Block));
    tmp->tag = tag;
    tmp->next = tmp->prev = NULL;

    if (set->head == NULL) { // This assumes if head == NULL, tail == NULL as well
        set->head = tmp;
        set->tail = tmp;
    } else {
        set->tail->next = tmp;
        tmp->prev = set->tail;
        set->tail = tmp;
    }
    set->size++;
}

Block *find_block(Set *set, uint tag) {
    Block *tmp = set->head;

    while (tmp != NULL) {
        if (tmp->tag == tag)
            return tmp;
        tmp = tmp->next;
    }
    return NULL;
}

void printFile(FILE *trace_file_open) {
    //unsigned int c;
    char op;
    uint addr;
    //char *i[10];
    //fscanf(trace_file_open, "%c %x", &operation, &i);

    while (fscanf(trace_file_open, "%c %x ", &op, &addr) != EOF)
    {
        //printf("%c %x\n",op, addr);
        printf("# %d : %s %x\n", totalCount + 1, (op == 'r') ? "read" : "write", addr);

        if (replacement_policy == FIFO)
            fifo(op, addr & mask);
        else if (replacement_policy == LRU)
            lru(op, addr & mask);
        
        printf("------------------------------------\n");
    }
    //fifo(operation, i & 0xfffffff0);
    printf("\n");
    //printList(head);
    printf("===== Simulation results (raw) =====\n");
    printf("a. number of L1 reads: %d\n", countRead);
    printf("b. number of L1 read misses: %d\n", readMiss);
    printf("c. number of L1 writes: %d\n", countWrite);
    printf("d. number of L1 write misses: %d\n", writeMiss);
}