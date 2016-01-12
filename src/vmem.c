#include <stdint.h>
#include "vmem.h"
#include "sched.h"

void start_mmu_C()
{
	register unsigned int control;
	__asm("mcr p15, 0, %[zero], c1, c0, 0" : : [zero] "r"(0)); //Disable cache
	__asm("mcr p15, 0, r0, c7, c7, 0"); //Invalidate cache (data and instructions) */
	__asm("mcr p15, 0, r0, c8, c7, 0"); //Invalidate TLB entries
	
	/* Enable ARMv6 MMU features (disable sub-page AP) */
	control = (1<<23) | (1 << 15) | (1 << 4) | 1;
	
	/* Invalidate the translation lookaside buffer (TLB) */
	__asm volatile("mcr p15, 0, %[data], c8, c7, 0" : : [data] "r" (0));
	
	/* Write control register */
	__asm volatile("mcr p15, 0, %[control], c1, c0, 0" : : [control] "r" (control));
	
}


void configure_mmu_C()
{
	register unsigned int pt_addr = MMUTABLEBASE;
	total++;
	
	/* Translation table 0 */
	__asm volatile("mcr p15, 0, %[addr], c2, c0, 0" : : [addr] "r" (pt_addr));
	
	/* Translation table 1 */
	__asm volatile("mcr p15, 0, %[addr], c2, c0, 1" : : [addr] "r" (pt_addr));
	
	/* Use translation table 0 for everything */
	__asm volatile("mcr p15, 0, %[n], c2, c0, 2" : : [n] "r" (0));
	
	/* Set Domain 0 ACL to "Manager", not enforcing memory permissions
	* Every mapped section/page is in domain 0
	*/
	__asm volatile("mcr p15, 0, %[r], c3, c0, 0" : : [r] "r" (0x3));
	
}

unsigned int init_kern_translation_table() 
{

	unsigned int *tt1_base = (unsigned int* )FIRST_LVL_TT_DEBUT; //cf p18 3.2
	for(unsigned int i = 0; i < FIRST_LVL_TT_COUN; i++)
	{
		tt1_base[i] =  
		0b01   << 0  | //mark of coarse page lines
		0      << 2  | //SBZ
		1      << 3  | //NS
		0      << 4  | //SBZ
		0b0000 << 5  | //Domain
		0      << 9  | //P		
		(SECON_LVL_TT_DEBUT + i*SECON_LVL_TT_SIZE);
		
		unsigned int *tt2_base = (unsigned int* )(tt1_base[i] & 0xFFFFFC00);
		for(unsigned int j = 0; j < SECON_LVL_TT_COUN; j++)
		{
			//adresse logique = adresse physique pour toute adresse physique entre 0x0 et __kernel_heap_end__
			if((i*0x100000)<=KERNEL_HEAP_END)
			{
				tt2_base = kernel_flags | (i*SECON_LVL_TT_COUN*SECON_LVL_PAGE_SIZE + j*SECON_LVL_PAGE_SIZE);
			}
			//adresse logique = adresse physique pour toute adresse physique entre 0x20000000 et 0x20FFFFFF : I/O devices
			else if(i*0x100000>=0x20000000 | i*0x100000<=0x20FFFFFF)
			{
				tt2_base = device_flags | (i*SECON_LVL_TT_COUN*SECON_LVL_PAGE_SIZE + j*SECON_LVL_PAGE_SIZE);
				}
			//défaut de traduction
			else
			{
				tt2_base = 0x0;
			}
			
		}
	}
	return 0;
	
}

void vmem_init() 
{
	init_table_frame_systeme();
	init_table_occ();
	sys_mmap();
	start_mmu_C();
	configure_mmu_C();
	
}



//table occ des frames
void init_table_frame_systeme()
{
	char * table_frame=ADR_TABLE_FRAME_LIBRE_SYST;
	int i=0;
	//148 = 5 (taille de la table frame system) + 16 (taille la tabe d'hyperpage system) + 127 (taille de la table d'occupation) (en KO)
	for (i=0;i<148;i++){
		table_frame[i]=1;
	}
	for (i=148;i<SIZE_TABLE_FRAME_LIBRE_SYST;i++){
		table_frame[i]=0;
	}
}

void init_table_occ() {
	unsigned char* ptr=ADR_TABLE_OCC;
	int i=0;
	for (i=0;i<SIZE_TABLE_OCC;i++) {
		ptr[i]=0;
	}
}

