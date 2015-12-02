#include "sched.h"
#include "kheap.h"
 
pcb_s* current_process;
pcb_s kmain_process;

void sched_init()
{
	kheap_init();
	current_process = &kmain_process;
}

void create_process(func_t* entry)
{
	pcb_s* process = (pcb_s*)kAlloc(sizeof(pcb_s));
	process->context.lr_user = (uint32_t)entry;
	process->context.sp_user = (uint32_t)kAlloc(10000) + 10000;
}

