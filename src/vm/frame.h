#ifndef VM_FRAME_H
#define VM_FRAME_H

#include <inttypes.h>

struct list frame_table;

struct frame_table_entry {
    uint32_t* frame;
    struct page *page;
};

#endif /* vm/frame.h */