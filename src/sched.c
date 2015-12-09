#include "sched.h"
#include "kheap.h"
#include "hw.h"
 
pcb_s* current_process;
pcb_s* first_process;
pcb_s* last_process;
pcb_s kmain_process;

void sched_init()
{
	kheap_init();
	kmain_process.next_process = &kmain_process;
	current_process = &kmain_process;
	last_process = current_process;
}

void create_process(func_t* entry)
{
	pcb_s* process = (pcb_s*)kAlloc(sizeof(pcb_s));
	process->entry = entry;
	process->context.lr = (uint32_t)&start_current_process;
	process->sp_start = (uint32_t)kAlloc(STACK_SIZE);
	process->context.sp = process->sp_start + STACK_SIZE;
	
	if(first_process == 0)
	{
		first_process = process;
	}

    last_process->next_process = process;
    last_process = process;
    last_process->next_process = first_process;
    
    process->current_state = RUNNING;
}

void start_current_process()
{ 
	current_process->entry();
	sys_exit(0);
}

void delete_process(pcb_s* process)
{
	kFree((uint8_t*)process->sp_start, STACK_SIZE);
	kFree((uint8_t*)process,sizeof(pcb_s));
}
void elect()
{
	while( current_process->next_process->current_state == TERMINATED )
	{
		pcb_s* next_process = current_process->next_process;
		current_process->next_process = next_process->next_process;
		delete_process(next_process);
	}
	if( current_process != current_process->next_process ) 
	{
		current_process = current_process->next_process;
	}
	else
	{
		terminate_kernel();
	}
	

}
