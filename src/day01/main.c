#include <stdint.h>

// Reference _estack
extern unsigned int _estack;

volatile uint32_t global_var = 0;

void Reset_Handler(void);

uint32_t *vector_table[] __attribute__((section(".isr_vector"))) = {
  (uint32_t*)&_estack,
  (uint32_t*)Reset_Handler
};

void Reset_Handler(void){
  while(global_var < 3){
    global_var++;
  }
}

int main(){
  return 0;
}
