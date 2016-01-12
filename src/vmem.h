#ifndef VMEM
#define VMEM

#define PAGE_SIZE 0xFFFFFFFF
#define SECON_LVL_TT_COUN 0x100 
#define SECON_LVL_TT_SIZE 0x400
#define SECON_LVL_TT_DEBUT 0x4c000
#define SECON_LVL_PAGE_SIZE 0X1000
#define FIRST_LVL_TT_COUN 0x1000
#define FIRST_LVL_TT_SIZE 0x4000
#define FIRST_LVL_TT_DEBUT 0x48000
#define KERNEL_HEAP_END 0x1000000
#define FRAME_TABLE_BASE 0x44c000
#define FRAME_TABLE_SIZE_OCT

#define ADR_TABLE_FRAME_LIBRE_SYST 0x4c000
#define SIZE_TABLE_FRAME_LIBRE_SYST 4832

#define ADR_TABLE_OCC 0x4d400
#define SIZE_TABLE_OCC 0x1FA00 

uint32_t device_flags = 
	1    << 0  | // Executable bit code
	1    << 1  | // Bit defaulting to 1
	0b01 << 2  | // 2 bits corresponding to C & B
	0b01 << 4  | // 2 bits corresponding to AP
	0b000<< 6  | // 3 bits correspondants à TEX
	0    << 9  | // Bit corresponding to APX
	0    << 10 | // Bit corresponding to S (share)
	0    << 11 ; // Bit corresponding to nG (not general)
uint32_t kernel_flags = 
	1    << 0  | // Executable bit code
	1    << 1  | // Bit defaulting to 1
	0b00 << 2  | // 2 bits corresponding to C & B
	0b01 << 4  | // 2 bits corresponding to AP
	0b001<< 6  | // 3 bits correspondants à TEX
	0    << 9  | // Bit corresponding to APX
	1    << 10 | // Bit corresponding to S (share)
	0    << 11 ; // Bit corresponding to nG (not general)

void start_mmu_C();
void configure_mmu_C();

unsigned int init_kern_translation_table();
void vmem_init();

void* sys_mmap(unsigned int size);
void sys_munmap(void* addr, unsigned int size);

