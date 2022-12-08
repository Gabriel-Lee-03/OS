#include <stdio.h>
#include <stdlib.h>
#include "vm/frame.h"
#include "threads/palloc.h"
#include "threads/thread.h"
#include "threads/loader.h"
#include "userprog/pagedir.h"

static struct frame_table_entry* entries;
static int frame_table_size;

static struct lock frame_alloc_lock;

void frame_init(void) {
    void* frame_ptr;
    struct frame_table_entry* entry;
    entries = malloc(sizeof(*entries) * init_ram_pages);
    if (entries != NULL) {
        frame_ptr = palloc_get_page(PAL_USER);
        while (frame_ptr != NULL) {
            frame_table_size++;
            entry = &entries[frame_table_size];
            entry->frame_ptr = frame_ptr;
            entry->page_entry = NULL;
            lock_init (&entry->f_lock);
            frame_ptr = palloc_get_page(PAL_USER);
        }
    }
    lock_init (&frame_alloc_lock);
}

/* Allocates a frame by looking for a free frame, and if none exist, evicting a page to get a frame.
   Also locks the frame. */
struct frame_table_entry* frame_alloc(struct supp_page_table_entry* page_entry) {
    struct frame_table_entry* entry;
    lock_acquire (&frame_alloc_lock);

    for (int i = 0; i < frame_table_size; i++) {
        entry = &entries[i];
        if (!lock_try_acquire(&entry->f_lock)){
            continue;
        }
        /* Find a free frame */
        if (entry->page_entry == NULL) {
            entry->page_entry = page_entry;
            lock_release (&frame_alloc_lock);
            return entry;
        }
        lock_release(&entry->f_lock);
    }

    /* No free frame, so need to evict a frame. */
    static size_t loop_helper = 0;
    for (int i = 0; i < frame_table_size * 2; i++) {

        struct frame_table_entry *entry = &entries[loop_helper];

        loop_helper++;
        if (loop_helper >= frame_table_size) {
            loop_helper = 0;
        }

        if (!lock_try_acquire(&entry->f_lock)){
            continue;
        }

        /* If frame does not have a page, for example if it was emptied 
        during this process, allocate directly. */
        if (entry->page_entry == NULL) {
            entry->page_entry = page_entry;
            lock_release (&frame_alloc_lock);
            return entry;
        }

        if (pagedir_is_accessed (entry->page_entry->owner->pagedir, entry->page_entry->user_vaddr)) {
            pagedir_set_accessed (entry->page_entry->owner->pagedir, entry->page_entry->user_vaddr, false);
            lock_release (&entry->f_lock);
            continue;
        }

        lock_release (&frame_alloc_lock);
        evict_page (entry->page_entry);
        entry->page_entry = page_entry;
        return entry;
    }
    /* Run out of frames and swap slot. */
    lock_release (&frame_alloc_lock);
    return NULL;
}

void frame_lock (struct supp_page_table_entry* page_entry) {
    struct frame_table_entry* entry = page_entry->frame_entry;
    if (entry != NULL) {
        lock_acquire (&entry->f_lock);
    }
}

void frame_unlock (struct frame_table_entry *f) {
    lock_release (&f->f_lock);
}


void frame_release (struct frame_table_entry *f) {
    f->page_entry == NULL;
}