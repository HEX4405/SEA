#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>

typedef int (func_t) (void);

typedef enum state { RUNNING, TERMINATED } state;

typedef struct Context
{
	uint32_t r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, lr, sp, cpsr, lr_user;
} Context;

typedef struct pcb_s 
{	
	struct pcb_s* next_process;
	func_t* entry;
	uint32_t sp_start;
	state current_state;
	uint32_t status;
	Context context;
} pcb_s;


void sys_reboot();
void swi_handler();
void irq_handler();
void sys_nop();
void sys_settime(uint64_t date_ms);
uint64_t sys_gettime();
void sys_yield();
void sys_yieldto();
void sys_exit();
void save_context(Context* context);

#endif // SYSCALL_H

