#include <stdint.h>
#include "stm32f103.h"
#include "dma.h"

/*
  THE IDEA
  --------
  DMA is a tiny copy machine that works next to the CPU. Each of the 7
  channels is one worker. To give a worker a job you fill in a "job card":

      CPAR   address on the peripheral side   (e.g. UART1->DR)
      CMAR   address in RAM                   (e.g. a buffer)
      CNDTR  how many items to move           (counts DOWN to 0)
      CCR    options (direction, auto-increment, circular, interrupt)

  Then you set the EN bit and the worker runs without the CPU.

  This file only does three things:
      1. fill in the job card          -> dma1_channel_config()
      2. start / stop the worker       -> dma1_channel_enable() / _disable()
      3. call you back when it's done  -> dma1_set_tc_callback() + the IRQ handlers

  It knows nothing about UART. uart.c decides WHAT to move.
*/

#define DMA_NUM_CHANNELS 7

// One "who do I call when this channel finishes?" slot per channel.
// Slot number = channel number - 1  (channel 1 lives in slot 0).
static dma_callback_t tc_callback[DMA_NUM_CHANNELS];

// Channels are numbered 1..7 in the manual, but C arrays start at 0.
// This helper hides the "- 1" so the rest of the file doesn't repeat it.
static dma_channel *channel(uint8_t ch){
  return &DMA1->CH[ch - 1];
}

// Setup

// Turn on the DMA1 clock. Peripherals do nothing until their clock is on.
void dma1_init(void){
  SET_BIT(RCC->AHBENR, RCC_AHBENR_DMA1EN);
  (void)RCC->AHBENR;   // dummy read: makes sure the clock is really on before we continue
}

// Fill in a channel's job card. Does NOT start it (call dma1_channel_enable).
void dma1_channel_config(uint8_t ch, uint32_t periph_addr, uint32_t mem_addr,
                         uint16_t count, uint32_t ccr_flags){
  dma_channel *c = channel(ch);

  // Switch the worker OFF first. CPAR/CMAR/CNDTR can only be written
  // while the channel is disabled.
  CLEAR_BIT(c->CCR, DMA_CCR_EN);

  // Wipe leftover flags (done / half-done / error) from the previous job.
  // CGIF = "clear ALL flags of this channel".
  DMA1->IFCR = DMA_IFCR_CGIF(ch);

  // Write the job card.
  c->CPAR  = periph_addr;
  c->CMAR  = mem_addr;
  c->CNDTR = count;

  // Write the options. "& ~DMA_CCR_EN" makes sure config never starts the
  // channel by accident, even if the caller passed the EN bit.
  c->CCR = ccr_flags & ~DMA_CCR_EN;
}

// Start / stop / progress
void dma1_channel_enable(uint8_t ch){
  SET_BIT(channel(ch)->CCR, DMA_CCR_EN);    // worker starts
}

void dma1_channel_disable(uint8_t ch){
  CLEAR_BIT(channel(ch)->CCR, DMA_CCR_EN);  // worker stops
}

// How many items are still left to move. Counts down from the original count.
// (uart.c uses this to know how far the RX DMA has written.)
uint16_t dma1_channel_remaining(uint8_t ch){
  return (uint16_t)channel(ch)->CNDTR;
}

// "Transfer complete" callback
/*
  How a finished transfer reaches your function:

    DMA moves the last item
      -> hardware sets this channel's TCIF flag in DMA1->ISR
      -> because TCIE was set in CCR, the NVIC fires DMA1_ChannelN_IRQHandler
      -> that handler calls dma1_irq(N)
      -> dma1_irq clears the flag and calls the function you registered
*/

// Register cb for channel ch and let that channel's interrupt through the NVIC.
// The channel's CCR must also have DMA_CCR_TCIE set, or the DMA never raises it.
void dma1_set_tc_callback(uint8_t ch, dma_callback_t cb){
  tc_callback[ch - 1] = cb;

  // The 7 DMA1 channel interrupts have consecutive numbers,
  // so channel N is Channel1's number + (N - 1).
  nvic_enable_irq(DMA1_Channel1_IRQn + (ch - 1));
}

// Shared body of all 7 interrupt handlers (runs in interrupt context).
static void dma1_irq(uint8_t ch){
  // Did THIS channel finish a transfer?
  if (DMA1->ISR & DMA_ISR_TCIF(ch)){

    // Clear the flag, otherwise the interrupt would fire again forever.
    DMA1->IFCR = DMA_IFCR_CTCIF(ch);

    // Call the registered function, if there is one.
    if (tc_callback[ch - 1]){
      tc_callback[ch - 1]();
    }
  }
}

// The real handlers (names fixed by the startup file's vector table).
// Each one just says which channel it is.
void DMA1_Channel1_IRQHandler(void){ dma1_irq(1); }
void DMA1_Channel2_IRQHandler(void){ dma1_irq(2); }
void DMA1_Channel3_IRQHandler(void){ dma1_irq(3); }
void DMA1_Channel4_IRQHandler(void){ dma1_irq(4); }
void DMA1_Channel5_IRQHandler(void){ dma1_irq(5); }
void DMA1_Channel6_IRQHandler(void){ dma1_irq(6); }
void DMA1_Channel7_IRQHandler(void){ dma1_irq(7); }
