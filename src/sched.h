#include "syscall.h"

#define STACK_SIZE 10000

extern pcb_s* current_process;
extern pcb_s kmain_process;

void sched_init();
void create_process(func_t* entry, uint32_t priority);
void delete_process(pcb_s* process);
void start_current_process();
void elect();
pcb_s* get_max_priority_process();
void wakeup_processes();
void delete_terminated_processes();
void update_priorities(); 

void terminate_current_process();
//for data_handler in vmem.c

