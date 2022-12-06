#ifndef VM_SWAP_H
#define VM_SWAP_H

#include "vm/page.h"

/* Initialises the swap table. */
void swap_init (void);
/* Swaps a page from main memory to a swap slot on the disk. */
void swap_to_disk (struct page*);
/* Swaps a page from a swap slot on the disk to main memory. */
void swap_from_disk (struct page*);

#endif /* vm/swap.h */