void* sys_mmap(unsigned int size)
{
	//1. Trouver 1 page libre dans l’espace d’adressage du processus courant ;
	
	unsigned int nbPages = (unsigned int)(ceil(size/PAGE_SIZE));
	unsigned int debut_adresse;
	unsigned int nbPagesLibresContinus = 0;
	
	if (nbPages = 0) return 0;
	
	unsigned int *tt1_base = (unsigned int *)TT1_BASE;
	for (unsigned int i = KERNEL_HEAP_END + 1; nbPagesLibresContinus < nbPages && i < FIRST_LVL_TT_COUN; ++i) {
		unsigned int *tt2_base = (unsigned int *) (tt1_base[i] & 0xFFFFFC00); 

		for (unsigned int j = 0; nbPagesLibresContinus < nbPages && j < SECON_LVL_TT_COUN; ++j) {

			if (0 == (tt2_base[j] & 0b11)) { // if it is free (translation fault)

				if (0 == nbPagesLibresContinus) {
					debut_adresse = i << 20 | j << 12;
				}

				nbPagesLibresContinus++;
			} else {
				nbPagesLibresContinus = 0;
			}
		}
	}
	if (nbPagesLibresContinus < nbPages) return 0;
	
	//2. Trouver 1 frame libre dans la mémoire physique ;

	uint8_t *frame_table = (uint8_t *) FRAME_TABLE_BASE;
	unsigned int nbAllocated = 0;
	unsigned int nbFreeFrames = 0;

	for (unsigned int f = 0; f < FRAME_TABLE_SIZE_OCT; ++f) {
		if (0 == frame_table[f]) nbFreeFrames++;
	}

	//3. Insérer la traduction de l’adresse de la page vers l’adresse de la frame dans la table des pages du processus courant.

	if (nbFreeFrames < nbPages) return 0;

	unsigned int firstI = debut_adresse >> 20;
	unsigned int firstJ = (debut_adresse >> 12) & 0xFF;
	unsigned int currentI = firstI;
	unsigned int currentJ = firstJ;

	for (unsigned int f = 0; nbAllocated < nbPages; ++f) {
		unsigned int *tt2_base = (unsigned int *) (tt1_base[currentI] & 0xFFFFFC00);
		if (0 == frame_table[f]) {
			tt2_base[currentJ] = normal_flags |
					(currentI*SECON_LVL_TT_COUN*SECON_LVL_PAGE_SIZE + currentJ*SECON_LVL_PAGE_SIZE);

			frame_table[f] = 1;
			++nbAllocated;

			++currentJ;
			if (currentJ >= SECON_LVL_TT_COUN) {
				currentJ = 0;
				++currentI;
			}
		}
	}

	return (uint8_t *)debut_adresse;
}
void sys_munmap(void* addr, unsigned int size)
{
	unsigned int nbPages = (unsigned int)(ceil(size/PAGE_SIZE));
	
	unsigned int adresseVirtuelle = (unsigned int) addr;
	unsigned int tt1_index = adresseVirtuelle >> 20;
	unsigned int tt2_index = (adresseVirtuelle >> 12) & 0xFF;

	unsigned int *tt1_base = (unsigned int *)FIRST_LVL_TT_DEBUT;
	unsigned int *tt2_base = (unsigned int *) (tt1_base[tt1_index] & 0xFFFFFC00);
	unsigned int *tt2_end = tt2_base + tt2_index + nbPages;

	for (unsigned int *tt2_cell = tt2_base + tt2_index; tt2_cell < tt2_end; ++tt2_cell) {
		uint8_t *frame_table = (uint8_t *) FRAME_TABLE_BASE;
		uint8_t *frame_cell = frame_table + ((*tt2_cell & 0xFFFFF000) >> 12); 
		*frame_cell = 0;
		*tt2_cell &= 0xFFFFFFFC; 
	}
}

__attribute__ ((naked)) data_handler() {
	__asm("cps #0x13");
	unsigned int causeFaute=0;
	unsigned int adrVirtuelle=0;
	
	__asm("MRC p15, 0, r10, c5, c0, 0"); //copie dans le registre r10 du processeur le contenu de c5/DFSR (Data Fault Status Register). (voir page 40)
	__asm("MRC p15, 0, r11, c6, c0, 0"); //copie dans le registre r11 du processeur le contenu de c6/FAR (Fault Address Register). (voir page 40)
	
	__asm("mov %0,r10" : "=r"(causeFaute));
	__asm("mov %0,r11" : "=r"(adrVirtuelle));
	
	causeFaute%=0x10;
	switch (causeFaute) {
		case 0b0111 :
			terminate_current_process();
			//Translation Fault
			break;
		case 0b0110 :
			//Access fault
			terminate_current_process();
			break;
		case 0b1111 :
			//Permission Fault
			terminate_current_process();
			break;
	}
	//__asm("mov lr,%0" : : "r"(lrGlobal)); //lr = lrGlobal
}

void commute_mmu(){
	__asm("MCR p15,0,r0,c8,c6,0");
	configure_mmu_C();
	start_mmu_C();
}

void commute_to_sys()
{
	commute_mmu();
}