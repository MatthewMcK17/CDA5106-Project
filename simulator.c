#include <stdio.h>
#include <stdlib.h>

#include "simulator.h"

dispatch_list *dispatch;
issue_list *issue;
execute_list *execute;

int queue_size;
int fetch_dispatch_rate;

int main(int argc, char *argv[]) {
    FILE *trace_file;
    
    if (argc != 4) {
        usage();
    }

    queue_size = atoi(argv[1]);
    fetch_dispatch_rate = atoi(argv[2]);
    issue_rate = fetch_dispatch_rate + 1;

    trace_file = fopen(argv[3], "r");

    dispatch = malloc(sizeof(dispatch_list));
    issue = malloc(sizeof(issue_list));
    execute = malloc(sizeof(execute_list));

    do {
        // Remove instructions from the end of the fake_ROB
        // until an instruction is reached that is not in the WB state
        FakeRetire();

        // From the execute_list, check for instructions that are finishing
        // execution this cycle, and:
        // 1) Remove the instruction from the execute_list.
        // 2) Transition from EX state to WB state.
        // 3) Update the register file state e.g., ready flag) and wakeup
        // dependent instructions (set their operand ready flags).
        Execute();

        // From the issue_list, construct a temp list of instructions whose
        // operands are ready – these are the READY instructions.
        // Scan the READY instructions in ascending order of
        // tags and issue up to N+1 of them. To issue an instruction:
        // 1) Remove the instruction from the issue_list and add it to the
        // execute_list.
        // 2) Transition from the IS state to the EX state.
        // 3) Free up the scheduling queue entry (e.g., decrement a count
        // of the number of instructions in the scheduling queue)
        // 4) Set a timer in the instruction’s data structure that will allow
        // you to model the execution latency.
        Issue();

        // From the dispatch_list, construct a temp list of instructions in the ID
        // state (don’t include those in the IF state – you must model the
        // 1 cycle fetch latency). Scan the temp list in ascending order of
        // tags and, if the scheduling queue is not full, then:
        // 1) Remove the instruction from the dispatch_list and add it to the
        // issue_list. Reserve a schedule queue entry (e.g. increment a
        // count of the number of instructions in the scheduling
        // queue) and free a dispatch queue entry (e.g. decrement a count of
        // the number of instructions in the dispatch queue).
        // 2) Transition from the ID state to the IS state.
        // 3) Rename source operands by looking up state in the register file;
        // Rename destination by updating state in the register file.
        // For instructions in the dispatch_list that are in the IF state,
        // unconditionally transition to the ID state (models the 1 cycle
        // latency for instruction fetch).
        Dispatch();

        // Read new instructions from the trace as long as
        // 1) you have not reached the end-of-file,
        // 2) the fetch bandwidth is not exceeded, and
        // 3) the dispatch queue is not full.
        // Then, for each incoming instruction:
        // 1) Push the new instruction onto the fake-ROB. Initialize the
        // instruction’s data structure, including setting its state to IF.
        // 2) Add the instruction to the dispatch_list and reserve a
        // dispatch queue entry (e.g., increment a count of the number
        // of instructions in the dispatch queue).
        Fetch();
    } while (Advance_Cycle());

    fclose(trace_file);
    free_all(dispatch, issue, execute);
}

void usage() {
    char *usage_statement = "Usage: ./sim <S> <N> <tracefile>\n" \
                            "   <S> - is the Scheduling Queue size (integer)\n" \
                            "   <N> - is the peak fetch and dispatch rate (integer), issue rate will be up to N + 1\n" \
                            "   <tracefile> - is the filename of the input trace (string)";
    printf("%s\n", usage_statement);
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