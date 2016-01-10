#include "src/syscall.h"
#include "src/sched.h"
#include "src/hw.h"
#include "asm_tools.h"
#include "src/util.h"
#include "src/uart.h"

#define NB_PROCESS 5
#define UART_BUFFER_SIZE 256u

//static char uart_buffer[UART_BUFFER_SIZE];

void user_process_1()
{
    uart_send_str("Start of process 1\n");
    int v1=0;
    while(v1 < 500000000)
    {
        v1++;
        //uart_send_int(v1);
        //uart_send_str("\n");
    }
    uart_send_str("End of process 1\n");
}

void user_process_2()
{
    uart_send_str("Start of process 2\n");
    int v2=0;
    while(v2 < 1000000000)
    {
        v2++;
        //uart_send_int(v2);
        //uart_send_str("\n");
    }
    uart_send_str("End of process 2\n");
}

void user_process_3()
{
    uart_send_str("Start of process 3\n");
    int v3=0;
    while(v3 < 1500000000)
    {
        v3++;
        //uart_send_int(v3);
        //uart_send_str("\n");
    }
    uart_send_str("End of process 3\n");
}

void user_process_4()
{
    uart_send_str("Start of process 4\n");
    int v4=0;
    while(v4 < 2000000000)
    {
        v4++;
        //uart_send_int(v3);
        //uart_send_str("\n");
    }
    uart_send_str("End of process 4\n");
}

void kmain(void)
{
    sched_init();
    
    uart_init();

    
    create_process((func_t*)&user_process_1, 4);
    create_process((func_t*)&user_process_2, 3);
    create_process((func_t*)&user_process_3, 2);
    create_process((func_t*)&user_process_4, 1);


    //Initialisation du timer et activation des interruptions
    timer_init();
    
    ENABLE_IRQ();
    
    //Passage mode user
    __asm("cps #0x10");
	
    while(1) {
        sys_yield();
    }
}
