#include <stdint.h>
#include "stm32f103.h"
#include "dma.h"
#include "uart.h"

/*
  THE IDEA
  --------
  RX (receiving) = a ring of 256 pigeonholes.
      DMA is a mail carrier: every byte that arrives on the wire, it drops
      into the next pigeonhole, wrapping back to hole 0 after hole 255.
      You keep a bookmark (rx_tail) of the last hole YOU read.
      Unread bytes = the holes between your bookmark and the carrier.

  TX (sending) = a print queue with a printer.
      uart_putc() adds a byte at the back of the queue (tx_head).
      DMA is the printer: it takes a run of bytes from the front (tx_tail)
      and sends them on its own. When it finishes a run it interrupts us,
      and we start the next run if more bytes were queued meanwhile.
*/

#define UART_BAUD            115200
#define UART_RX_BUFFER_SIZE  256
#define TX_QUEUE_SIZE        128

// Which DMA1 channel serves USART1 is fixed by the chip (RM0008, DMA request table).
#define UART1_TX_DMA_CH  4
#define UART1_RX_DMA_CH  5

// RX  (receive)

// The DMA writes here. The CPU only reads. "volatile" because the DMA changes
// it behind the compiler's back.
static volatile uint8_t dma_rx_buffer[UART_RX_BUFFER_SIZE];

// Our bookmark: index of the next byte WE will read.
static uint16_t rx_tail = 0;

/*
  Where is the DMA right now? (index of the next hole it will write)

  CNDTR = items still to go, counting DOWN from 256 and reloading to 256
  after each lap (because the channel is circular). So:
      nothing received      CNDTR = 256  ->  head = 0
      3 bytes received      CNDTR = 253  ->  head = 3
*/
static uint16_t rx_head(void){
  return UART_RX_BUFFER_SIZE - dma1_channel_remaining(UART1_RX_DMA_CH);
}

// 1 if the DMA has written bytes we haven't read yet.
// Example: head = 5, tail = 3  ->  holes 3 and 4 are unread  ->  returns 1.
int uart_data_available(void){
  return (rx_tail != rx_head());
}

// Read one byte, or '\0' if nothing is waiting.
char uart_read_char(void){
  if (!uart_data_available()){
    return '\0';
  }

  char c = (char)dma_rx_buffer[rx_tail];

  // Move bookmark forward; "% SIZE" wraps 255 -> 0.
  rx_tail = (rx_tail + 1) % UART_RX_BUFFER_SIZE;
  return c;
}

// Note: nothing stops the DMA from lapping us. If the CPU doesn't read for
// 256 bytes, unread bytes are silently overwritten.

// TX  (transmit)
/*
  The queue is a ring (128 slots). Two indexes chase each other around it:

      tx_head  next FREE slot.  Moved only by uart_putc()      (main code)
      tx_tail  oldest byte NOT YET SENT. Moved only by the DMA-done interrupt

  Bytes between tail and head are waiting to go out:

      index:   0   1   2   3   4   5   6   7
      slot:   [ ] [ ] [A] [B] [C] [ ] [ ] [ ]
                       ^tail       ^head       -> A, B, C are waiting

  tail only moves AFTER the DMA finished sending, so bytes still being sent
  can never be overwritten by uart_putc().

  head == tail means EMPTY. To tell "full" from "empty" we always leave one
  slot unused, so the queue really holds 127 bytes.
*/
static uint8_t           tx_buffer[TX_QUEUE_SIZE];
static volatile uint16_t tx_head = 0;
static volatile uint16_t tx_tail = 0;

// 1 while a DMA transfer is in flight, 0 when the DMA is idle.
static volatile uint8_t  tx_busy = 0;

// How many bytes the transfer currently in flight covers.
static volatile uint16_t tx_chunk_len = 0;

