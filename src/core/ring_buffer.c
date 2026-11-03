#include "ring_buffer.h"

/*
  Bare Metal C - Mutex 
  Intterupt lock macros
*/

#if defined(__arm__) || defined(__thumb__)
// Save PRIMASK state and disable interrupt 
#define CRITICAL_ENTER() uint32_t primask; __asm__ volatile ("mrs %0, primask\ncpsid i" : "=r" (primask) :: "memory")

// Restore PRIMASK state 
#define CRITICAL_EXIT() __asm__ volatile ("msr primask, %0" :: "r" (primask) : "memory")
#else 
#define CRITICAL_ENTER() do {} while(0)
#define CRITICAL_EXIT() do {} while(0) 
#endif

void ring_buffer_init(ring_buffer_t *buffer, char *buf, size_t buf_size){
  RING_BUFFER_ASSERT(RING_BUFFER_IS_POWER_OF_TWO(buf_size) == 1);
  CRITICAL_ENTER();
  buffer->buffer = buf;
  buffer->buffer_mask = buf_size - 1;
  buffer->tail_index = 0;
  buffer->head_index = 0;
  CRITICAL_EXIT();
}

uint8_t ring_buffer_queue(ring_buffer_t *buffer, char data){
  CRITICAL_ENTER();
  if(ring_buffer_full(buffer)){
    CRITICAL_EXIT();
    return 0;
  }

  // Place data in buffer 
  buffer->buffer[buffer->head_index] = data;
  buffer->head_index = ((buffer->head_index + 1) & RING_BUFFER_MASK(buffer));
  CRITICAL_EXIT();
  return 1;
}

void ring_buffer_queue_arr(ring_buffer_t *buffer, const char *data, ring_buffer_size_t size){
  // Add bytes 
  ring_buffer_size_t i;
  for(i = 0; i < size; i++){
    ring_buffer_queue(buffer, data[i]);
  }
}

// Retrieves oldest pending event from queue for main.c to process 
uint8_t ring_buffer_dequeue(ring_buffer_t *buffer, char *data){
  CRITICAL_ENTER();
  if(ring_buffer_empty(buffer)){
    CRITICAL_EXIT();
    return 0;
  }
  *data = buffer->buffer[buffer->tail_index];
  buffer->tail_index = ((buffer->tail_index + 1) & RING_BUFFER_MASK(buffer));
  CRITICAL_EXIT();
  return 1;
}

ring_buffer_size_t ring_buffer_dequeue_arr(ring_buffer_t *buffer, char *data, ring_buffer_size_t len){
  CRITICAL_ENTER();
  if(ring_buffer_empty(buffer)){
    CRITICAL_EXIT();
    return 0;
  }
  char *data_ptr = data;
  ring_buffer_size_t cnt = 0;
  while((cnt < len) && ring_buffer_dequeue(buffer, data_ptr)){
    cnt++;
    data_ptr++;
  }
  CRITICAL_EXIT();
  return cnt;
}

uint8_t ring_buffer_peek(ring_buffer_t *buffer, char *data, ring_buffer_size_t index){
  CRITICAL_ENTER();
  if(index >= ring_buffer_num_items(buffer)){
    CRITICAL_EXIT();
    return 0;
  }
  // Add index to pointer 
  ring_buffer_size_t data_index = ((buffer->tail_index + index) & RING_BUFFER_MASK(buffer));
  *data = buffer->buffer[data_index];
  CRITICAL_EXIT();
  return 1;
}

extern inline uint8_t ring_buffer_empty(ring_buffer_t *buffer);
extern inline uint8_t ring_buffer_full(ring_buffer_t *buffer);
extern inline ring_buffer_size_t ring_buffer_num_items(ring_buffer_t *buffer);
