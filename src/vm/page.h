#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <inttypes.h>

struct page {
    struct thread* owner;
    struct frame_table_entry* entry;
};

#endif /* vm/page.h */