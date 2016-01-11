#include <stdint.h>
#include "kheap.h"


struct fl {
	struct fl		*next;
	unsigned int	 size;
	void 			*bloc;
} *freelist = (struct fl *) 0;



struct ol {
	struct ol		*next;
	unsigned int 	 size;
	void 			*bloc;
} *occupiedlist = (struct ol *)0;

uint8_t* kernel_heap_top;
uint8_t* kernel_heap_limit;

unsigned int
aligned_value(unsigned int addr, unsigned int pwr_of_2)
{    
    unsigned int modulo = (1 << pwr_of_2);
    unsigned int max_value_modulo = modulo - 1;
    return (addr + max_value_modulo) & ~max_value_modulo;
}

uint8_t*
kAlloc_aligned(unsigned int size, unsigned int pwr_of_2)
{
	register struct fl *cfl = freelist, **prev;
	unsigned int aligned_cfl = aligned_value((unsigned int) cfl, pwr_of_2);
	unsigned int size_aligned = aligned_value(size, 2);

	prev = &freelist;
	while (cfl && (aligned_cfl != (unsigned int) cfl || cfl->size != size_aligned))
	{
	    prev = &(cfl->next);
	    cfl = cfl->next;
	    aligned_cfl = aligned_value((unsigned int) cfl, pwr_of_2);
	}

	if (! cfl)
	{
	    cfl = (struct fl *) kernel_heap_top;
	    aligned_cfl = aligned_value((unsigned int) cfl, pwr_of_2);

	    if (aligned_cfl == (unsigned int) cfl)
	    {
		kernel_heap_top += size_aligned;
	    }
	    else
	    {
		kFree((uint8_t*) cfl, (aligned_cfl - (unsigned int) cfl));
		kernel_heap_top = (uint8_t*) (aligned_cfl + size_aligned);
	    }

	    /* No space available anymore */
	    if (kernel_heap_top > kernel_heap_limit)
		return FORBIDDEN_ADDRESS;
	}
	else
	{
	    if (aligned_cfl == (unsigned int) cfl)
	    {
		*prev = cfl->next;
	    }
	    else
	    {
		kFree((uint8_t*) cfl, cfl->size - size_aligned);
	    }
	}

	/* Fill with FORBIDDEN_BYTE to debug (more) easily */
	for (int i = 0 ; i < size_aligned ; i++) {
	    *(((uint8_t*) aligned_cfl) + i) = FORBIDDEN_BYTE;
	}
	
	return ((uint8_t *) aligned_cfl);
}

uint8_t*
kAlloc(unsigned int size)
{
	register struct fl *cfl = freelist, **prev;
	unsigned int size_aligned = (size + 3) & ~3;

	prev = &freelist;

	while (cfl && cfl->size != size_aligned)
	{
	    prev = &cfl->next;
	    cfl = cfl->next;
	}

	if (! cfl)
	{
	    cfl = (struct fl *) kernel_heap_top;
	    kernel_heap_top += size_aligned;

	    /* No space available anymore */
	    if (kernel_heap_top > kernel_heap_limit)
		return FORBIDDEN_ADDRESS;
	}
	else
	{
	    *prev = cfl->next;
	}

	/* Fill with FORBIDDEN_BYTE to debug (more) easily */
	for (int i = 0 ; i < size_aligned ; i++) {
	    *((uint8_t*) cfl) = FORBIDDEN_BYTE;
	}

	return ((uint8_t *) cfl);
}

void
kFree(uint8_t* ptr, unsigned int size)
{
	register struct fl* cfl = (struct fl*) ptr;

	cfl->size = (size + 3) & ~3;
	cfl->next = freelist;
	freelist = cfl;
}

void *gmalloc(unsigned int size)
{
	register struct fl *cfl = freelist, **prev;
	unsigned int size_aligned = (size + 3) & ~3;

	prev = &freelist;



	
	while(cfl && (cfl->size + sizeof(struct fl)) < (size_aligned  + sizeof(struct ol)))
	{
		prev = &cfl->next;
		cfl = cfl->next;
	}
	if(!cfl)
	{
		return FORBIDDEN_ADDRESS;
	}
	else
	{
		if(cfl->size + sizeof(struct fl) > size_aligned + sizeof(struct ol))
		{
			uint8_t *temp = (uint8_t*)cfl + size_aligned + sizeof(struct ol);
			struct fl *splitBloc = (struct fl *)(temp);
			splitBloc->size = cfl->size - sizeof(struct ol) - size_aligned;
			uint8_t *temp2 = (uint8_t*)cfl + size_aligned + sizeof(struct ol) + sizeof(struct fl);
			splitBloc->bloc = temp2;
			splitBloc->next = cfl->next;
			(*prev) = splitBloc;
		}
		struct ol *fullBloc = (struct ol *)cfl;
		fullBloc->size = size_aligned;
		uint8_t *temp3 = (uint8_t*)cfl + sizeof(struct ol);
		fullBloc->bloc = temp3;
		if(!occupiedlist)
		{
			occupiedlist = fullBloc;
		}
		else
		{
			register struct ol* col = occupiedlist, **prevo;
			prevo = &occupiedlist;
			while(col && fullBloc > col)
			{
				prevo = &col->next;
				col = col->next;
			}
			fullBloc->next = col;
			(*prevo) = fullBloc;
		}
		return fullBloc->bloc;
		
	}
	return FORBIDDEN_ADDRESS;
}

void gfree(void * ptr)
{
	uint8_t* spaceToClean = (uint8_t *) ptr - sizeof(struct ol);
	struct ol * col = (struct ol *)spaceToClean;
	struct fl *emptyBloc = (struct fl* ) spaceToClean;
	emptyBloc->size = col->size;
	emptyBloc->bloc = col->bloc;

	struct ol **prev;
	prev = (struct ol**)&spaceToClean;
	(*prev) = col->next;

	struct fl * cfl = freelist, **prevf;
	prevf = &freelist;
	while(cfl && emptyBloc > cfl)
	{
		prevf = &cfl->next;
		cfl = cfl->next;
	}
	emptyBloc->next = cfl;
	(*prevf) = emptyBloc;


}


void
kheap_init()
{
    kernel_heap_top = (uint8_t*) &__kernel_heap_start__;
    kernel_heap_limit = (uint8_t*) &__kernel_heap_end__;

    freelist = (struct fl *)kernel_heap_top;

    kernel_heap_top += sizeof(struct fl);

    freelist->size = (unsigned int)( kernel_heap_limit - kernel_heap_top);
    freelist->bloc = kernel_heap_top;
}
