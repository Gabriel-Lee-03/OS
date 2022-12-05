#ifndef VM_FRAME_H
#define VM_FRAME_H

#include <inttypes.h>
#include "vm/page.h"
#include "threads/synch.h"

struct frame_table_entry {
    void* frame_ptr;
    struct supp_page_table_entry* page_entry;
    struct lock f_lock;
};


void frame_init(void);
struct frame_table_entry* frame_alloc(struct supp_page_table_entry*);

void frame_lock (struct supp_page_table_entry *);
void frame_unlock (struct frame_table_entry *);

#endif /* vm/frame.h */