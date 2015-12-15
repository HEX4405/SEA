#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>

typedef int (func_t) (void);

typedef enum states { RUNNING, TERMINATED } state;
//SORTIR TOUT APRES LR
typedef struct Context
{
	uint32_t r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, lr;
} Context;

typedef struct pcb_s 
{	
	Context context; //Registers LEAVE CONTEXT FIRST!!!!!!!!!
	struct pcb_s* next_process;
	uint32_t lr_user; //User link register
	func_t* entry; //Entry point
	uint32_t sp_start; //Start of pcb stack (Needed to free mem)
	uint32_t status; //Return status
	state current_state; //Current state of pcb
	uint32_t cpsr; //Status register
	uint32_t sp; //Stack pointer
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

