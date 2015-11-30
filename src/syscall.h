#ifndef SYSCALL_H
#define SYSCALL_H

#include <stdint.h>

typedef struct pcb_s 
{
	uint32_t r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, lr_svc,lr_user;
} pcb_s;

typedef struct context
{
	uint32_t r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, lr_svc, lr_user;
} context;

void sys_reboot();
void swi_handler();
void sys_nop();
void sys_settime(uint64_t date_ms);
uint64_t sys_gettime();
void sys_yieldto(pcb_s* dest);

#endif // SYSCALL_H

