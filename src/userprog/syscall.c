#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include <stdlib.h>
#include <console.h>
#include <stdlib.h>
#include <inttypes.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "userprog/process.h"
#include "userprog/pagedir.h"
#include "threads/vaddr.h"
#include "filesys/directory.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "threads/synch.h"
#include "devices/shutdown.h"
#include "devices/input.h"

// Task 2
static void syscall_handler (struct intr_frame *);
static void sc_halt(void);
void sc_exit(int);
static pid_t sc_exec(const char *);
static int sc_wait (pid_t);
static bool sc_create (const char *, unsigned );
static bool sc_remove (const char *);
static int sc_open (const char *);
static int sc_filesize (int);
static int sc_read (int, void *, unsigned);
static int sc_write (int, const void *, unsigned);
static void sc_seek (int, unsigned);
static unsigned sc_tell (int);
static void sc_close (int);
static mapid_t sc_mmap(int, void*);
static void sc_munmap(mapid_t);
static struct mmapping* get_mmapping(mapid_t);
static struct file* get_file(int);
static void* check_mem_access(const void *);

static struct mmapping {
    mapid_t mapid;
    int fd;
    uint8_t* begin_addr;
    uint8_t* end_addr;
    struct list_elem elem;
};

void
syscall_init (void) 
{
  lock_init(&file_lock);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

/* gets the system call value and calls the corresponding function*/
static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  /* gets the value of the system call*/
  check_mem_access(f->esp);
  int syscall_num = *((int*)f->esp);

  int status;
  char *file;
  const char *cmd_line;
  pid_t pid;
  unsigned initial_size;
  int fd;
  void *buffer;
  unsigned size;
  unsigned position;
  void *addr;
  mapid_t mapping;

  switch (syscall_num) {
  case SYS_HALT:
    sc_halt();
    break;

  case SYS_EXIT:
    check_mem_access(f->esp + 1);
    status = *(((int*)f->esp) + 1);
    if (status < -1) {
      status = -1;
    }
    sc_exit(status);
    break;

  case SYS_EXEC:
    check_mem_access(f->esp + 1);
    cmd_line = *(((char**)f->esp) + 1); // first argv
    check_mem_access(cmd_line);
    f->eax = (uint32_t) sc_exec(cmd_line);
    break;

  case SYS_WAIT:
    check_mem_access(f->esp + 1);
    pid = *(((int*)f->esp) + 1);
    f->eax = (uint32_t) sc_wait(pid);
    break;

  case SYS_CREATE:
    check_mem_access(f->esp + 1);
    check_mem_access(f->esp + 2);
    file = *(((char**)f->esp) + 1);
    check_mem_access(file);
    initial_size = *(((int*)f->esp) + 2);
    f->eax = (uint32_t) sc_create(file, initial_size);
    break;
    
  case SYS_REMOVE:
    check_mem_access(f->esp + 1);
    file = *(((char**)f->esp) + 1);
    f->eax = (uint32_t) sc_remove(file);
    break;

  case SYS_OPEN:
    check_mem_access(f->esp + 1);
    file = *(((char**)f->esp) + 1);
    check_mem_access(file);
    f->eax = (uint32_t) sc_open(file);
    break;

  case SYS_FILESIZE:
    check_mem_access(f->esp + 1);
    fd = *(((int*)f->esp) + 1);
    f->eax = (uint32_t) sc_filesize(fd);
    break;
    
  case SYS_READ:
    check_mem_access(f->esp + 1);
    check_mem_access(f->esp + 2);
    check_mem_access(f->esp + 3);
    fd = *(((int*)f->esp) + 1);
    buffer = *(((int**)f->esp) + 2);
    size = *(((unsigned*)f->esp) + 3);
    check_mem_access(buffer);
    f->eax = (uint32_t) sc_read(fd, buffer, size);
    break; 

  case SYS_WRITE:
    check_mem_access(f->esp + 1);
    check_mem_access(f->esp + 2);
    check_mem_access(f->esp + 3);
    fd = *(((int*)f->esp) + 1);
    buffer = *(((int**)f->esp) + 2);
    size = *(((int*)f->esp) + 3);
    check_mem_access(buffer);
    f->eax = (uint32_t) sc_write(fd, buffer, size);
    break;

  case SYS_SEEK:
    check_mem_access(f->esp + 1);
    check_mem_access(f->esp + 2);
    fd = *(((int*)f->esp) + 1);
    position = *(((unsigned*)f->esp) + 2);
    sc_seek(fd, position);
    break;

  case SYS_TELL:
    check_mem_access(f->esp + 1);
    fd = *(((int*)f->esp) + 1);
    f->eax = (uint32_t) sc_tell(fd);
    break;
    
  case SYS_CLOSE:
    check_mem_access(f->esp + 1);
    fd = *(((int*)f->esp) + 1);
    sc_close(fd);
    break; 

  case SYS_MMAP:
    check_mem_access(f->esp + 1);
    check_mem_access(f->esp + 2);
    fd = *(((int*)f->esp) + 1);
    addr = *(((int**)f->esp) + 2);
    check_mem_access(addr);
    f->eax = (mapid_t) sc_mmap(fd, addr);
    break; 
  
  case SYS_MUNMAP:
    check_mem_access(f->esp + 1);
    mapping = *(((int*)f->esp) + 1);
    sc_munmap(mapping);
    break; 


  }
}

