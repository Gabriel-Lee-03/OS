#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "userprog/process.h"
#include "threads/vaddr.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "lib/kernel/stdio.h"

// Task 2
static void syscall_handler (struct intr_frame *);
static void sc_halt(void);
static void sc_exit(int);
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
static struct file* get_file(int);

// Task 2
struct file_with_fd {
  struct file* file_ptr;
  int fd;
  struct list_elem elem;
};

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  /*
  if (!is_user_vaddr(f->esp)) {
    //page_fault(&f);
    return;
  }
  if (!pagedir_get_page(thread_current()->pagedir, f->esp)) {
    //page_fault(&f);
    return;
  }
  */
  int syscall_num = (int)f->esp;

  int status;
  char *file;
  const char *cmd_line;
  pid_t pid;
  unsigned initial_size;
  int fd;
  void *buffer;
  unsigned size;
  unsigned position;

  switch (syscall_num) {
  case SYS_HALT:
    sc_halt();
    break;

  case SYS_EXIT:
    status = (int)(f->esp) + 1;
    sc_exit(status);
    break;

  case SYS_EXEC:
    cmd_line = (char *)((int)(f->esp) + 1); // first argv
    f->eax = (uint32_t) sc_exec(cmd_line);
    break;

  case SYS_WAIT:
    pid = (int)(f->esp) + 1;
    f->eax = (uint32_t) sc_wait(pid);
    break;

  case SYS_CREATE:
    file = (char *)((int)(f->esp) + 1);
    initial_size = (int)(f->esp) + 2;
    f->eax = (uint32_t) sc_create(file, initial_size);
    break;
    
  case SYS_REMOVE:
    file = (char *)((int)(f->esp) + 1);
    f->eax = (uint32_t) sc_remove(file);
    break;

  case SYS_OPEN:
    file = (char *)((int)(f->esp) + 1);
    f->eax = (uint32_t) sc_open(file);
    break;

  case SYS_FILESIZE:
    fd = (int)(f->esp) + 1;
    f->eax = (uint32_t) sc_filesize(fd);
    break;
    
  case SYS_READ:
    fd = (int)(f->esp) + 1;
    buffer = (int)(f->esp) + 2;
    size = (int)(f->esp) + 3;
    f->eax = (uint32_t) sc_read(fd, buffer, size);
    break; 

  case SYS_WRITE:
    fd = (int)(f->esp) + 1;
    buffer = (int)(f->esp) + 2;
    size = (int)(f->esp) + 3;
    f->eax = (uint32_t) sc_write(fd, buffer, size);
    break;

  case SYS_SEEK:
    fd = (int)(f->esp) + 1;
    position = (int)(f->esp) + 2;
    sc_seek(fd, position);
    break;

  case SYS_TELL:
    fd = (int)(f->esp) + 1;
    f->eax = (uint32_t) sc_tell(fd);
    break;
    
  case SYS_CLOSE:
    fd = (int)(f->esp) + 1;
    sc_close(fd);
    break; 
  }

  printf ("system call!\n");
  thread_exit ();
}

// Task 2
static void sc_halt(void) {
  shutdown_power_off();
}

static void sc_exit(int status) {
  thread_current()->exit_status = status;
  thread_exit();
}

static pid_t sc_exec(const char *cmd_line) {
  return 0;
}

static int sc_wait (pid_t pid) {
  return process_wait(pid);
}

static bool sc_create (const char *file, unsigned initial_size) {
  return filesys_create(file, initial_size);
}

static bool sc_remove (const char *file) {
  return filesys_remove(file);
}

static int sc_open (const char *file) {
  struct file_with_fd* new_file_with_fd;
  struct file* new_file = filesys_open(file);
  if (new_file == NULL) {
    return -1;
  }
  new_file_with_fd->file_ptr = new_file;
  new_file_with_fd->fd = list_size(&thread_current()->file_list); 
  list_push_back(&thread_current()->file_list, &new_file_with_fd->elem);
  return new_file_with_fd->fd;
}

static int sc_filesize (int fd) {
  return 0;
}

static int sc_read (int fd, void *buffer, unsigned size) {
  return 0;
}

static int sc_write (int fd, const void *buffer, unsigned size) {
  if (fd == 1) {
    if (size > 500) {
      putbuf((char *) buffer, 500);
      return 500;
    }
    else {
      putbuf((char *) buffer, size);
      return size;
    }
  }
  else {
    struct file* file_to_write = get_file(fd);
    return file_write(file_to_write, buffer, size);
  }
}

static void sc_seek (int fd, unsigned position) {

}

static unsigned sc_tell (int fd) {
  return (unsigned) 0;
}

static void sc_close (int fd) {

}

static struct file* get_file(int fd) {
  struct list_elem* curr_elem = list_front(&thread_current()->file_list);
  for (int i = 2; i < fd; i++) {
    curr_elem = curr_elem->next;
  }
  return list_entry(curr_elem, struct file_with_fd, elem)->file_ptr;
}