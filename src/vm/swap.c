#include "vm/swap.h"
#include <bitmap.h>
#include "devices/block.h"
#include "threads/synch.h"
#include "threads/vaddr.h"
#include "vm/frame.h"
#include "vm/page.h"

#define SECTORS_PER_PAGE (PGSIZE / BLOCK_SECTOR_SIZE)

static struct lock swap_lock;
static struct block *swap_block;
static struct bitmap *swap_bitmap;

void
swap_init() {
    lock_init(&swap_lock);
    swap_block = block_get_role (BLOCK_SWAP);
    swap_bitmap = bitmap_create (block_size (swap_block));
}

void
swap_from_disk (struct page *p) {
    lock_acquire (&swap_lock);
    for (size_t i = 0; i < SECTORS_PER_PAGE; i++) {
        block_read (swap_block, p->first_sector + i, p->entry->frame_ptr + (i * BLOCK_SECTOR_SIZE));
        bitmap_reset (swap_bitmap, p->first_sector + i);
    }
    lock_release (&swap_lock);
}

void
swap_to_disk (struct page *p) {
    lock_acquire (&swap_lock);
    size_t swap_slot = bitmap_scan_and_flip (swap_bitmap, 0, SECTORS_PER_PAGE, false);
    lock_release (&swap_lock);

    ASSERT (swap_slot != BITMAP_ERROR);

    p->first_sector = swap_slot;

    for (size_t i = 0; i < SECTORS_PER_PAGE; i++) {
        block_write (swap_block, p->first_sector + i, p->entry->frame_ptr + (i * BLOCK_SECTOR_SIZE));
    }
}