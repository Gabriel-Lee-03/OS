#include <stdio.h>
#include "vm/frame.h"
#include "threads/palloc.h"

#define MAX_FRAME_SIZE (1024 * 1024)

static struct frame_table_entry* entries;
static int size;

void frame_init() {
    void* frame_ptr;
    struct frame_table_entry* entry;
    entries = malloc(sizeof(*entries) * MAX_FRAME_SIZE);
    if (entries != NULL) {
        frame_ptr = palloc_get_page(PAL_USER);
        while (frame_ptr != NULL) {
            size++;
            entry = &entries[size];
            entry->frame_ptr = frame_ptr;
            entry->page = NULL;
            frame_ptr = palloc_get_page(PAL_USER);
        }
    }
}