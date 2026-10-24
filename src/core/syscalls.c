#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include "uart.h"

/*
  syscalls.c - glue between newlib (printf, puts, ...) and our hardware.
  Newlib calls these; they are not part of any one peripheral driver.
*/

/*
  Override the Newlib's low-level write syscall
  printf, puts, write() all end up here
*/
int _write(int file, char *ptr, int len){
  if(file != STDOUT_FILENO && file != STDERR_FILENO){
    return -1;
  }

  for (int i = 0; i < len; i++){
    if (*ptr == '\n'){
      uart_putc('\r');
    }
    uart_putc(*ptr++);
  }
  return len;
}

/*
  Minimal syscall stubs for newlib 
*/
int _close(int file){
  (void)file;
  return -1;
}

int _fstat(int file, struct stat *st){
  (void)file;
  st->st_mode = S_IFCHR;  // tell newlib stdout is a character device (a terminal)
  return 0;
}

int _isatty(int file){
  (void)file;
  return 1;               // stdout is a terminal
}

int _lseek(int file, int ptr, int dir){
  (void)file; (void)ptr; (void)dir;
  return 0;
}

int _read(int file, char *ptr, int len){
  (void)file; (void)ptr; (void)len;
  return 0;               // nothing to read via stdin; the shell uses uart_read_char()
}

int _getpid(void){
  return 1;
}

int _kill(int pid, int sig){
  (void)pid; (void)sig;
  errno = EINVAL;
  return -1;
}
