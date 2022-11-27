#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <stdbool.h>
#include <hash.h>

struct page {
    struct thread* owner;
    struct frame_table_entry* entry;
};

struct supp_page_table_entry {
    void *user_vaddr;
    bool no_data;       /* Not expect any data */
    bool kernel_vm;     /* Lies within kernel virtual memory */
    bool read_only;     /* Read-only page */

    struct hash_elem h_elem;
};

#endif /* vm/page.h */