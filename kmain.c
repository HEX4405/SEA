#include "src/syscall.h"
#include "src/sched.h"
#include "src/hw.h"
#include "asm_tools.h"
#include "src/util.h"

#define NB_PROCESS 5

void user_process_1()
{

}

void user_process_2()
{
    
}

void user_process_3()
{
    
}

void kmain(void)
{
    sched_init();
    
    create_process((func_t*)&user_process_1, 1);
    create_process((func_t*)&user_process_2, 2);
    create_process((func_t*)&user_process_3, 3);
    
    //Initialisation du timer et activation des interruptions
    timer_init();
    
    ENABLE_IRQ();
    
    //Passage mode user
    __asm("cps #0x10");
	
  
    while(1)
    {
        sys_yield();
    }
  
    PANIC();
}
