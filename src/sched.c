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

void create_process(func_t* entry, uint32_t priority)
{
	pcb_s* process = (pcb_s*)kAlloc(sizeof(pcb_s));
	process->entry = entry;
	process->lr_user = (uint32_t)&start_current_process;
	process->context.lr = process->lr_user;
	process->sp_start = (uint32_t)kAlloc(STACK_SIZE);
	process->sp = process->sp_start + STACK_SIZE;

	//Function to define priority
	process->priority = priority;
	
	if(first_process == 0)
	{
		first_process = process;
	}

    last_process->next_process = process;
    last_process = process;
    last_process->next_process = first_process;
    
    process->current_state = READY;
}

void start_current_process()
{
	current_process->current_state = RUNNING; 
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
	while( current_process->next_process->current_state == TERMINATED && current_process != current_process->next_process)
	{
		pcb_s* next_process = current_process->next_process;
		current_process->next_process = next_process->next_process;
		delete_process(next_process);
	}
	if( current_process != current_process->next_process ) 
	{
		if( current_process->current_state != TERMINATED)
		{
			current_process->current_state = READY;
		}

		current_process = get_max_priority_process();
		current_process->current_state = RUNNING;
	}
	else if( current_process->current_state == TERMINATED )
	{
		delete_process(current_process);
		terminate_kernel();
	}
	

}

pcb_s* get_max_priority_process()
{
	if( current_process == &kmain_process )
	{
		current_process = current_process->next_process;
	}
	pcb_s* elected_process = current_process->next_process;
	pcb_s* iterator_process = current_process->next_process;
	while( iterator_process->next_process != current_process )
	{
		if( iterator_process->next_process->current_state == READY && iterator_process->next_process->priority > elected_process->priority )
		{
			elected_process = iterator_process->next_process;
		}
		iterator_process = iterator_process->next_process;
	}

	return elected_process;
}
