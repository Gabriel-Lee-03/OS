#include "vm/page.h"
#include <stdint.h>
#include "threads/thread.h"

struct supp_page_table_entry* page_info_lookup(void *);
uint32_t page_hash (struct hash_elem*, void *aux);
bool page_less (const struct hash_elem*, const struct hash_elem*,
           void *aux);
void page_free(struct hash_elem*, void *aux);
// TBD: Add
/*
void add() {
    struct supp_page_table_entry* entry = malloc(sizeof(struct supp_page_table_entry));
    // Set variables
    // hash_insert()
}
*/

static bool add_page (struct supp_page_table_entry *p) {
    /* try to allocate a frame, returns false if it can't */
    p->entry = frame_alloc (p);
    if (p->entry == NULL) {
        return false;
    }

    /* if the page has a file, copy the data from said file */
    if (p->f != NULL) {
        off_t read = file_read_at (p->f, p->entry->frame_ptr, p->f_size, p->f_offset);
        memset (p->entry->frame_ptr + read, 0, (PGSIZE - read));
    } else {
        /* if not, add an all 0 page */
        memset (p->entry->frame_ptr, 0, PGSIZE);
    }

    return true;
}

bool add_from_page_fault (void *fault_addr) {
    /* if the current thread doesn't have a page table it can't handle the fault */
    if (thread_current()->supp_page_table == NULL) {
        return false;
    }

    /* searches for the page, if it doesn't exist, return false */
    struct supp_page_table_entry *p = page_info_lookup (fault_addr);
    if (p == NULL) {
        return false;
    }

    /* if it doesn't have a frame, attempt to asign it one, returning false on fail */
    if (p->entry == NULL) {
        if (!add_page (p)) {
            return false;
        }
    }

    /* set the page to the frame table, returns if it sucseeded or not */
    return pagedir_set_page (thread_current()->pagedir, p->user_vaddr, p->entry->frame_ptr, !p->read_only);
}


struct supp_page_table_entry* page_info_lookup(void *user_vaddr) {
    struct supp_page_table_entry* entry;
    entry->user_vaddr = user_vaddr;

    struct hash_elem* result = hash_find(&thread_current()->supp_page_table, &entry->h_elem);
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

void page_free(struct hash_elem* elem, void *aux UNUSED) {
    struct supp_page_table_entry *entry = hash_entry (elem, struct supp_page_table_entry, h_elem);
    // free(entry);
}