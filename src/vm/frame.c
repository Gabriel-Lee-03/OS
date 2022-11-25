#include <stdio.h>
#include "vm/frame.h"
#include "threads/palloc.h"

#define MAX_FRAME_SIZE (1024 * 1024)

static struct frame_table_entry* entries;
static int frame_size;

void frame_init(void) {
    void* frame_ptr;
    struct frame_table_entry* entry;
    entries = malloc(sizeof(*entries) * MAX_FRAME_SIZE);
    if (entries != NULL) {
        frame_ptr = palloc_get_page(PAL_USER);
        while (frame_ptr != NULL) {
            frame_size++;
            entry = &entries[frame_size];
            entry->frame_ptr = frame_ptr;
            entry->page = NULL;
            frame_ptr = palloc_get_page(PAL_USER);
        }
    }
}

struct frame_table_entry* frame_alloc(struct page* page) {
    struct frame_table_entry* entry;
    for (int i = 0; i < frame_size; i++) {
        entry = &entries[i];
        /* Find a free frame */
        if (entry->page == NULL) {
            entry->page = page;
            return entry;
        }
    }
    return NULL;
}