// Task 2
/* terminates pintos */
static void sc_halt(void) {
  shutdown_power_off();
}

/* terminates the current user program */
void sc_exit(int status) {
  while (!list_empty(&thread_current()->mmapping_list)) {
    struct list_elem* curr_elem = list_front(&thread_current()->mmapping_list);
    sc_munmap(list_entry(curr_elem, struct mmapping, elem)->mapid);
  }

  /* saving its exit status to the current thread */
  thread_current()->exit_status = status;
  thread_exit();
}

/* runs process_execute on the program with the corresponding file name */
static pid_t sc_exec(const char *cmd_line) {
  lock_acquire(&file_lock);
  char* file_name = malloc(strlen(cmd_line) + 1);
  char* save_ptr;
	strlcpy(file_name, cmd_line, strlen(cmd_line) + 1);
	file_name = strtok_r(file_name, " ", &save_ptr);

  struct file* f = filesys_open (file_name);
  if (f == NULL) {
    lock_release(&file_lock);
    return -1;
  }
  file_close(f);
  lock_release(&file_lock);
  return process_execute(cmd_line);
}

/* waits on the child process with the corresponding pid */
static int sc_wait (pid_t pid) {
  return process_wait(pid);
}

/* adds a new file with the specified name and size to the file system */
static bool sc_create (const char *file, unsigned initial_size) {
  if (file == NULL) {
    sc_exit(-1);
  }
  lock_acquire(&file_lock);
  bool created = filesys_create(file, initial_size);
  lock_release(&file_lock);
  return created;
}

/* removes a file from the file system and returns if it was sucsessful */
static bool sc_remove (const char *file) {
  lock_acquire(&file_lock);
  bool removed = filesys_remove(file);
  lock_release(&file_lock);
  return removed;
}

/* opens the file with the corresponding name, returns -1 if the file doesnt exist or the fd otherwise */
static int sc_open (const char *file) {
  if (file == NULL) {
    return -1;
  }
  struct file_with_fd* new_file_with_fd = malloc(sizeof(struct file_with_fd));
  lock_acquire(&file_lock);
  struct file* new_file = filesys_open(file);
  lock_release(&file_lock);
  if (new_file == NULL) {
    return -1;
  }

  /* Generate new fd for the file and store the conversion 
     in file_list of current thread */
  new_file_with_fd->file_ptr = new_file;
  new_file_with_fd->fd = list_size(&thread_current()->file_list)+2; 
  list_push_back(&thread_current()->file_list, &new_file_with_fd->elem);
  return new_file_with_fd->fd;
}

/* returns the byte size of the file */
static int sc_filesize (int fd) {
  struct file *file = get_file(fd);
  if (file == NULL) {
    sc_exit(-1);
  }
  lock_acquire(&file_lock);
  int size = file_length(file);
  lock_release(&file_lock);
  return size;
}

/* reads bytes from the open file to a buffer. returns -1 if it couldnt be read, otherwise returns the number of bytes read */
static int sc_read (int fd, void *buffer, unsigned size) {
  if (fd == 0){
    uint8_t *temp_buff = (uint8_t *) buffer;
    for (int i = 0; i < size; i++) {
      temp_buff[i] = input_getc();
    }
    return size;
  }
  
  if (fd > 0) {
    struct file* file_to_read = get_file(fd);
    if (file_to_read == NULL) {
      sc_exit(-1);
    }
    lock_acquire(&file_lock);
    off_t size_read = file_read(file_to_read, buffer, size);
    lock_release(&file_lock);
    return size_read;
  }

  return -1;
}

/* writes from the buffer to the open file. it returns the number of bytes written */
static int sc_write (int fd, const void *buffer, unsigned size) {
  int written_bytes;
  /* Write to console */
  if (fd == 1) {
    /* Only write 500 characters if it contains more than 500 */
    if (size > 500) {
      putbuf((char *) buffer, 500);
      return size;
    }
    else {
      putbuf((char *) buffer, size);
      return size;
    }
  }
  /* Write to file */
  else {
    struct file* file_to_write = get_file(fd);
    if (file_to_write == NULL) {
      sc_exit(-1);
    }
    lock_acquire(&file_lock);
    int written_bytes = file_write(file_to_write, buffer, size);
    lock_release(&file_lock);
    return written_bytes;
  }
}

