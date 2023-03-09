#include <stdlib.h>

#define DEBUG 1
#define OUTPUT_FILE_SIZE 30
#define DISPATCH_SIZE 12
#define ISSUE_SIZE 12
#define EXECUTE_SIZE 12

enum instruction_state {IF, ID, IS, EX, WB};

typedef struct instruction instruction;
typedef struct instruction_queue instruction_queue;
typedef struct fake_ROB fake_ROB;
typedef struct dispatch_list dispatch_list;
typedef struct issue_list issue_list;
typedef struct execute_list execute_list;

struct instruction {
    int state;
    int seq_num;
    int PC;
    int opcode;
    int dst;
    int src1;
    int src2;
    instruction *next = NULL;
};

struct instruction_queue {
    ??? *head = ??;
    ??? *tail = ??;
    int size;
};

struct fake_ROB {
    ??? *head = ??;
    ??? *tail = ??;
    int size;
};

struct dispatch_list {
    int size;
};

struct issue_list {
    int size;
};

struct execute_list {
    int size;
};

void usage();
void free_all(dispatch_list *, issue_list *, execute_list *);

void FakeRetire();
void Execute();
void Issue();
void Dispatch();
void Fetch();
int Advance_Cycle();