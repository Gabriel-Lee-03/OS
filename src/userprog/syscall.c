#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include <console.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "userprog/process.h"
#include "threads/vaddr.h"
#include "filesys/file.h"


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

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  if (!is_user_vaddr(f->esp)) {
    //page_fault(&f);
    return;
  }
  if (!pagedir_get_page(thread_current()->pagedir, f->esp)) {
    //page_fault(&f);
    return;
  }
  int syscall_num = (int *)f->esp;
  switch (syscall_num) {
  case SYS_HALT:
    sc_halt();
    break;

  case SYS_EXIT:
    int status = (int *)(f->esp) + 1;
    sc_exit(status);
    break;

  case SYS_EXEC:
    const char *cmd_line = (int *)(f->esp) + 1; // first argv
    f->eax = (uint32_t) sc_exec(cmd_line);
    break;

  case SYS_WAIT:
    pid_t pid = (int *)(f->esp) + 1;
    f->eax = (uint32_t) sc_wait(pid);
    break;

  case SYS_CREATE:
    char *file = (int *)(f->esp) + 1;
    unsigned initial_size = (int *)(f->esp) + 2;
    f->eax = (uint32_t) sc_create(file, initial_size);
    break;
    
  case SYS_REMOVE:
    *file = (int *)(f->esp) + 1;
    f->eax = (uint32_t) sc_remove(file);
    break;

  case SYS_OPEN:
    *file = (int *)(f->esp) + 1;
    f->eax = (uint32_t) sc_open(file);
    break;

  case SYS_FILESIZE:
    int fd = (int *)(f->esp) + 1;
    f->eax = (uint32_t) sc_filesize(fd);
    break;
    
  case SYS_READ:
    fd = (int *)(f->esp) + 1;
    void *buffer = (int *)(f->esp) + 2;
    unsigned size = (int *)(f->esp) + 3;
    f->eax = (uint32_t) sc_read(fd, buffer, size);
    break; 

  case SYS_WRITE:
    fd = (int *)(f->esp) + 1;
    buffer = (f->esp) + 2;
    size = (f->esp) + 3;
    f->eax = (uint32_t) sc_write(fd, buffer, size);
    break;

  case SYS_SEEK:
    fd = (int *)(f->esp) + 1;
    unsigned position = (int *)(f->esp) + 2;
    sc_seek(fd, position);
    break;

  case SYS_TELL:
    fd = (int *)(f->esp) + 1;
    f->eax = (uint32_t) sc_tell(fd);
    break;
    
  case SYS_CLOSE:
    fd = (int *)(f->esp) + 1;
    sc_close(fd);
    break; 
  }

  printf ("system call!\n");
  thread_exit ();
}

// Task 2
static void sc_halt(void) {
}

static void sc_exit(int status) {
  thread_current()->exit_status = status;
  thread_exit();
}

static pid_t sc_exec(const char *cmd_line) {
  return 0;
}

static int sc_wait (pid_t pid) {
  return 0;
}

static bool sc_create (const char *file, unsigned initial_size) {
  return 0;
}

static bool sc_remove (const char *file) {
  return 0;
}

static int sc_open (const char *file) {
  return 0;
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
    return file_write(fd, buffer, size);
  }
}

static void sc_seek (int fd, unsigned position) {

}

static unsigned sc_tell (int fd) {
  return (unsigned) 0;
}

static void sc_close (int fd) {

}