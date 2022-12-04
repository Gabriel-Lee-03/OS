#ifndef VM_FRAME_H
#define VM_FRAME_H

#include <inttypes.h>

struct frame_table_entry {
    void* frame_ptr;
    struct page *page;
    struct lock f_lock;
};

void frame_init(void);
struct frame_table_entry* frame_alloc(struct page*);

void frame_lock (struct page *);
void frame_unlock (struct frame *);

#endif /* vm/frame.h */