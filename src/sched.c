#include "sched.h"
 
 
pcb_s* current_process;
pcb_s kmain_process;

void sched_init()
{
	current_process = &kmain_process;
}
