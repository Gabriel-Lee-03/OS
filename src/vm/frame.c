#include <stdio.h>
#include <stdlib.h>
#include "vm/frame.h"
#include "threads/palloc.h"
#include "threads/thread.h"
#include "userprog/pagedir.h"
#include "threads/loader.h"

static struct list entries;

static struct lock frame_alloc_lock;

void frame_init(void) {
    list_init(&entries);
    lock_init (&frame_alloc_lock);
}

struct frame_table_entry* frame_alloc(struct supp_page_table_entry* page_entry) {
    lock_acquire (&frame_alloc_lock);
    void *frame_ptr = palloc_get_page(PAL_USER);

    if (frame_ptr != NULL) {
        struct frame_table_entry* entry = malloc(sizeof(entry));
        if (entry == NULL) {
            return false;
        }
        // allocate frame entry
        entry->frame_ptr = frame_ptr;
        entry->page_entry = page_entry;
        lock_init(&entry->f_lock);
        lock_acquire(&entry->f_lock);
        list_push_back(&entries, &entry->elem);
        lock_release (&frame_alloc_lock);
        return entry;
    }  

    for (int i = 0; i < list_size(&entries); i++) {
        struct list_elem* curr_elem = list_pop_front(&entries);
        struct frame_table_entry* entry = list_entry(curr_elem, struct frame_table_entry, elem);
        list_push_back(&entries, curr_elem);
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
    for (int i = 0; i < 2 * list_size(&entries); i++) {

        struct list_elem* curr_elem = list_pop_front(&entries);
        struct frame_table_entry* entry = list_entry(curr_elem, struct frame_table_entry, elem);
        list_push_back(&entries, curr_elem);

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