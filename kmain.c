#include "src/syscall.h"
#include "src/sched.h"
#include "src/util.h"

pcb_s pcb1, pcb2;
pcb_s *p1;
pcb_s *p2;

void user_process_1()
{
    int v1=5;
    while(1)
    {
        v1++;
        sys_yieldto(p2);
    }
}
void user_process_2()
{ 
    int v2=-12;
    while(1)
    {
        v2-=2;
        sys_yieldto(p1);
    }
}

void kmain(void)
{
    sched_init();
    pcb1.lr_svc = (uint32_t)&user_process_1;
    pcb2.lr_svc = (uint32_t)&user_process_2;
    pcb1.lr_user = (uint32_t)&user_process_1;
    pcb2.lr_user = (uint32_t)&user_process_2;
    p1 = &pcb1;
    p2 = &pcb2;
    
    
    __asm("cps #0x10");
   
  
    sys_yieldto(p1);
  
    PANIC();
}
