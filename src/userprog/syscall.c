#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  if (!is_user_vaddr(f->esp)) {
    page_fault(&f);
    return;
  }
  if (!pagedir_get_page(f->esp)) {
    page_fault(&f);
    return;
  }
  int syscall_num = f->esp;
  switch (syscall_num) {
  case SYS_HALT:
    halt();
    break;

  case SYS_EXIT:
    int status = (f->esp) + 1;
    exit(status);
    break;

  case SYS_EXEC:
    const char *cmd_line = (f->esp) + 1; // first argv
    pid_t result = exec(cmd_line);
    f->eax = (uint32_t) result;
    break;

  case SYS_WAIT:
    __pid_t pid = (f->esp) + 1;
    int result = wait(pid);
    f->eax = (uint32_t) result;
    break;

  case SYS_CREATE:
    char *file = (f->esp) + 1;
    unsigned initial_size = (f->esp) + 2;
    bool result = create(file, initial_size);
    f->eax = (uint32_t) result;
    break;
    
  case SYS_REMOVE:
    char *file = (f->esp) + 1;
    bool result = remove(file);
    f->eax = (uint32_t) result;
    break;

  case SYS_OPEN:
    char *file = (f->esp) + 1;
    int result = open(file);
    f->eax = (uint32_t) result;
    break;

  case SYS_FILESIZE:
    int fd = (f->esp) + 1;
    int result = filesize(fd);
    f->eax = (uint32_t) result;
    break;
    
  case SYS_READ:
    int fd = (f->esp) + 1;
    void *buffer = (f->esp) + 2;
    unsigned size = (f->esp) + 3;
    int result = read(fd, buffer, size);
    f->eax = (uint32_t) result;
    break; 

  case SYS_WRITE:
    int fd = (f->esp) + 1;
    void *buffer = (f->esp) + 2;
    unsigned size = (f->esp) + 3;
    int result = write(fd, buffer, size);
    f->eax = (uint32_t) result;
    break;

  case SYS_SEEK:
    int fd = (f->esp) + 1;
    unsigned position = (f->esp) + 2;
    seek(fd, position);
    break;

  case SYS_TELL:
    int fd = (f->esp) + 1;
    unsigned result = tell(fd);
    f->eax = (uint32_t) result;
    break;
    
  case SYS_CLOSE:
    int fd = (f->esp) + 1;
    close(fd);
    break; 
  }

  printf ("system call!\n");
  thread_exit ();
}

// Task 2
void halt(void) {
}

void exit(int status) {
  thread_current()->exit_status = status;
  thread_exit();
}

__pid_t exec(const char *cmd_line) {
  return 0;
}

int wait (__pid_t pid) {
  return 0;
}

bool create (const char *file, unsigned initial_size) {
  return 0;
}

bool remove (const char *file) {
  return 0;
}

int open (const char *file) {
  return 0;
}

int filesize (int fd) {
  return 0;
}

int read (int fd, void *buffer, unsigned size) {
  return 0;
}

int write (int fd, const void *buffer, unsigned size) {
  if (fd == 1) {
    if (size > 500) {
      putbuf(buffer, 500);
      return 500;
    }
    else {
      putbuf(buffer, size);
      return size;
    }
  }
  else {
    return file_write(fd, buffer, size);
  }
}

void seek (int fd, unsigned position) {

}

unsigned tell (int fd) {
  return 0;
}

void close (int fd) {

}