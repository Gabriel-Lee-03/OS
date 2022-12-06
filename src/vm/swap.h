#ifndef VM_SWAP_H
#define VM_SWAP_H

#include "vm/page.h"

/* Initialises the swap table. */
void swap_init (void);
/* Swaps a page from main memory to a swap slot on the disk. */
void swap_to_disk (struct supp_page_table_entry*);
/* Swaps a page from a swap slot on the disk to main memory. */
void swap_from_disk (struct supp_page_table_entry*);

#endif /* vm/swap.h */