/* changes the next byte in open file to the given position */
static void sc_seek (int fd, unsigned position) {
  struct file *file = get_file(fd);
  if (file == NULL) {
    sc_exit(-1);
  }
  
  lock_acquire(&file_lock);
  file_seek(file, position);
  lock_release(&file_lock);
  
}

/* returns the next byte's position in the open file */
static unsigned sc_tell (int fd) {
  struct file *file = get_file(fd);
  if (file == NULL) {
    sc_exit(-1);
  }
  
  lock_acquire(&file_lock);
  off_t pos = file_tell(file);
  lock_release(&file_lock);
  return pos;
}

/* closes the given file */
static void sc_close (int fd) {
  struct file *file_to_close = get_file(fd);
  if (file_to_close == NULL) {
    sc_exit(-1);
  }
  
  lock_acquire(&file_lock);
  file_close(file_to_close);

  /* Change file to null in struct file_with_fd */
  struct list_elem* curr_elem = list_front(&thread_current()->file_list);
  for (int i = 2; i < fd; i++) {
    curr_elem = curr_elem->next;
  }
  list_entry(curr_elem, struct file_with_fd, elem)->file_ptr = NULL;
  lock_release(&file_lock);
}

static mapid_t sc_mmap(int fd, void* addr) {
  struct file *file_to_map = get_file(fd);
  if (file_to_map == NULL || addr == 0 || pg_ofs(addr) != 0) {
    return -1;
  }

  struct mmapping *map = malloc(sizeof(struct mmapping));
  lock_acquire(&file_lock);
  off_t length = file_length(&file_to_map);
  lock_release(&file_lock);

  if (length == 0) {
    return -1;
  }

  struct file_with_fd* new_file_with_fd = malloc(sizeof(struct file_with_fd));
  int new_file = file_reopen(file_to_map);
  /* Generate new fd for the file and store the conversion 
     in file_list of current thread */
  new_file_with_fd->file_ptr = new_file;
  new_file_with_fd->fd = list_size(&thread_current()->file_list) + 2; 
  list_push_back(&thread_current()->file_list, &new_file_with_fd->elem);

  // TBD: Page allocation
  /*
  off_t offset = 0
  while (length > 0) {
    struct page* p = // allocate ...
    p->file = new_file;
    p->offset = offset;
    if (length > PGSIZE) {
      offset += PGSIZE;
      length -= PGSIZE;
    }
    else {
      length -= PGSIZE;
    }
  }
  */

  map->mapid = thread_current()->next_mapid;
  thread_current()->next_mapid++;
  map->fd = new_file_with_fd->fd;
  map->begin_addr = addr;
  // map->end_addr = addr + offset;
  list_push_back(&thread_current()->mmapping_list, &map->elem);
}

static void sc_munmap(mapid_t mapping) {
  struct mmapping* map = get_mmapping(mapping);
  if (map != NULL) {
    list_remove(&map->elem);

    // TBD: Page deallocation
    /*
    void* curr_addr = map->start_addr;
    int offset = 0
    while (curr_addr < end->start_addr) {
      // deallocate page ...
        curr_addr += PGSIZE;
    }
    */
    free(map);
  }
}

/* gets the mapping from mapid */
static struct mmapping* get_mmapping(mapid_t mapid) {
  if (mapid < 1 || list_empty(&thread_current()->mmapping_list)) {
    return NULL;
  }
  struct list_elem* curr_elem = list_front(&thread_current()->mmapping_list);
  while (curr_elem != list_tail(&thread_current()->mmapping_list)) {
    struct mmapping* map = list_entry(curr_elem, struct mmapping, elem)->mapid;
    /* Found */
    if (map->mapid == mapid) {
      return map;
    }
    curr_elem = curr_elem->next;
  }
  /* Not found */
  return NULL;
}

/* gets the given file */
static struct file* get_file(int fd) {
  if (fd < 2 || list_empty(&thread_current()->file_list)) {
    return NULL;
  }
  struct list_elem* curr_elem = list_front(&thread_current()->file_list);
  for (int i = 2; i < fd; i++) {
    curr_elem = curr_elem->next;
    if (curr_elem == list_tail(&thread_current()->file_list)) {
      return NULL;
    }
  }
  return list_entry(curr_elem, struct file_with_fd, elem)->file_ptr;
}

static void* check_mem_access(const void *vaddr)
{
	if (!is_user_vaddr(vaddr)) {
		sc_exit(-1);
		return 0;
	}
	void *ptr = pagedir_get_page(thread_current()->pagedir, vaddr);
	if (!ptr)	{
		sc_exit(-1);
		return 0;
	}
	return ptr;
}