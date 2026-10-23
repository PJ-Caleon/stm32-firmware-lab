#include <stdint.h>
#include <unistd.h>
#include "stm32f103.h"
#include <errno.h>
#include <sys/stat.h>

#define UART_RX_BUFFER_SIZE 64

typedef struct Ring_Buffer{
  volatile uint8_t buffer[UART_RX_BUFFER_SIZE];
  volatile uint16_t head;
  volatile uint16_t tail;
} ring_buffer;

static ring_buffer rx_buffer = { {0}, 0, 0};

// Initialize uart1 clock  
void uart1_enable(void){
  SET_BIT(RCC->APB2ENR, (1 << 14));
}

/*
  Set PA9 as alternate function push pull 
  50 MHz 
  nibble 0xB
  10000

  Set PA10 as floating input
  nibble 0x4
  000000
*/
void init_tx1_rx1(void){
  CLEAR_BIT(GPIOA->CRH, (0xF << 4)); // Clear PINA9 (explanation of 4 & 8 in day8)
  CLEAR_BIT(GPIOA->CRH, (0xF << 8)); // Clear PINA10 

  MODIFY_REG(GPIOA->CRH, (0xF << 4), (0xB << 4)); // Configure PA9 (TX)
  MODIFY_REG(GPIOA->CRH, (0xF << 8), (0x4 << 8)); // Configure PA10 (RX)
}

/*
  BRR = 72,000,000/115,200 = 625 (0x271)
  CR1 gets UE, TE, RE 
  UART Enable - Bit 13
  Transmitter Enable - Bit 3 
  Receiver Enable - Bit 2 
*/
void enable_baud_rate(uint32_t pclk_freq){
  CLEAR_BIT(UART1->BRR, (0xF << 0));
  CLEAR_BIT(UART1->CR1, (0xF << 0));

  UART1->BRR = pclk_freq/115200;
  SET_BIT(UART1->CR1, (1 << 13) | (1 << 3) | (1 << 2));
}

/*
  Transmit a single character 
  waits until TXE (Transmit Register Data) flag is set to Status Register 
  Bits 7 of SR = TXE 
*/
void uart_putc(char c){
  while(!(UART1->SR & (1 << 7)));
  UART1->DR = (uint32_t)c;
}

/*
  Receives single character 
  Waits until RXE flag is set in the SR 
  Bits 5 of SR = RXNE (Read Data Register Not Empty)
*/
char uart_getc(void){
  while(!(UART1->SR & (1 << 5)));
  return (char)(UART1->DR & 0xFF);
}

/*
  Transmit null terminated string character by character
*/
void uart_puts(const char *s){
  while(*s != '\0'){
    uart_putc(*s);
    s++;
  }
}

/*
  Override the Newlib's low-level write syscall
  printf, puts, write()
*/
int _write(int file, char *ptr, int len){
  if(file != STDOUT_FILENO && file != STDERR_FILENO){
    return -1;
  }

  // Prints the characters from UART 
  for (int i = 0; i<len; i++){
    if (*ptr == '\n'){
      uart_putc('\r');
    }
    uart_putc(*ptr++);
  }
  return len;
}

/*
  Enable UART1 RX Interrupt and NVIC IRQ 37 
  Enable RXNE Interrupt 
  Enable IRQ 37
*/
void uart_rx_interrupt_init(void){
  SET_BIT(UART1->CR1, (1 << 5));
  NVIC->ISER[1] = (1 << (37 % 32));
}

/*
  Check if data is available in the ring buffer 
*/
int uart_data_available(void){
  return (rx_buffer.head != rx_buffer.tail);
}

/*
  Read a single character from the ring buffer  
*/
char uart_read_char(void){
  if (!uart_data_available()){
    return '\0';
  }
  char c = rx_buffer.buffer[rx_buffer.tail];
  rx_buffer.tail = (rx_buffer.tail + 1) % UART_RX_BUFFER_SIZE;
  return c;
}

/*
  UART1 Interrupt Service 
*/
void UART_IRQHandler(void){
  if (UART1->SR & (1 << 5)){
    uint8_t data = (uint8_t)(UART1->DR & 0xFF);
    uint8_t next_head = (rx_buffer.head + 1) % UART_RX_BUFFER_SIZE;

    if (next_head != rx_buffer.tail){
      rx_buffer.buffer[rx_buffer.head] = data;
      rx_buffer.head = next_head;
    } else{}
  }
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
  return 0;               // nothing to read via stdin; the shell uses its own UART buffer
}
int _getpid(void){
  return 1;
}
int _kill(int pid, int sig){
  (void)pid; (void)sig;
  errno = EINVAL;
  return -1;
}
