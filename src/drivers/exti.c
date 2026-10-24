#include "stm32f103.h"
#include "exti.h"
#include "clock.h"

/*
  HOW EXTI WORKS
  --------------
  The chip has 16 interrupt "lines": EXTI0 ... EXTI15.
  Line N listens to pin N of ONE port that you choose:
  line 3 can listen to PA3 or PB3 or PC3, but only one of them at a time.
  So PA0 and PB0 can never both be interrupts at once.

  Making a pin an interrupt takes 4 steps. exti_enable_line() does all of them:
    1. AFIO->EXTICR   choose which PORT line N listens to
    2. FTSR / RTSR    choose the edge (falling / rising)
    3. IMR            unmask the line (a masked line is ignored)
    4. NVIC           enable the CPU interrupt this line is wired to

  Lines do NOT each have their own CPU interrupt:
    lines 0-4    one handler each:  EXTI0_IRQHandler ... EXTI4_IRQHandler
  A shared handler must look at EXTI->PR to see WHICH line fired.

  CHECKLIST: adding another pin (example: PA1, falling edge, line 1)
    1. port_pin.c   configure PA1 as an input (pull-up/pull-down if needed)
    2. exti.c       call exti_enable_line(EXTI_PORTA, 1, EXTI_FALLING) from an init function
    3. exti.c       write the handler: void EXTI1_IRQHandler(void)
                    it MUST clear its pending bit: EXTI->PR = (1U << 1);
    4. startup.c    add the handler to the vector table AND a weak alias for it,
                    or the interrupt jumps to nothing and the board hard-faults
*/

volatile uint8_t exti_flag = 0; // PINA0: 0 not pressed, 1 pressed 
volatile uint32_t interrupt_timestamp = 0;
volatile uint32_t dwt_start = 0;
volatile uint32_t dwt_isr = 0;

// ------------------------------------------------------------------ setup
// Which NVIC interrupt serves a given EXTI line
static uint8_t exti_irq_number(uint8_t line){
  if (line <= 4){
    return EXTI0_IRQn + line;   // lines 0..4 -> IRQ 6..10, one each
  }
  if (line <= 9){
    return EXTI9_5_IRQn;        // lines 5..9 share one
  }
  return EXTI15_10_IRQn;        // lines 10..15 share one
}

void exti_enable_line(uint8_t port, uint8_t line, exti_edge_t edge){
  if (line > 15){
    return;
  }

  uint32_t line_mask = (1U << line);   // this line's bit in IMR / FTSR / RTSR / PR

  /*
    Step 1: route the pin to the line.
    EXTICR[0..3] hold 4 bits per line, 4 lines per register:
      EXTICR[0] = lines 0-3, EXTICR[1] = lines 4-7, EXTICR[2] = 8-11, EXTICR[3] = 12-15
    The 4-bit value is the port: A=0, B=1, C=2.
    Example: line 13 -> EXTICR[3], bits 7:4.
  */
  uint32_t reg_index = line / 4;
  uint32_t bit_shift = (line % 4) * 4;
  MODIFY_REG(AFIO->EXTICR[reg_index], (0xFUL << bit_shift), ((uint32_t)port << bit_shift));

  // Step 2: which edge(s) trigger it
  if (edge == EXTI_FALLING || edge == EXTI_BOTH){
    SET_BIT(EXTI->FTSR, line_mask);
  }
  if (edge == EXTI_RISING || edge == EXTI_BOTH){
    SET_BIT(EXTI->RTSR, line_mask);
  }

  // Step 3: drop anything left pending, then unmask the line
  EXTI->PR = line_mask;
  SET_BIT(EXTI->IMR, line_mask);

  // Step 4: let the CPU receive it
  nvic_enable_irq(exti_irq_number(line));
}

// The PA1 button
void exti1_init(void){
  exti_enable_line(EXTI_PORTA, 1, EXTI_FALLING);
}

// ------------------------------------------------------------------ handlers
/*
  Clear pending bit in ISR
  Record the time 
  Set a flag 
  
  EXTI1 since that is where PA1 lives at. Keep handlers SHORT: record
  what happened and let the main loop (button.c) decide what it means.
*/
void EXTI1_IRQHandler(void){
  dwt_isr = DWT_CYCCNT;
  EXTI->PR = EXTI_LINE1; // Acknowledges interrupt occured (write 1 to clear). Forget this and it re-fires forever.
  interrupt_timestamp = millis();
  exti_flag = 1;
}

// ------------------------------------------------------------------ debug
/*
  Software-trigger line 1 to measure interrupt latency with the DWT counter.
  The fake edge is swallowed here so the game never sees it as a button press.
*/
void exti1_latency_test(void){
  dwt_start = DWT_CYCCNT;
  EXTI->SWIER = EXTI_LINE1;

  uint32_t t = millis();
  while (!exti_flag && (millis() - t) < 2); // bounded wait, never hangs the boot
  exti_flag = 0;
}
