#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "simulator.h"

int block_size;
int l1_size;
int l1_assoc;
int l2_size;
int l2_assoc;
Replacement replacement_policy;
Inclusion inclusion_property;
char *trace_file;

typedef long unsigned int mem_addr;
mem_addr addr;

struct Block {
    unsigned int addr;
    unsigned int tag;
    char dirty;
    int replacementCount;
};
typedef struct Block Block;

int countRead = 0;
int countWrite = 0;

int readHit = 0;
int readMiss = 0;
int writeHit = 0;
int writeMiss = 0;

int totalCount = 1;

int writeback = 0;
Block matrix[32][2];

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
    printFile(trace_file_open);

    //dispatch = malloc(sizeof(dispatch_list));
    //issue = malloc(sizeof(issue_list));
    //execute = malloc(sizeof(execute_list));

    fclose(trace_file_open);
    //free_all(dispatch, issue, execute);
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
    for (int x = 0; x < 32; x++){
        printf("Set\t%d:\t%x  %c\t%x  %c\n",x,matrix[x][0].tag,matrix[x][0].dirty,matrix[x][1].tag,matrix[x][1].dirty);
    }
    printf("===== Simulation results (raw) =====\n");
    printf("a. number of L1 reads: %d\n", countRead);
    printf("b. number of L1 read misses: %d\n", readMiss);
    printf("c. number of L1 writes: %d\n", countWrite);
    printf("d. number of L1 write misses: %d\n", writeMiss);
    printf("e. L1 miss rate: %f\n", (float)(readMiss + writeMiss)/100000);
    printf("f. number of L1 writebacks: %d\n", writeback);
    printf("g. number of L2 reads:        0\nh. number of L2 read misses:  0\ni. number of L2 writes:       0\nj. number of L2 write misses: 0\nk. L2 miss rate:              0\nl. number of L2 writebacks:   0\n");
    printf("m. total memory traffic: %d\n", readMiss + writeMiss + writeback);
}

void fifoFunction(unsigned int tag, int index){
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
    }
    matrix[index][smallestIndex].tag = tag;
    matrix[index][smallestIndex].replacementCount = fifoCount++;
}

void lruFunction(unsigned int tag, int index){
    int biggest = -1;
    int biggestIndex = 0;
    for(int x = 0; x < l1_assoc; x++){
        if(matrix[index][x].replacementCount > biggest){
            biggest = matrix[index][x].replacementCount;
            biggestIndex = x;
        }
    }
    if(matrix[index][biggestIndex].dirty == 'D'){
        writeback++;
    }
    matrix[index][biggestIndex].tag = tag;
    matrix[index][biggestIndex].replacementCount = 0;
    for(int x = 0; x < l1_assoc; x++){
        if(matrix[index][x].tag != tag){
            matrix[index][x].replacementCount += 1;
        }
    }
}

void l1Cache(char operation,unsigned int addr){
    int sets = l1_size / (l1_assoc * block_size);
    int offsetSize = log2(block_size);
    int indexSize = log2(sets);
    int sizeInBits = sizeof(offsetSize) * 8;
    unsigned int x = addr;
    x = x >> offsetSize;
    int index = x & (int)(pow(2,indexSize)-1);
    x = x >> indexSize;
    unsigned int tag = x & (int)(pow(2,32-indexSize-offsetSize)-1);
    char y = 'r';
    char z = 'w';
    if(operation == y){
        countRead++;
    }
    if(operation == z){
        countWrite++;
    }
    printf("%d: (index: %d, ",totalCount++, index);
    //printf("tag: %x)\n", i);
    printf("tag: %x)\n", tag);
    printf("matrix 1: %x\n", matrix[index][0].tag);
    printf("matrix 2: %x\n", matrix[index][1].tag);
    int flag = 0;
    for(int x = 0; x < l1_assoc; x++){
        if(matrix[index][x].tag == tag){
            flag = 1;
        }
    }
    if(flag){
        if(operation == y){
            readHit++;
            if(replacement_policy == 0){
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
        if(operation == z){
            writeHit++;
            for(int x = 0; x < l1_assoc; x++){
                if(matrix[index][x].tag == tag){
                    matrix[index][x].dirty = 'D';
                }
            }
            if(replacement_policy == 0){
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
                if(replacement_policy == 1){
                    matrix[index][x].replacementCount = fifoCount++;
                }
                emptyPlacement = 1;
                break;
            }
        }
        if(!emptyPlacement){
            if(replacement_policy == 0){
                lruFunction(tag,index);
            }
            if(replacement_policy == 1){
                fifoFunction(tag,index);
            }
        } else{
            if(replacement_policy == 0){
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
        if(operation == y){
            readMiss++;
            for(int x = 0; x < l1_assoc; x++){
                if(matrix[index][x].tag == tag){
                    matrix[index][x].dirty = ' ';
                }
            }
        }
        if(operation == z){
            writeMiss++;
            for(int x = 0; x < l1_assoc; x++){
                if(matrix[index][x].tag == tag){
                    matrix[index][x].dirty = 'D';
                }
            }
        }
    }
}

void printTagIndex(unsigned int i){
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
}

void printFile(FILE *trace_file_open) {
    char operation;
    unsigned int addr;
    fscanf(trace_file_open, "%c %08x ", &operation, &addr);

    while (!feof(trace_file_open))
    {
        //printf ("%c %08x\n",operation, i & (0xfffffff0));
        int sets = l1_size / (l1_assoc * block_size);
        int offsetSize = log2(block_size);
        int indexSize = log2(sets);
        int sizeInBits = sizeof(offsetSize) * 8;
        unsigned int x = addr;
        x = x >> offsetSize;
        int index = x & (int)(pow(2,indexSize)-1);
        x = x >> indexSize;
        l1Cache(operation, addr);
        //printTagIndex(i);
        fscanf(trace_file_open,"%c %x ", &operation, &addr);
    }
    l1Cache(operation, addr);
    printResults();
}

void free_all(dispatch_list *dis, issue_list *iss, execute_list *exec) {
    free(dis);
    free(iss);
    free(exec);
}