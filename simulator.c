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

int totalCount = 1;

int writeback = 0;
int writebackL2= 0;

// TODO FIX TO NOT BE HARD CODED
Block matrix[64][1];
Block matrixL2[128][4];

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

    fclose(trace_file_open);
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
    /*for (int x = 0; x < 32; x++){
        printf("Set\t%d:\t%x  %c\t%x  %c\n",x,matrix[x][0].tag,matrix[x][0].dirty,matrix[x][1].tag,matrix[x][1].dirty);
    }*/
    // TODO FIX TO NOT BE HARD CODED
    for (int x = 0; x < 64; x++){
        printf("Set\t%d:\t%x  %c\n",x,matrix[x][0].tag,matrix[x][0].dirty);
    }
    printf("===== L2 contents =====\n");
    for (int x = 0; x < 128; x++){
        printf("Set\t%d:\t%x  %c\t%x  %c\t%x  %c\t%x  %c\n",x,matrixL2[x][0].tag,matrixL2[x][0].dirty,matrixL2[x][1].tag,matrixL2[x][1].dirty,matrixL2[x][2].tag,matrixL2[x][2].dirty,matrixL2[x][3].tag,matrixL2[x][3].dirty);
    }
    printf("===== Simulation results (raw) =====\n");
    printf("a. number of L1 reads:        %d\n", countRead);
    printf("b. number of L1 read misses:  %d\n", readMiss);
    printf("c. number of L1 writes:       %d\n", countWrite);
    printf("d. number of L1 write misses: %d\n", writeMiss);
    // TODO UN HARD CODE TOTAL COUNT
    printf("e. L1 miss rate:              %f\n", (float)(readMiss + writeMiss)/100000);
    printf("f. number of L1 writebacks:   %d\n", writeback);
    printf("g. number of L2 reads:        %d\n",countReadL2);
    printf("h. number of L2 read misses:  %d\n", readMissL2);
    printf("i. number of L2 writes:       %d\n",countWriteL2);
    printf("j. number of L2 write misses: %d\n", writeMissL2);
    printf("k. L2 miss rate:              0\n");
    // TODO UN HARD CODE TOTAL COUNT
    printf("l. number of L2 writebacks:   %d\n", writebackL2);
    printf("m. total memory traffic: %d\n", readMiss + writeMiss + writeback);
}

void lruFunctionL2(unsigned int tag, int index){
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
    int sets = l2_size / (l2_assoc * block_size);
    int offsetSize = log2(block_size);
    int indexSize = log2(sets);
    int sizeInBits = sizeof(offsetSize) * 8;
    unsigned int x = addr;
    x = x >> offsetSize;
    int index = x & (int)(pow(2,indexSize)-1);
    x = x >> indexSize;
    unsigned int tag = x & (int)(pow(2,32-indexSize-offsetSize)-1);
    if(operation == 'r'){
        countReadL2++;
    }
    if(operation == 'w'){
        countWriteL2++;
    }
    printf("L2 operation %c:\n", operation);
    printf("%d: L2(index: %d, ",totalCount-1, index);
    printf("tag: %x) addr %x\n", tag,addr);
    printf("tag2: %x addr %x\n", tag,addr);
    printf("matrix 1 L2: %x\n", matrixL2[index][0].tag);
    printf("matrix 2 L2: %x\n", matrixL2[index][1].tag);
    printf("matrix 3 L2: %x\n", matrixL2[index][3].tag);
    printf("matrix 4 L2: %x\n", matrixL2[index][4].tag);
    int flag = 0;
    for(int x = 0; x < l2_assoc; x++){
        if(matrixL2[index][x].tag == tag){
            flag = 1;
        }
    }
    if(flag){
        if(operation == 'r'){
            readHitL2++;
            if(replacement_policy == 0){
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
            if(replacement_policy == 0){
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
                if(replacement_policy == 1){
                    matrixL2[index][x].replacementCount = fifoCount++;
                }
                emptyPlacement = 1;
                break;
            }
        }
        if(!emptyPlacement){
            if(replacement_policy == 0){
                lruFunctionL2(tag,index);
            }
            if(replacement_policy == 1){
                //fifoFunction(tag,index);
            }
        } else{
            if(replacement_policy == 0){
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
    int sets = l1_size / (l1_assoc * block_size);
    int offsetSize = log2(block_size);
    int indexSize = log2(sets);
    int sizeInBits = sizeof(offsetSize) * 8;
    unsigned int x = addr;
    x = x >> offsetSize;
    int index = x & (int)(pow(2,indexSize)-1);
    x = x >> indexSize;
    unsigned int tag = x & (int)(pow(2,32-indexSize-offsetSize)-1);
    if(operation == 'r'){
        countRead++;
    }
    if(operation == 'w'){
        countWrite++;
    }
    printf("%d: (index: %d, ",totalCount++, index);
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
        if(operation == 'r'){
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
        if(operation == 'w'){
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
                matrix[index][x].addr = addr;
                if(replacement_policy == 1){
                    matrix[index][x].replacementCount = fifoCount++;
                }
                emptyPlacement = 1;
                break;
            }
        }
        if(!emptyPlacement){
            if(replacement_policy == 0){
                lruFunction(tag,index,addr);
            }
            if(replacement_policy == 1){
                fifoFunction(tag,index,addr);
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
        l1Cache(operation, addr);
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
