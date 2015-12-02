#include "syscall.h"

typedef int (func_t) (void);

extern pcb_s* current_process;
extern pcb_s kmain_process;

void sched_init();
pcb_s* create_process(func_t* entry);


