#include "vm/page.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "filesys/file.h"

static bool add_page (struct supp_page_table_entry *);

/* creates a new page */
struct supp_page_table_entry* new_page(void *user_vaddr, bool read_only) {
    struct supp_page_table_entry* entry = malloc(sizeof(struct supp_page_table_entry));
    ASSERT(entry != NULL);
    entry->owner = thread_current();
    entry->user_vaddr = user_vaddr;
    entry->frame_entry = NULL;
    entry->first_sector = NOT_IN_SWAP;
    entry->read_only = read_only;
    entry->f = NULL;
    entry->f_offset = 0;
    entry->f_size = 0;
    hash_insert(&thread_current()->supp_page_table, &entry->h_elem);
    return entry;
}

/* removes a given page */
void remove_page(void *user_vaddr) {
    struct supp_page_table_entry* entry = page_info_lookup(user_vaddr);
    frame_lock(entry);
    /* frees the frame if needed */
    if (entry->frame_entry != NULL) {
        free(entry->frame_entry);
    }
    entry->frame_entry = NULL;
    frame_unlock(entry->frame_entry);
    /* deletes to page from the hash table */
    hash_delete(&thread_current()->supp_page_table, &entry->h_elem);
    free(entry);
}

static bool add_page (struct supp_page_table_entry *p) {
    /* try to allocate a frame, returns false if it can't */
    p->frame_entry = frame_alloc (p);
    if (p->frame_entry == NULL) {
        return false;
    }

    /* if the page was stored to swap, swap_in and mark first_sector as NOT_IN_SWAP */
    if (p->first_sector != NOT_IN_SWAP) {
        swap_in (p->frame_entry->frame_ptr, p->first_sector);
        p->first_sector = NOT_IN_SWAP;
    } else if (p->f != NULL) {
        /* if the page has a file, copy the data from said file */
        lock_acquire(&file_lock);
        off_t read = file_read_at (p->f, p->frame_entry->frame_ptr, p->f_size, p->f_offset);
        lock_release(&file_lock);
        memset (p->frame_entry->frame_ptr + read, 0, (PGSIZE - read));
    } else {
        /* if not, add an all 0 page */
        memset (p->frame_entry->frame_ptr, 0, PGSIZE);
    }

    lock_release (&p->frame_entry->f_lock);

    return true;
}

bool add_from_page_fault (struct supp_page_table_entry *p) {
    // if the current thread doesn't have a page table it can't handle the fault
    if (&thread_current()->supp_page_table == NULL) {
        return false;
    }

    /* searches for the page, if it doesn't exist, return false */
    if (p == NULL) {
        return false;
    }

    /* if it doesn't have a frame, attempt to asign it one, returning false on fail */
    if (p->frame_entry == NULL) {
        if (!add_page (p)) {
            return false;
        }
    }

    /* set the page to the frame table, returns if it sucseeded or not */
    return pagedir_set_page (thread_current()->pagedir, p->user_vaddr, p->frame_entry->frame_ptr, !p->read_only);
}

/* looks up the page given an address */
struct supp_page_table_entry* page_info_lookup(void *user_vaddr) {
    /* if it isnt a user address return NULL */
    if (user_vaddr >= PHYS_BASE) {
        return NULL;
    }

    struct supp_page_table_entry entry;
    entry.user_vaddr = pg_round_down(user_vaddr);

    /* try and find the page in the hash table */
    struct hash_elem* result = hash_find(&thread_current()->supp_page_table, &entry.h_elem);
    if (result == NULL) {
        return NULL;
    }
    else {
        return hash_entry(result, struct supp_page_table_entry, h_elem);
    }
}

uint32_t page_hash (struct hash_elem* elem, void *aux UNUSED) {
	struct supp_page_table_entry* entry = hash_entry(elem, struct supp_page_table_entry, h_elem);
	return hash_int((uint32_t) entry->user_vaddr);
}

bool page_less (const struct hash_elem *a_elem, const struct hash_elem *b_elem,
           void *aux UNUSED) {
  struct supp_page_table_entry *a_page = hash_entry (a_elem, struct supp_page_table_entry, h_elem);
  struct supp_page_table_entry *b_page = hash_entry (b_elem, struct supp_page_table_entry, h_elem);

  return a_page->user_vaddr < b_page->user_vaddr;
}

/* frees a page */
void page_free (struct hash_elem *elem, void *aux UNUSED) {
    struct supp_page_table_entry *entry = hash_entry (elem, struct supp_page_table_entry, h_elem);
    free(entry);
}

/* evicts a page */
void
evict_page (struct supp_page_table_entry *p) {
    pagedir_clear_page (p->owner->pagedir, p->user_vaddr);
    if (pagedir_is_dirty (p->owner->pagedir, p-> user_vaddr)) {
        if (!(p->f == NULL) && writable_file (p->f)) {
            file_write_at (p->f, p->frame_entry->frame_ptr, p->f_size, p->f_offset);
        }
        else {
            p->first_sector = swap_out (p->frame_entry->frame_ptr);
        }
    }
    p->frame_entry = NULL;
}