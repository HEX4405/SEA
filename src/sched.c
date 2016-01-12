#include "sched.h"
#include "kheap.h"
#include "hw.h"
 
pcb_s* current_process;
pcb_s* first_process;
pcb_s* last_process;
pcb_s kmain_process;
int pcb_count;

void sched_init()
{
	kheap_init();
	pcb_count = 0;
	kmain_process.next_process = &kmain_process;
	current_process = &kmain_process;
	last_process = current_process;
}

//Allocate memory to the pcb and it's stack
void create_process(func_t* entry, uint32_t priority)
{
	int size = sizeof(pcb_s);
	pcb_s* process = (pcb_s*)gmalloc(size);
	process->id = ++pcb_count;
	process->entry = entry;
	process->lr_user = (uint32_t)&start_current_process;
	process->context.lr = process->lr_user;
	process->sp_start = (uint32_t)gmalloc(STACK_SIZE);
	process->sp = process->sp_start + STACK_SIZE;
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

//pcb "wrap", to make it exit without having the user do it
void start_current_process()
{
	current_process->current_state = RUNNING; 
	current_process->entry();
	sys_exit(0);
}

//Free mem used by the pcb and it's stack
void delete_process(pcb_s* process)
{
	//gfree((uint8_t*)process->sp_start);
	//gfree((uint8_t*)process);
	kFree((uint8_t*)process->sp_start, STACK_SIZE);
	kFree((uint8_t*)process,sizeof(pcb_s));
}
void elect()
{

	//We don't want our main process in the process list
	if( current_process == &kmain_process )
	{
		current_process = current_process->next_process;
	}

	//Wake up sleeping processes
	wakeup_processes();

	//Put the running process to sleep
	if( current_process->current_state == RUNNING)
	{
		current_process->current_state = WAITING;
	}
	
	delete_terminated_processes();
	
	//If process isn't alone, elect new one
	if( current_process != current_process->next_process ) 
	{
		current_process = get_max_priority_process();
		current_process->current_state = RUNNING;
		//update_priorities();
	}
	else if( current_process->current_state == TERMINATED )
	{
		delete_process(current_process);
		terminate_kernel();
		sys_reboot();
	}
	
	current_process->current_state = RUNNING;

}
// Returns the process with the highest priority (SJF)
pcb_s* get_max_priority_process()
{
	pcb_s* elected_process = current_process->next_process;
	pcb_s* iterator_process = current_process;

	do
	{
		if( iterator_process->current_state == READY && iterator_process->priority > elected_process->priority )
		{
			elected_process = iterator_process;
		}
		iterator_process = iterator_process->next_process;
	} while( iterator_process != current_process );

	return elected_process;
}

//Wake up sleeping processes
void wakeup_processes()
{
	pcb_s* tmp_process = current_process;
	do
	{
		if( tmp_process->current_state == WAITING )
		{
			tmp_process->current_state = READY;
		}
		tmp_process = tmp_process->next_process;
	} while(tmp_process != current_process);
}

////Deletes terminated processes
void delete_terminated_processes()
{
	while( current_process->next_process->current_state == TERMINATED && current_process != current_process->next_process)
	{
		pcb_s* next_process = current_process->next_process;
		current_process->next_process = next_process->next_process;
		delete_process(next_process);
	}
}
//Updates priorities in case of dynamic priority scheduling
void update_priorities()
{
	pcb_s* tmp_process = current_process;
	do
	{
		if( tmp_process->current_state == READY )
		{
			tmp_process->priority++;
		}
		tmp_process = tmp_process->next_process;
	} while(tmp_process != current_process);
}