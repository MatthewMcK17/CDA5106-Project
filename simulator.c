#include <stdio.h>
#include <stdlib.h>

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

struct Node {
    unsigned int data;
    struct Node* next;
    struct Node* prev;
};

struct Node *head = NULL;
struct Node *last = NULL;

int countRead = 0;
int countWrite = 0;

int readHit = 0;
int readMiss = 0;
int writeHit = 0;
int writeMiss = 0;

int totalCount = 1;

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

int length() {
   int length = 0;
   struct Node *current;
	
   for(current = head; current != NULL; current = current->next){
      length++;
   }
	
   return length;
}

void beginning_delete()  {  
    struct Node *ptr;  
    if(head == NULL)  
    {  
        printf("\n UNDERFLOW\n");  
    }  
    else if(head->next == NULL)  
    {  
        head = NULL;   
        free(head);  
        printf("\nNode Deleted\n");  
    }  
    else  
    {  
        ptr = head;  
        head = head -> next;  
        head -> prev = NULL;  
        free(ptr);  
        printf("\nNode Deleted\n");  
    }  
}

void appendLast(struct Node** head_ref, unsigned int new_data)
{
    struct Node* new_node
        = (struct Node*)malloc(sizeof(struct Node));
 
    struct Node* last = *head_ref; 
 
    new_node->data = new_data;
 
    new_node->next = NULL;
 
    if (*head_ref == NULL) {
        new_node->prev = NULL;
        *head_ref = new_node;
        return;
    }
 
    while (last->next != NULL)
        last = last->next;
    
    last->next = new_node;
 
    new_node->prev = last;
 
    return;
}

void printList(struct Node* node)
{
    struct Node* last;
    printf("\nTraversal in forward direction \n");
    while (node != NULL) {
        printf("%x ", node->data);
        last = node;
        node = node->next;
    }
}

void fifo(char operation,unsigned int i){
    //printf ("%c %d", operation, i);
    char y = 'r';
    char z = 'w';
    if(operation == y){
        countRead++;
        struct Node* last;
        struct Node* temp;
        temp = head;
        int flag = 0;
        while (temp != NULL) {
            //printf("%x ", temp->data);
            if(temp->data == i){
                flag = 1;
                break;
            }
            last = temp;
            temp = temp->next;
        }
        if(flag == 1){
            printf("Total Count: %d + Read Hit\n", totalCount);
            readHit++;
        }
        else {
            printf("Total Count: %d + Read Miss\n", totalCount);
            readMiss++;
            if(length() < 64){
                appendLast(&head, i);
            }
            else{
                beginning_delete();
                appendLast(&head, i);
            }
        }
    }
    if(operation == z){
        countWrite++;
        struct Node* last;
        struct Node* temp;
        temp = head;
        int flag = 0;
        while (temp != NULL) {
            //printf("%x ", temp->data);
            if(temp->data == i){
                flag = 1;
                break;
            }
            last = temp;
            temp = temp->next;
        }
        if(flag == 1){
            writeHit++;
            printf("Total Count: %d + Write Hit\n", totalCount);
        }
        else {
            writeMiss++;
            printf("Total Count: %d + Write Miss\n", totalCount);
            if(length() < 64){
                appendLast(&head, i);
            }
            else{
                beginning_delete();
                appendLast(&head, i);
            }
        }
    }
    totalCount++;
}

void printFile(FILE *trace_file_open) {
    //unsigned int c;
    char operation;
    unsigned int i;
    //char *i[10];
    fscanf(trace_file_open, "%c %x ", &operation, &i);

    while (!feof(trace_file_open))
    {
        //printf ("%c %x\n",operation, i & (0xfffffff0));
        fifo(operation, i & (0xfffffff0));
        fscanf(trace_file_open,"%c %x ", &operation, &i);
    }
    printf("\n");
    //printList(head);
    printf("===== Simulation results (raw) =====\n");
    printf("a. number of L1 reads: %d\n", countRead + 1);
    printf("b. number of L1 read misses: %d\n", readMiss + 1);
    printf("c. number of L1 writes: %d\n", countWrite + 1);
    printf("d. number of L1 write misses: %d\n", writeMiss + 1);
}

void FakeRetire() {

}

void Execute() {

}

void Issue() {

}

void Dispatch() {

}

void Fetch() {

}

int Advance_Cycle() {

}

void free_all(dispatch_list *dis, issue_list *iss, execute_list *exec) {
    free(dis);
    free(iss);
    free(exec);
}