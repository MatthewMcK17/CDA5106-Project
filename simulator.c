#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "simulator.h"

int block_size;
int l1_size;
int l1_assoc;
int l1_num_sets;
int l2_size;
int l2_assoc;
int l2_num_sets;
Replacement replacement_policy;
Inclusion inclusion_property;
char *trace_file;

/* typedef long unsigned int mem_addr;
mem_addr addr; */

int countRead = 0;
int countWrite = 0;

int countReadL2 = 0;
int countWriteL2 = 0;

int readHit = 0;
int readMiss = 0;
int writeHit = 0;
int writeMiss = 0;

int readHitL2 = 0;
int readMissL2 = 0;
int writeHitL2 = 0;
int writeMissL2 = 0;

int totalCount = 0;

int writeback = 0;
int writebackL2= 0;

// TODO FIX TO NOT BE HARD CODED
/* Block matrix[64][1];
Block matrixL2[128][4]; */
Block **matrix = NULL;
Block **matrixL2 = NULL;

ArrayList *memory_addresses = NULL;

int fifoCount = 0;

int main(int argc, char *argv[]) {
    FILE *trace_file_open;
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
    trace_file = argv[8];
    trace_file_open = fopen(argv[8], "r");
    printInput();

    init();
    printFile(trace_file_open);

    free_everything();
    fclose(trace_file_open);
}

void free_everything() {
    free(memory_addresses->ar);
    free(memory_addresses);

    for (int i = 0; i < l1_num_sets; i++)
        free(matrix[i]);
    free(matrix);

    if (matrixL2 != NULL) {
        for (int i = 0; i < l2_num_sets; i++) 
            free(matrixL2[i]);
        free(matrixL2);
    }
}

void init() {
    l1_num_sets = l1_size / (l1_assoc  * block_size);

    matrix = malloc(sizeof(Block *) * l1_num_sets);
    
    for (int i = 0; i < l1_num_sets; i++) {
        matrix[i] = calloc(l1_assoc, sizeof(Block));
    }

    if (l2_size) {
        l2_num_sets = l2_size / (l2_assoc * block_size);

        matrixL2 = malloc(sizeof(Block *) * l2_num_sets);
        for (int i = 0; i < l2_num_sets; i++) {
            matrixL2[i] = calloc(l2_assoc, sizeof(Block));
        }
    }

    memory_addresses = malloc(sizeof(ArrayList));
    memory_addresses->cap = DEFAULT_CAP;
    memory_addresses->size = 0;

    memory_addresses->ar = malloc(sizeof(uint) * memory_addresses->cap);
}

