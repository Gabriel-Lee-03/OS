#ifndef VM_FRAME_H
#define VM_FRAME_H

#include <inttypes.h>

struct frame_table_entry {
    void* frame_ptr;
    struct page *page;
};

#endif /* vm/frame.h */