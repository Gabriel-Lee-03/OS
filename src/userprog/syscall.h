#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

struct lock file_lock;   /* Lock for sychronizing file actions */

void syscall_init (void);
void sc_exit(int);

#endif /* userprog/syscall.h */
