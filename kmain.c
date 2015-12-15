#include "src/syscall.h"
#include "src/sched.h"
#include "src/hw.h"
#include "asm_tools.h"
#include "src/util.h"

#define NB_PROCESS 5

void user_process_1()
{
    int v1=0;
    while(1)
    {
        v1++;
    }
}

void user_process_2()
{
    int v1=0;
    while(1)
    {
        v1++;
    }
}

void user_process_3()
{
    int v1=0;
    while(1)
    {
        v1++;
    }
}

void kmain(void)
{
    sched_init();
    
    create_process((func_t*)&user_process_1);
    create_process((func_t*)&user_process_2);
    create_process((func_t*)&user_process_3);
    
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
