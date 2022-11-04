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
    int status;
    exit(status);
    break;

  case SYS_EXEC:
    const char *cmd_line;
    __pid_t result = exec(cmd_line);
    f->eax = (uint32_t) result;
    break;

  case SYS_WAIT:
    __pid_t pid;
    int result = wait(pid);
    f->eax = (uint32_t) result;
    break;

  case SYS_CREATE:
    char *file;
    unsigned initial_size;
    bool result = create(file, initial_size);
    f->eax = (uint32_t) result;
    break;
    
  case SYS_REMOVE:
    char *file;
    bool result = remove(file);
    f->eax = (uint32_t) result;
    break;

  case SYS_OPEN:
    char *file;
    int result = open(file);
    f->eax = (uint32_t) result;
    break;

  case SYS_FILESIZE:
    int fd;
    int result = filesize(fd);
    f->eax = (uint32_t) result;
    break;
    
  case SYS_READ:
    int fd;
    void *buffer;
    unsigned size;
    int result = read(fd, buffer, size);
    f->eax = (uint32_t) result;
    break; 

  case SYS_WRITE:
    int fd;
    void *buffer;
    unsigned size;
    int result = write(fd, buffer, size);
    f->eax = (uint32_t) result;
    break;

  case SYS_SEEK:
    int fd;
    unsigned position;
    seek(fd, position);
    break;

  case SYS_TELL:
    int fd;
    unsigned result = tell(fd);
    f->eax = (uint32_t) result;
    break;
    
  case SYS_CLOSE:
    int fd;
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
  return 0;
}

void seek (int fd, unsigned position) {

}

unsigned tell (int fd) {
  return 0;
}

void close (int fd) {

}