/*
  Start sending whatever is waiting in the queue - if the DMA is free.
  Safe to call any time. Two callers:
      - uart_putc()       after it queued a byte
      - uart_tx_dma_done() when a transfer ends, to chain the next one
  They can't collide: a transfer-done interrupt only happens while tx_busy=1,
  and this function only starts a transfer while tx_busy=0.
*/
static void uart_tx_kick(void){

  // DMA already sending? Do nothing. uart_tx_dma_done() will call us again
  // when it finishes.
  if (tx_busy){
    return;
  }

  // Take a snapshot of the indexes.
  uint16_t head = tx_head;
  uint16_t tail = tx_tail;

  // Nothing queued? Done.
  if (head == tail){
    return;
  }

  // Decide how many bytes to send in ONE DMA transfer.
  // DMA copies a straight run of memory. It can't jump from the end of the
  // array back to the start by itself, so we send in pieces:
  if (head > tail){
    // Not wrapped: everything from tail up to head is one straight run.
    tx_chunk_len = head - tail;
  } else {
    // Wrapped: send only tail..end of array now.
    // The part at the start of the array goes in the next transfer.
    // Example: size 128, tail = 120, head = 10
    //          -> this transfer sends 8 bytes (slots 120..127)
    //          -> the next one sends 10 bytes (slots 0..9)
    tx_chunk_len = TX_QUEUE_SIZE - tail;
  }

  // Mark the DMA as busy BEFORE starting it.
  tx_busy = 1;

  // Fill the job card and start the worker.
  // DMA_CCR_DIR  = read from memory, write to the peripheral (UART->DR)
  // DMA_CCR_MINC = memory address goes up by 1 after every byte
  //                   (UART->DR stays the same address)
  // DMA_CCR_TCIE = interrupt me when the whole chunk is sent
  // (item size is 8-bit by default, which is what a byte stream needs)
  dma1_channel_config(UART1_TX_DMA_CH,
                      (uint32_t)&(UART1->DR),
                      (uint32_t)&tx_buffer[tail],
                      tx_chunk_len,
                      DMA_CCR_DIR | DMA_CCR_MINC | DMA_CCR_TCIE);
  dma1_channel_enable(UART1_TX_DMA_CH);
}

// Called by the DMA driver (in interrupt context) when a chunk is fully sent.
static void uart_tx_dma_done(void){
  // Those bytes are gone, so their slots are free again: move tail past them.
  tx_tail = (tx_tail + tx_chunk_len) % TX_QUEUE_SIZE;

  // DMA is idle now.
  tx_busy = 0;

  // Anything queued while we were sending? Send it right away.
  uart_tx_kick();
}

// Queue one byte for sending. Returns immediately, unless the queue is full.
// Do NOT call from an interrupt: the wait loop below would wait forever.
void uart_putc(char c){
  uint16_t next_head = (tx_head + 1) % TX_QUEUE_SIZE;

  // Queue full? Wait here until the DMA frees a slot. The kick call is a
  // safety net so the DMA is definitely running while we wait.
  while (next_head == tx_tail){
    uart_tx_kick();
  }

  // Put the byte in the free slot, THEN publish it by moving head.
  tx_buffer[tx_head] = (uint8_t)c;
  tx_head = next_head;

  // Start the DMA if it's idle. (If it's busy, uart_tx_dma_done() will pick
  // this byte up when the current chunk ends.)
  uart_tx_kick();
}

// Send a null-terminated string.
void uart_puts(const char *s){
  while (*s != '\0'){
    uart_putc(*s);
    s++;
  }
}

// Init
/*
  BRR (baud rate register) = 72,000,000 / 115,200 = 625 = 0x271
  (the register is a fixed-point number, so this one division is all we need)

  CR1: UE = USART enable (bit 13), TE = transmitter on (bit 3), RE = receiver on (bit 2)
  CR3: DMAR = "USART asks DMA to move each RECEIVED byte"
       DMAT = "USART asks DMA to feed it bytes to TRANSMIT"
*/
void uart_init(uint32_t pclk_freq){

  // Clocks for USART1 and DMA1.
  SET_BIT(RCC->APB2ENR, RCC_APB2ENR_USART1EN);
  (void)RCC->APB2ENR;
  dma1_init();

  // RX: start the DMA worker (channel 5) once and leave it running forever.
  // No DMA_CCR_DIR  -> direction is peripheral -> memory
  // DMA_CCR_MINC    -> memory address +1 per byte (UART->DR stays fixed)
  // DMA_CCR_CIRC    -> after 256 bytes, jump back to hole 0 and keep going
  // No TCIE         -> no interrupt; the CPU just polls the position
  dma1_channel_config(UART1_RX_DMA_CH,
                      (uint32_t)&(UART1->DR),
                      (uint32_t)dma_rx_buffer,
                      UART_RX_BUFFER_SIZE,
                      DMA_CCR_MINC | DMA_CCR_CIRC);
  dma1_channel_enable(UART1_RX_DMA_CH);

  // TX: nothing to send yet, so don't start the DMA. Just register which
  // function to call when a chunk finishes (this also enables the IRQ).
  dma1_set_tc_callback(UART1_TX_DMA_CH, uart_tx_dma_done);

  // Configure the USART itself and switch it on.
  UART1->BRR = pclk_freq / UART_BAUD;
  UART1->CR3 = USART_CR3_DMAR | USART_CR3_DMAT;
  UART1->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
}
