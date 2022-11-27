#ifndef VM_FRAME_H
#define VM_FRAME_H

#include <inttypes.h>

struct frame_table_entry {
    void* frame_ptr;
    struct page *page;
};

void frame_init(void);
struct frame_table_entry* frame_alloc(struct page*);

#endif /* vm/frame.h */