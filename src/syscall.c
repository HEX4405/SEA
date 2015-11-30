#include "util.h"
#include "syscall.h"
#include "hw.h"
#include "sched.h"

void sys_reboot()
{
	__asm("mov r0, #1");
	__asm("swi #0");
}

void sys_nop()
{
	__asm("mov r0, #2");
	__asm("swi #0");
}

void sys_settime(uint64_t date_ms)
{
	uint32_t date_lowbits = date_ms & (0x00000000FFFFFFFF);
	uint32_t date_highbits = date_ms >> 32;
	__asm("mov r1, %0" : : "r"(date_lowbits));
	__asm("mov r2, %0" : : "r"(date_highbits));
	__asm("mov r0, #3");
	__asm("swi #0");
}

uint64_t sys_gettime()
{
	uint32_t date_lowbits, date_highbits;
	__asm("mov r0, #4");
	__asm("swi #0");
	__asm("mov %0, r0" : "=r"(date_lowbits));
	__asm("mov %0, r1" : "=r"(date_highbits));
	uint64_t date_ms = ((uint64_t)date_highbits << 32) | ((uint64_t)date_lowbits);
	return date_ms;
}

void sys_yieldto(pcb_s* dest) 
{
	__asm("mov r1, %0" : : "r"(dest));
	__asm("mov r0, #5");
	__asm("swi #0");
}

void do_sys_reboot()
{
	__asm("mov pc, #0x8000");
}

void do_sys_nop()
{
	
}

void do_sys_settime(context* current_context)
{
	//On reconstruit l'entier 64 bits
	uint64_t date_ms =  ((uint64_t)current_context->r2 << 32) | ((uint64_t)current_context->r1);
	
	set_date_ms(date_ms);
	
}

void do_sys_gettime(context* current_context)
{
	//On récupère la date
	uint64_t date_ms = get_date_ms();
	
	//On coupe le 64 bits en deux 32 bits
	uint32_t date_lowbits = date_ms & (0x00000000FFFFFFFF);
	uint32_t date_highbits = date_ms >> 32;
	
	//On met le contexte à jour 	
	current_context->r0 = date_lowbits;
	current_context->r1 = date_highbits;
}

void do_sys_yieldto(context* current_context) 
{
	pcb_s* dest;
	dest = (pcb_s*)current_context->r1;
	if( current_process->lr_user > 0 )
	{
		dest->lr_user = current_process->lr_user;
	}
	
	current_process = (pcb_s*)current_context;
	current_context->r1 = dest->r1;
	current_context->r2 = dest->r2;
	current_context->r3 = dest->r3;
	current_context->r4 = dest->r4;
	current_context->r5 = dest->r5;
	current_context->r6 = dest->r6;
	current_context->r7 = dest->r7;
	current_context->r8 = dest->r8;
	current_context->r9 = dest->r9;
	current_context->r10 = dest->r10;
	current_context->r11 = dest->r11;
	current_context->r12 = dest->r12;
	current_context->lr_svc = dest->lr_user;
}

void __attribute__((naked)) swi_handler()
{
	//On empile tous les registres et lr
	__asm("stmfd sp!, {r0-r12,lr}");
	//On fait pointer current_context vers le sp
	context* current_context;
	__asm("mov %0, sp" : "=r"(current_context));
	
	//On passe en mode system pour récupérer le lr_user
	__asm("cps #31");
	__asm("mov %0, r14" : "=r"(current_context->lr_user));
	__asm("cps #19");
	
	//On récupère le numéro de l'appel système
	int sysCallNumber;
	__asm("mov %0, r0" : "=r"(sysCallNumber));
	switch(sysCallNumber)
	{
		case 1 : 
		do_sys_reboot();
		break;
		
		case 2 :
		do_sys_nop();
		break;
	
		case 3 :
		do_sys_settime(current_context);
		break;
		
		case 4 :
		do_sys_gettime(current_context);
		break;
		
		case 5 :
		do_sys_yieldto(current_context);
		break;
		
		default :
		PANIC();
		break;
	}
	__asm("ldmfd sp!, {r0-r12,pc}^");
}
