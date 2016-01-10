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

void create_process(func_t* entry, uint32_t priority)
{
	int size = sizeof(pcb_s);
	pcb_s* process = (pcb_s*)kAlloc(size);
	process->id = ++pcb_count;
	process->entry = entry;
	process->lr_user = (uint32_t)&start_current_process;
	process->context.lr = process->lr_user;
	process->sp_start = (uint32_t)kAlloc(STACK_SIZE);
	process->sp = process->sp_start + STACK_SIZE;


	//Code permettant de mesurer le temps d'éxecution du process afin de lui donner une priorité
	/*uint64_t start = get_date_ms();
	process->entry();
	uint64_t end = get_date_ms();

	uint64_t diffTime = start - end;*/



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

	//We don't want our main process in the process list
	if( current_process == &kmain_process )
	{
		current_process = current_process->next_process;
	}

	//Wake up sleeping processes
	wakeup_processes();

	//If the process isn't terminated, make it sleep
	if( current_process->current_state == RUNNING)
	{
		current_process->current_state = WAITING;
	}
	
	
	delete_terminated_processes();
	
	if( current_process != current_process->next_process ) 
	{
		current_process = get_max_priority_process();
		current_process->current_state = RUNNING;
	}
	else if( current_process->current_state == TERMINATED )
	{
		delete_process(current_process);
		terminate_kernel();
		sys_reboot();
	}
	
	update_priorities();

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