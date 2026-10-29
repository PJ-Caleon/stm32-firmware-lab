#ifndef FAULT_HANDLER_H
#define FAULT_HANDLER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"{
#endif
  void fault_handler_init(void);
  void assert_failed(const char *file, int line);
#ifdef __cplusplus
}
#endif

#define HALT_IF_DEBUGGING() \
do { \
  if ((*(volatile uint32_t *)0xE000EDf0) & (1 << 0)){ \
    __asm("bkpt 1"); \
  } \
} while(0)

/*
  ASSERT Macro 
  Eval the expression, if false, call the assert_failed 
  with the current file and number line 
*/
#define ASSERT(expr) \
  do { \
    if (!(expr)) { \ 
      assert_failed(__FILE__, __LINE__); \
    } \
  } while (0)

typedef struct __attribute((packed)) ContextStateFrame {
  uint32_t r0;
  uint32_t r1;
  uint32_t r2;
  uint32_t r3;
  uint32_t r12;
  uint32_t lr;
  uint32_t return_address;
  uint32_t xpsr;
} sContextStateFrame;

#endif