void usage() {
    char *usage_statement = "Usage: ./sim_cache <BLOCKSIZE> <L1_SIZE> <L1_ASSOC> <L2_SIZE> <L2_ASSOC> <REPLACEMENT_POLICY> <INCLUSION_PROPERTY> <trace_file> \n" \
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


void printInput() {
    printf("===== Simulator configuration =====\n");
    printf("BLOCKSIZE: %d\n", block_size);
    printf("L1_SIZE: %d\n", l1_size);
    printf("L1_ASSOC: %d\n", l1_assoc);
    printf("L2_SIZE: %d\n", l2_size);
    printf("L2_ASSOC: %d\n", l2_assoc);
    printf("REPLACEMENT POLICY: %s\n", convertReplacement(replacement_policy));
    printf("INCLUSION POLICY: %s\n", convertInclusion(inclusion_property));
    printf("trace_file: %s\n", trace_file);
}

void printResults() {
    printf("===== L1 contents =====\n");
    for (int x = 0; x < l1_num_sets; x++) {
        printf("Set    %d:", x);
        for (int y = 0; y < l1_assoc; y++) {
            printf("%x %c  ",matrix[x][y].tag,matrix[x][y].dirty);
        }
        printf("\n");
    }
    if (matrixL2 != NULL) {
        printf("===== L2 contents =====\n");
        for (int x = 0; x < l2_num_sets; x++) {
            printf("Set    %d:", x);
            for (int y = 0; y < l2_assoc; y++) {
                printf("%x %c  ", matrixL2[x][y].tag, matrixL2[x][y].dirty);
            }
            printf("\n");
        }
    }
    printf("===== Simulation results (raw) =====\n");
    printf("a. number of L1 reads:        %d\n", countRead);
    printf("b. number of L1 read misses:  %d\n", readMiss);
    printf("c. number of L1 writes:       %d\n", countWrite);
    printf("d. number of L1 write misses: %d\n", writeMiss);
    printf("e. L1 miss rate:              %f\n", (float)(readMiss + writeMiss) / (countRead + countWrite));
    printf("f. number of L1 writebacks:   %d\n", writeback);
    printf("g. number of L2 reads:        %d\n",countReadL2);
    printf("h. number of L2 read misses:  %d\n", readMissL2);
    printf("i. number of L2 writes:       %d\n",countWriteL2);
    printf("j. number of L2 write misses: %d\n", writeMissL2);

    if (matrixL2 == NULL)
        printf("k. L2 miss rate:              %d\n", 0);
    else
        printf("k. L2 miss rate:              %f\n", (float)(readMissL2) / countReadL2);
    
    printf("l. number of L2 writebacks:   %d\n", writebackL2);

    if (matrixL2 == NULL)
        printf("m. total memory traffic:      %d\n", readMiss + writeMiss + writeback);
    else  {
        int mem_count = readMissL2 + writeMissL2 + writebackL2;
        switch (inclusion_property) {
            case inclusive:
                printf("m. total memory traffic:      %d\n", mem_count + writeback);
                break;
            case noninclusive:
                printf("m. total memory traffic:      %d\n", mem_count);
                break;
            default:
                break;
        }
    }
}

void lruFunctionL2(unsigned int addr, unsigned int tag, int index){
    int biggest = -1;
    int biggestIndex = 0;
    for(int x = 0; x < l2_assoc; x++){
        if(matrixL2[index][x].replacementCount > biggest){
            biggest = matrixL2[index][x].replacementCount;
            biggestIndex = x;
        }
    }
    if(matrixL2[index][biggestIndex].dirty == 'D'){
        writebackL2++;
    }
    matrixL2[index][biggestIndex].tag = tag;
    matrixL2[index][biggestIndex].addr = addr;
    matrixL2[index][biggestIndex].replacementCount = 0;
    for(int x = 0; x < l2_assoc; x++){
        if(matrixL2[index][x].tag != tag){
            matrixL2[index][x].replacementCount += 1;
        }
    }
}

void l2Cache(char operation,unsigned int addr){

    if (matrixL2 == NULL)
        return; 

/*     int sets = l2_size / (l2_assoc * block_size);
    int offsetSize = log2(block_size);
    int indexSize = log2(sets);
    int sizeInBits = sizeof(offsetSize) * 8;
    unsigned int x = addr;
    x = x >> offsetSize;
    int index = x & (int)(pow(2,indexSize)-1);
    x = x >> indexSize;
    unsigned int tag = x & (int)(pow(2,32-indexSize-offsetSize)-1); */

    Address tmp = calc_addressing(addr, 1);
    int index = tmp.index, tag = tmp.tag;

    if(operation == 'r'){
        countReadL2++;
    }
    if(operation == 'w'){
        countWriteL2++;
    }
#if DEBUG
    printf("L2 operation %c:\n", operation);
    printf("%d: L2(index: %d, ",totalCount, index);
    printf("tag: %x) addr %x\n", tag,addr);
    printf("tag2: %x addr %x\n", tag,addr);
    for (int i = 0; i < l2_assoc; i++) {
        printf("matrix %d L2: %x\n", (i + 1), matrixL2[index][i].tag);
    }
#endif

    int flag = 0;
    for(int x = 0; x < l2_assoc; x++){
        if(matrixL2[index][x].tag == tag){
            flag = 1;
        }
    }
    if(flag){
        if(operation == 'r'){
            readHitL2++;
            if(replacement_policy == LRU){
                for(int x = 0; x < l2_assoc; x++){
                    if(matrixL2[index][x].tag == tag){
                        matrixL2[index][x].replacementCount = 0;
                    }
                    else{
                        matrixL2[index][x].replacementCount += 1;
                    }
                }
            }
        }
        if(operation == 'w'){
            writeHitL2++;
            for(int x = 0; x < l2_assoc; x++){
                if(matrixL2[index][x].tag == tag){
                    matrixL2[index][x].dirty = 'D';
                }
            }
            if(replacement_policy == LRU){
                for(int x = 0; x < l2_assoc; x++){
                    if(matrixL2[index][x].tag == tag){
                        matrixL2[index][x].replacementCount = 0;
                    }
                    else{
                        matrixL2[index][x].replacementCount += 1;
                    }
                }
            }
        }
    }
    else{
        int emptyPlacement = 0;
        for(int x = 0; x < l2_assoc; x++){
            if(matrixL2[index][x].tag == 0){
                matrixL2[index][x].tag = tag;
                matrixL2[index][x].addr = addr;
                if(replacement_policy == FIFO){
                    matrixL2[index][x].replacementCount = fifoCount++;
                }
                emptyPlacement = 1;
                break;
            }
        }
        if(!emptyPlacement){
            if(replacement_policy == LRU){
                lruFunctionL2(addr, tag,index);
            }
            if(replacement_policy == FIFO){
                //fifoFunction(tag,index);
            }
        } else{
            if(replacement_policy == LRU){
                for(int x = 0; x < l2_assoc; x++){
                    if(matrixL2[index][x].tag == tag){
                        matrixL2[index][x].replacementCount = 0;
                    }
                    else{
                        matrixL2[index][x].replacementCount += 1;
                    }
                }
            }
        }
        if(operation == 'r'){
            readMissL2++;
            for(int x = 0; x < l2_assoc; x++){
                if(matrixL2[index][x].tag == tag){
                    matrixL2[index][x].dirty = ' ';
                }
            }
        }
        if(operation == 'w'){
            writeMissL2++;
            for(int x = 0; x < l2_assoc; x++){
                if(matrixL2[index][x].tag == tag){
                    matrixL2[index][x].dirty = 'D';
                }
            }
        }
    }
}

void fifoFunction(unsigned int tag, int index,unsigned int addr){
    int smallest = 9999999;
    int smallestIndex = 0;
    for(int x = 0; x < l1_assoc; x++){
        if(matrix[index][x].replacementCount < smallest){
            smallest = matrix[index][x].replacementCount;
            smallestIndex = x;
        }
    }
    if(matrix[index][smallestIndex].dirty == 'D'){
        writeback++;
        l2Cache('w',matrix[index][smallestIndex].addr);
    }
    matrix[index][smallestIndex].tag = tag;
    matrix[index][smallestIndex].addr = addr;
    matrix[index][smallestIndex].replacementCount = fifoCount++;
}

void lruFunction(unsigned int tag, int index,unsigned int addr){
    int biggest = -1;
    int biggestIndex = 0;
    for(int x = 0; x < l1_assoc; x++){
        if(matrix[index][x].replacementCount > biggest && matrix[index][x].tag != 0){
            biggest = matrix[index][x].replacementCount;
            biggestIndex = x;
        }
    }
    if(matrix[index][biggestIndex].dirty == 'D'){
        writeback++;
        l2Cache('w',matrix[index][biggestIndex].addr);
    }
    matrix[index][biggestIndex].tag = tag;
    matrix[index][biggestIndex].addr = addr;
    matrix[index][biggestIndex].replacementCount = 0;
    for(int x = 0; x < l1_assoc; x++){
        if(matrix[index][x].tag != tag){
            matrix[index][x].replacementCount += 1;
        }
    }
}

void l1Cache(char operation,unsigned int addr){
    /* int sets = l1_size / (l1_assoc * block_size);
    int offsetSize = log2(block_size);
    int indexSize = log2(sets);
    int sizeInBits = sizeof(offsetSize) * 8;
    unsigned int x = addr;
    x = x >> offsetSize;
    int index = x & (int)(pow(2,indexSize)-1);
    x = x >> indexSize;
    unsigned int tag = x & (int)(pow(2,32-indexSize-offsetSize)-1); */
    Address tmp = calc_addressing(addr, 1);
    int index = tmp.index, tag = tmp.tag;


    if(operation == 'r'){
        countRead++;
    }
    if(operation == 'w'){
        countWrite++;
    }
#if DEBUG
    printf("%d: (index: %d, ",totalCount, index);
    printf("tag: %x)\n", tag);
    printf("matrix 1: %x\n", matrix[index][0].tag);
    printf("matrix 2: %x\n", matrix[index][1].tag);
#endif
    int flag = 0;
    for(int x = 0; x < l1_assoc; x++){
        if(matrix[index][x].tag == tag){
            flag = 1;
        }
    }
    if(flag){
        if(operation == 'r'){
            readHit++;
            if(replacement_policy == LRU){
                for(int x = 0; x < l1_assoc; x++){
                    if(matrix[index][x].tag == tag){
                        matrix[index][x].replacementCount = 0;
                    }
                    else{
                        matrix[index][x].replacementCount += 1;
                    }
                }
            }
        }
        if(operation == 'w'){
            writeHit++;
            for(int x = 0; x < l1_assoc; x++){
                if(matrix[index][x].tag == tag){
                    matrix[index][x].dirty = 'D';
                }
            }
            if(replacement_policy == LRU){
                for(int x = 0; x < l1_assoc; x++){
                    if(matrix[index][x].tag == tag){
                        matrix[index][x].replacementCount = 0;
                    }
                    else{
                        matrix[index][x].replacementCount += 1;
                    }
                }
            }
        }
    }
    else{
        int emptyPlacement = 0;
        for(int x = 0; x < l1_assoc; x++){
            if(matrix[index][x].tag == 0){
                matrix[index][x].tag = tag;
                matrix[index][x].addr = addr;
                if(replacement_policy == FIFO){
                    matrix[index][x].replacementCount = fifoCount++;
                }
                emptyPlacement = 1;
                break;
            }
        }
        if(!emptyPlacement){
            if(replacement_policy == LRU){
                lruFunction(tag,index,addr);
            }
            if(replacement_policy == FIFO){
                fifoFunction(tag,index,addr);
            }
        } else{
            if(replacement_policy == LRU){
                for(int x = 0; x < l1_assoc; x++){
                    if(matrix[index][x].tag == tag){
                        matrix[index][x].replacementCount = 0;
                    }
                    else{
                        matrix[index][x].replacementCount += 1;
                    }
                }
            }
        }
        if(operation == 'r'){
            readMiss++;
            for(int x = 0; x < l1_assoc; x++){
                if(matrix[index][x].tag == tag){
                    matrix[index][x].dirty = ' ';
                    l2Cache('r',addr);
                }
            }
        }
        if(operation == 'w'){
            writeMiss++;
            for(int x = 0; x < l1_assoc; x++){
                if(matrix[index][x].tag == tag){
                    matrix[index][x].dirty = 'D';
                    l2Cache('r',addr);
                }
            }
        }
    }
}

/* void printTagIndex(unsigned int i){
    int sets = l1_size / (l1_assoc * block_size);
    int offsetSize = log2(block_size);
    int indexSize = log2(sets);
    int sizeInBits = sizeof(offsetSize) * 8;
    i = i >> offsetSize;
    printf("(index: %d, ", i & (int)(pow(2,indexSize)-1));
    i = i >> indexSize;
    printf("tag: %x)\n", i & (int)(pow(2,32-indexSize-offsetSize)-1));
}

int returnTagIndex(unsigned int i){
    int sets = l1_size / (l1_assoc * block_size);
    int offsetSize = log2(block_size);
    int indexSize = log2(sets);
    int sizeInBits = sizeof(offsetSize) * 8;
    i = i >> offsetSize;
    return i & (int)(pow(2,indexSize)-1);
} */

Address calc_addressing(uint addr, int lvl) {
    Address tmp;
    int offset_size = log2(block_size), index_size = 0;
    uint tag, index;

    if (lvl == 1)
        index_size = log2(l1_num_sets);
    else if (lvl == 2)
        index_size = log2(l2_num_sets);
    addr >>= offset_size;
    tmp.index = (addr & ((1 << index_size) - 1));
    addr >>= index_size;
    tmp.tag = (addr & ((1 << (32 - index_size - offset_size)) - 1));

    return tmp;
}

void resize() {
    memory_addresses->ar = realloc(memory_addresses->ar, sizeof(uint) * (memory_addresses->cap << 1));
    
    if (memory_addresses->ar == NULL) {
        printf("Allocation Error in ArrayList resizing\n");
        exit(1);
    }

    memory_addresses->cap <<= 1;
}

void append(uint addr) {
    if (memory_addresses->size == memory_addresses->cap)
        resize();
    memory_addresses->ar[memory_addresses->size++] = addr;
}

void trim() {
    memory_addresses->ar = realloc(memory_addresses->ar, sizeof(uint) * memory_addresses->size);

    if (memory_addresses->ar == NULL) {
        printf("Allocation Error in trimming\n");
        exit(1);
    }

    memory_addresses->cap = memory_addresses->size;
}

void printFile(FILE *trace_file_open) {
    char operation;
    unsigned int addr;

    while (fscanf(trace_file_open, "%c %08x ", &operation, &addr) != EOF) {
        append(addr);
    }
    fseek(trace_file_open, 0, SEEK_SET);
    trim();

    fscanf(trace_file_open, "%c %08x ", &operation, &addr);

    while (!feof(trace_file_open))
    {
        //printf ("%c %08x\n",operation, i & (0xfffffff0));
        totalCount++;
        l1Cache(operation, addr);
        fscanf(trace_file_open,"%c %x ", &operation, &addr);
    }
    l1Cache(operation, addr);
    printResults();
}