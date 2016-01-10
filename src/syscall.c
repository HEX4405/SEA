#include "util.h"
#include "syscall.h"
#include "hw.h"
#include "sched.h"
#include "asm_tools.h"

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
	__asm("mov %0, r1" : "=r"(date_lowbits));
	__asm("mov %0, r2" : "=r"(date_highbits));
	uint64_t date_ms = ((uint64_t)date_highbits << 32) | ((uint64_t)date_lowbits);
	return date_ms;
}


void sys_yieldto(pcb_s* dest)
{
	__asm("mov r1, %0" : : "r"(dest));
	__asm("mov r0, #5");
	__asm("swi #0");
	__asm("mov sp, %0" : : "r"(current_process->sp));
	__asm("mov lr, %0" : : "r"(current_process->lr_user));
	__asm("msr cpsr, %0" : : "r"(current_process->cpsr));
}

void sys_yield()
{
	__asm("mov r0, #6");
	__asm("swi #0");
}

void sys_exit(int status)
{
	__asm("mov r1, %0" : : "r"(status));
	__asm("mov r0, #7");
	__asm("swi #0");
}

void do_sys_reboot()
{
	__asm("mov pc, #0x8000");
}

void do_sys_nop()
{

}

void do_sys_settime(Context* current_context)
{
	//On reconstruit l'entier 64 bits
	uint64_t date_ms =  ((uint64_t)current_context->r2 << 32) | ((uint64_t)current_context->r1);

	set_date_ms(date_ms);

}

void do_sys_gettime(Context* current_context)
{
	//On récupère la date
	uint64_t date_ms = get_date_ms();

	//On coupe le 64 bits en deux 32 bits
	uint32_t date_lowbits = date_ms & (0x00000000FFFFFFFF);
	uint32_t date_highbits = date_ms >> 32;

	//On met le contexte à jour
	current_context->r1 = date_lowbits;
	current_context->r2 = date_highbits;
}

void do_sys_yieldto(Context* current_context)
{
	pcb_s* dest = (pcb_s*)current_context->r1;
	current_process = dest;
}

void do_sys_yield()
{
	elect();
	uart_send_str("Elected process : ");
	uart_send_int(current_process->id);
	uart_send_str("\n");
}

void do_sys_exit(Context* current_context)
{
	current_process->status = current_context->r1;
	current_process->current_state = TERMINATED;
}

void __attribute__((naked)) swi_handler()
{
	//On empile tous les registres et lr
	__asm("stmfd sp!, {r0-r12,lr}");
	//On fait pointer current_context vers le sp
	Context* current_context;
	__asm("mov %0, sp" : "=r"(current_context));
	save_context(current_context);
	__asm("cps #31");
	__asm("mov %0, sp" : "=r"(current_process->sp));
	__asm("mov %0, lr" : "=r"(current_process->lr_user));
	__asm("cps #19");

	//On récupère le spsr qui correspond au cpsr_user
	__asm("mrs %0, spsr" : "=r"(current_process->cpsr));
	//On récupère le numéro de l'appel système
	int sysCallNumber = current_context->r0;
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

		case 6 :
		do_sys_yield();
		break;

		case 7 :
		do_sys_exit(current_context);
		do_sys_yield();
		break;

		default :
		PANIC();
		break;
	}
	__asm("mrs %0, spsr" : "=r"(current_process->cpsr));
	
	__asm("cps #31");
	__asm("mov sp, %0" : : "r"(current_process->sp));
	__asm("mov lr, %0" : : "r"(current_process->lr_user));
	__asm("cps #19");
	
	restore_context(current_context);
	
	__asm("ldmfd sp!, {r0-r12,pc}^");
}

void __attribute__((naked)) irq_handler()
{
	__asm("stmfd sp!, {r0-r12,lr}");
	Context* current_context;
	__asm("mov %0, sp" : "=r"(current_context));
	//On décale le LR pour retourner au bon endroit
	current_context->lr -= 4;
	//Sauvegarde de contexte
	save_context(current_context);
	__asm("mrs %0, spsr" : "=r"(current_process->cpsr));
	__asm("cps #31");
	__asm("mov %0, sp" : "=r"(current_process->sp));
	__asm("mov %0, lr" : "=r"(current_process->lr_user));
	__asm("cps #18");

	//CHANGEMENT PROCESSUS
	do_sys_yield();

	//Restauration de contexte
	__asm("mrs %0, spsr" : "=r"(current_process->cpsr));
	__asm("cps #31");
	__asm("mov sp, %0" : : "r"(current_process->sp));
	__asm("mov lr, %0" : : "r"(current_process->lr_user));
	__asm("cps #18");

	set_next_tick_default();
	ENABLE_TIMER_IRQ();
	
	restore_context(current_context);
	
	__asm("ldmfd sp!, {r0-r12,pc}^");
}


void save_context(Context* current_context)
{
	current_process->context.r0 = current_context->r0;
	current_process->context.r1 = current_context->r1;
	current_process->context.r2 = current_context->r2;
	current_process->context.r3 = current_context->r3;
	current_process->context.r4 = current_context->r4;
	current_process->context.r5 = current_context->r5;
	current_process->context.r6 = current_context->r6;
	current_process->context.r7 = current_context->r7;
	current_process->context.r8 = current_context->r8;
	current_process->context.r9 = current_context->r9;
	current_process->context.r10 = current_context->r10;
	current_process->context.r11 = current_context->r11;
	current_process->context.r12 = current_context->r12;
	current_process->context.lr = current_context->lr;
}

void restore_context(Context* current_context)
{
	current_context->r0 = current_process->context.r0;
	current_context->r1 = current_process->context.r1;
	current_context->r2 = current_process->context.r2;
	current_context->r3 = current_process->context.r3;
	current_context->r4 = current_process->context.r4;
	current_context->r5 = current_process->context.r5;
	current_context->r6 = current_process->context.r6;
	current_context->r7 = current_process->context.r7;
	current_context->r8 = current_process->context.r8;
	current_context->r9 = current_process->context.r9;
	current_context->r10 = current_process->context.r10;
	current_context->r11 = current_process->context.r11;
	current_context->r12 = current_process->context.r12;
	current_context->lr = current_process->context.lr;
}
