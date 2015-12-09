#include "syscall.h"

#define STACK_SIZE 10000

extern pcb_s* current_process;
extern pcb_s kmain_process;

void sched_init();
void create_process(func_t* entry);
void delete_process(pcb_s* process);
void start_current_process();
void elect();


