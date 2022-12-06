#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <stdbool.h>
#include <inttypes.h>
#include <hash.h>
#include "filesys/off_t.h"
#include "vm/frame.h"
#include "devices/block.h"

struct supp_page_table_entry {
    struct thread* owner;
    struct frame_table_entry* frame_entry;
    block_sector_t first_sector;
    void *user_vaddr;
    bool no_data;       /* Not expect any data */
    bool read_only;     /* Read-only page */

    struct file *f;
    off_t f_offset;
    off_t f_size;

    struct hash_elem h_elem;
};

struct supp_page_table_entry* page_info_lookup(void *);
uint32_t page_hash (struct hash_elem *, void *aux);
bool page_less (const struct hash_elem *, const struct hash_elem *,
           void *aux);
void page_free(struct hash_elem*, void *aux);

#endif /* vm/page.h */