#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include <stdio.h>
#include <list.h>

struct lock file_lock;   /* Lock for sychronizing file actions */

// Task 2
/* Struct for storing and converting file to fd */
struct file_with_fd {
  struct file* file_ptr;
  int fd;
  struct list_elem elem;
};

void syscall_init (void);
void sc_exit(int);

#endif /* userprog/syscall.h */
