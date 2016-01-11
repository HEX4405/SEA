#include "src/syscall.h"
#include "src/sched.h"
#include "src/hw.h"
#include "asm_tools.h"
#include "src/util.h"
#include "src/kheap.h"
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
    while(v3 < 1000000000)
    {
        v3++;
        //uart_send_int(v3);
        //uart_send_str("\n");
    }
    uart_send_str("End of process 3\n");
}

void kmain(void)
{
    sched_init();
    int *a = (int*)gmalloc(sizeof(int));
    int *b = (int*)gmalloc(sizeof(int));
    int *c = (int*)gmalloc(sizeof(int));
    int *d = (int*)gmalloc(sizeof(int));
    int *e = (int*)gmalloc(sizeof(int));
    int *f = (int*)gmalloc(sizeof(int));
    int *g = (int*)gmalloc(sizeof(int));
    int *h = (int*)gmalloc(sizeof(int));
    int *i = (int*)gmalloc(sizeof(int));

    (*a) = 1;
    (*b) = 2;
    (*c) = 3;
    (*d) = 4;
    (*e) = 5;
    (*f) = 6;   
    (*g) = 7;
    (*h) = 8;
    (*i) = 9;

    gfree(a);
    gfree(b);

    int *x = (int*)gmalloc(sizeof(int));
    (*x) = 1;
}
