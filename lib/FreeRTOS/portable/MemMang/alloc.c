/* 
 * This is a glue between newlib and FreeRTOS heap2 allocator !
 * You need to understand how heap2 works and its limitations,
 * otherwise you will run out of memory.
 *
 * Michal Demin - 2010
 *
 * TODO: reent is there for a reason !
 *
 */

#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"

/* definition of block structure, copied from heap2 allocator */
typedef struct A_BLOCK_LINK
{
	struct A_BLOCK_LINK *pxNextFreeBlock;	/*<< The next free block in the list. */
	size_t xBlockSize;						/*<< The size of the free block. */
} xBlockLink;

static const unsigned short  heapSTRUCT_SIZE	= ( sizeof( xBlockLink ) + portBYTE_ALIGNMENT - ( sizeof( xBlockLink ) % portBYTE_ALIGNMENT ) );

_PTR _realloc_r(struct _reent *re, _PTR oldAddr, size_t newSize) {
	xBlockLink *block;
	size_t toCopy;
	void *newAddr;

	newAddr = pvPortMalloc(newSize);

	if (newAddr == NULL)
		return NULL;

	/* We need the block struct pointer to get the current size */
	block = oldAddr;
	block -= heapSTRUCT_SIZE;

	/* determine the size to be copied */
	toCopy = (newSize<block->xBlockSize)?(newSize):(block->xBlockSize);

	/* copy old block into new one */
	memcpy((void *)newAddr, (void *)oldAddr, (size_t)toCopy);

	vPortFree(oldAddr);
	
	return newAddr;
}

_PTR _calloc_r(struct _reent *re, size_t num, size_t size) {
	return pvPortMalloc(num*size);
}

_PTR _malloc_r(struct _reent *re, size_t size) {
	return pvPortMalloc(size);
}

_VOID _free_r(struct _reent *re, _PTR ptr) {
	vPortFree(ptr);
}

